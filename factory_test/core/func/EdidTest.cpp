#include "../../inc/EdidTest.h"
#include "../../inc/fac_log.h"
#include "../../inc/fac_utils.h"
#include "../../inc/libx86.h"

#include <sys/types.h>
//#include <libx86.h>
#include <sys/io.h>

#include <fcntl.h>

#define MAGIC             (0x13)
#define EDID_BLOCK_SIZE	  (128)
#define SERVICE_READ_EDID (1)
#define SERVICE_LAST	  (1)  // Read VDIF has been removed from the spec.

#define access_register(reg_frame, reg)     (reg_frame.reg)
#define access_seg_register(reg_frame, es)  (reg_frame.es)
#define access_ptr_register(reg_frame, reg) (reg_frame->reg)

pthread_mutex_t g_reg_mutex = PTHREAD_MUTEX_INITIALIZER;
string edid_screen_log = "";

extern int edid_read_i2c_test(int edid_num);

EdidTest::EdidTest()
{

}

int EdidTest::do_vbe_service(unsigned int AX, unsigned int BX, reg_frame* regs) {

	int error = 0;
	unsigned int success;
	unsigned int function_sup;
	const unsigned int interrupt = 0x10;

	access_ptr_register(regs, eax) = AX;
	access_ptr_register(regs, ebx) = BX;

	if (!LRMI_int(interrupt, regs)) {
		LOG_ERROR("something went wrong performing real mode interrupt\n");
		error = 1;
	}

	AX = access_ptr_register(regs, eax);

	function_sup = ((AX & 0xff) == 0x4f);
	success = ((AX & 0xff00) == 0);

	if (!success) {
		error = 1;
		LOG_ERROR("call failed\n");
	}

	if (!function_sup) {
		error = 2;
		LOG_ERROR("function not support\n");
	}

	return error;
}

int EdidTest::do_vbe_ddc_service(unsigned BX, reg_frame* regs) {
	unsigned service = BX & 0xff;
	unsigned AX = 0x4f15;

	if (service > SERVICE_LAST) {
		LOG_ERROR("Unknown VBE/DDC service\n");
	}

	return do_vbe_service(AX, BX, regs);
}

int EdidTest::read_edid(unsigned int controller, char* output) {

	int i = 0;
	reg_frame regs;
	unsigned char* block = NULL;

	block = (unsigned char*)LRMI_alloc_real(EDID_BLOCK_SIZE);

	if (!block) {
		LOG_ERROR("can't allocate 0x%x bytes of DOS memory for output block\n",
				EDID_BLOCK_SIZE);
		return FAIL;
	}

	memset(block, MAGIC, EDID_BLOCK_SIZE);
	memset(&regs, 0, sizeof(regs));

	access_seg_register(regs, es) = (unsigned int) ((size_t) block) / 16;
	access_register(regs, edi) = 0;
	access_register(regs, ecx) = controller;
	access_register(regs, edx) = 1;

	if (do_vbe_ddc_service(SERVICE_READ_EDID, &regs)) {
		LRMI_free_real(block);
		LOG_ERROR("The EDID data  as the VBE call failed\n");
		return FAIL;
	}

	for (i = 0; i < EDID_BLOCK_SIZE; i++) {
		if (MAGIC != block[i]) {
			break;
		}
	}

	if (i == EDID_BLOCK_SIZE) {
		LRMI_free_real(block);
		LOG_ERROR("Error: output block unchanged\n");
		return FAIL;
	}

	memcpy(output, block, EDID_BLOCK_SIZE);

	LRMI_free_real(block);

	return SUCCESS;
}

int EdidTest::parse_edid(char* buf) {

	int i = 0;
	char check_sum = 0;

	for (i = 0; i < EDID_BLOCK_SIZE; i++) {
		check_sum += buf[i];
	}

	if (0 != check_sum) {
		LOG_ERROR("check sum failed sum=%d\n", check_sum);
		return FAIL;
	}

	return SUCCESS;
}

int EdidTest::edid_test_all(unsigned int num) {

	int ret = 0;
	int failed = 0;
    bool read_ret = true;
    bool parse_ret = true;
	char edid_buf[EDID_BLOCK_SIZE] = { 0, };
    int edid_num = num;

	LOG_INFO("edid test start\n");

	pthread_mutex_lock(&g_reg_mutex);

i2c_test:
    ret = edid_read_i2c_test(edid_num);
    if (ret == SUCCESS) {
        pthread_mutex_unlock(&g_reg_mutex);
        goto print;
    } else {
        if (ret == AGAIN && failed++ < 5) {
            LOG_ERROR("Failed to read EDID from I2C bus, try again.");
            goto i2c_test;
        }
        if (edid_num == 2) {           
            pthread_mutex_unlock(&g_reg_mutex);
            if (ret == AGAIN) {
                LOG_ERROR("ERROR: Failed to read any EDID information on the buses.\n");
            }
            ret = FAIL;
            goto print;
        }
    }
    
lrmi_start:
    failed = 0;
    ret = LRMI_init();
	if (!ret) {
		LOG_ERROR("init real mode interface failed\n");
		failed++;
		goto error;
	}

	ioperm(0, 0x400, 1);
	iopl(3);

	read_ret = read_edid(0, edid_buf);	
	if (FAIL == read_ret) {
		LOG_ERROR("read edid failed\n");
		failed++;
		goto error;
	}

	parse_ret = parse_edid(edid_buf);	
	if (FAIL == parse_ret) {
		LOG_ERROR("parse edid failed\n");
		failed++;
		goto error;
	}

	failed = 0;

error:
	if (0 != failed && failed < 4) {
		goto lrmi_start;
	}
    
	pthread_mutex_unlock(&g_reg_mutex);

    ret = read_ret & parse_ret;
    LOG_INFO("\tread edid: \t\t\t\t%s\n", PRINT_RESULT(read_ret));
    LOG_INFO("\tparse edid : \t\t\t\t%s\n", PRINT_RESULT(parse_ret));

print:
	LOG_INFO("\nedid test result: \t\t\t\t%s\n", PRINT_RESULT1(ret));
	return ret;
}

int EdidTest::get_edid_num(BaseInfo* baseInfo){
	int vga = 0, hdmi = 0;
	if (baseInfo->vga_exist == "" || baseInfo->vga_exist == "0") {
		vga = 0;
	} else {
		vga = get_int_value(baseInfo->vga_exist);
	}

	if (baseInfo->hdmi_exist == "" || baseInfo->hdmi_exist == "0") {
		hdmi = 0;
	} else {
		hdmi = get_int_value(baseInfo->hdmi_exist);
	}
	return vga + hdmi;
}
void* EdidTest::test_all(void *arg)
{
	
	Control *control = Control::get_control();
	control->set_interface_test_status(EDID_TEST_NAME, false);
	BaseInfo* baseInfo = (BaseInfo *)arg;	
	edid_screen_log += "==================== edid test ====================\n";
	int edid_num = get_edid_num(baseInfo);
	LOG_INFO("edid num: %d", edid_num);
	int is_pass = edid_test_all(edid_num);
	edid_screen_log += "edid test result:\t\t\t";
	if (is_pass == SUCCESS) {
		edid_screen_log += "SUCCESS\n\n";
        control->set_interface_test_result(EDID_TEST_NAME, true);
	} else {
		edid_screen_log += "FAIL\n\n";
        control->set_interface_test_result(EDID_TEST_NAME, false);
	}
	control->update_screen_log(edid_screen_log);
	control->set_interface_test_status(EDID_TEST_NAME, true);
	edid_screen_log = "";
	return NULL;
}

void EdidTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}


