#include "EdidTest.h"
#include "libx86.h"

#include <sys/types.h>
#include <sys/io.h>

#include <fcntl.h>

#define MAGIC             (0x13)
#define EDID_BLOCK_SIZE   (128)
#define SERVICE_READ_EDID (1)
#define SERVICE_LAST      (1)  // Read VDIF has been removed from the spec.

#define access_register(reg_frame, reg)     (reg_frame.reg)
#define access_seg_register(reg_frame, es)  (reg_frame.es)
#define access_ptr_register(reg_frame, reg) (reg_frame->reg)

pthread_mutex_t g_reg_mutex = PTHREAD_MUTEX_INITIALIZER;

extern string get_edid_i2c_screen_log();
extern string get_edid_i2c_screen_red();


string EdidTest::screen_log_black = "";
string EdidTest::screen_log_red = "";

bool EdidTest::do_vbe_service(unsigned int AX, unsigned int BX, reg_frame* regs)
{
    bool error = true;
    unsigned int success;
    unsigned int function_sup;
    const unsigned int interrupt = 0x10;

    access_ptr_register(regs, eax) = AX;
    access_ptr_register(regs, ebx) = BX;

    if (!LRMI_int(interrupt, regs)) {
        LOG_ERROR("something went wrong performing real mode interrupt\n");
        error = false;
    }

    AX = access_ptr_register(regs, eax);

    function_sup = ((AX & 0xff) == 0x4f);
    success = ((AX & 0xff00) == 0);

    if (!success) {
        error = false;
        LOG_ERROR("call failed\n");
    }

    if (!function_sup) {
        error = false;
        LOG_ERROR("function not support\n");
    }

    return error;
}

bool EdidTest::do_vbe_ddc_service(unsigned BX, reg_frame* regs)
{
    unsigned service = BX & 0xff;
    unsigned AX = 0x4f15;

    if (service > SERVICE_LAST) {
        LOG_ERROR("Unknown VBE/DDC service\n");
        return false;
    }

    return do_vbe_service(AX, BX, regs);
}

bool EdidTest::read_edid(unsigned int controller, char* output) //TODO: char* output
{
    int i = 0;
    reg_frame regs;
    unsigned char* block = NULL;  //TODO: unsigned char* block

    block = (unsigned char*)LRMI_alloc_real(EDID_BLOCK_SIZE);

    if (!block) {
        LOG_ERROR("can't allocate 0x%x bytes of DOS memory for output block\n", EDID_BLOCK_SIZE);
        return false;
    }

    memset(block, MAGIC, EDID_BLOCK_SIZE);
    memset(&regs, 0, sizeof(regs));

    access_seg_register(regs, es) = (unsigned int)((size_t) block) / 16;
    access_register(regs, edi) = 0;
    access_register(regs, ecx) = controller;
    access_register(regs, edx) = 1;

    if (!do_vbe_ddc_service(SERVICE_READ_EDID, &regs)) {
        LRMI_free_real(block);
        LOG_ERROR("The EDID data  as the VBE call failed\n");
        return false;
    }

    for (i = 0; i < EDID_BLOCK_SIZE; i++) {
        if (block[i] != MAGIC) {
            break;
        }
    }

    if (i == EDID_BLOCK_SIZE) {
        LRMI_free_real(block);
        LOG_ERROR("Error: output block unchanged\n");
        return false;
    }

    memcpy(output, block, EDID_BLOCK_SIZE);
    if (output == 0) {
        return false;
    }

    LRMI_free_real(block);

    return true;
}

bool EdidTest::parse_edid(char* buf)   //TODO: char* buf
{
    if (buf == NULL) {
        LOG_ERROR("buf is NULL");
        return false;
    }
    int i = 0;
    char check_sum = 0;

    for (i = 0; i < EDID_BLOCK_SIZE; i++) {
        check_sum += buf[i];
    }

    if (check_sum != 0) {
        LOG_ERROR("check sum failed sum=%d\n", check_sum);
        return false;
    }

    return true;
}

bool EdidTest::edid_test_all(unsigned int num)
{
    int ret = 0;
    int failed = 0;
    bool read_ret = true;
    bool parse_ret = true;
    bool result = true;
    char edid_buf[EDID_BLOCK_SIZE] = {0};  //TODO: char[] edid_buf
    int edid_num = num;

    LOG_INFO("edid test start\n");

    pthread_mutex_lock(&g_reg_mutex);

i2c_test:
    ret = edid_read_i2c_test(edid_num); // get linked edid devices and compare with edid_num
    screen_log_black += get_edid_i2c_screen_log();
    screen_log_red += get_edid_i2c_screen_red();
    if (ret == SUCCESS) {
        pthread_mutex_unlock(&g_reg_mutex);
        result = true;
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
                screen_log_black += "ERROR: Failed to read any EDID information on the buses.\n";
                screen_log_red += "\t错误：无法读取总线上的任何EDID信息\n";
            }
            result = false;
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
    if (read_ret == false) {
        LOG_ERROR("read edid failed\n");
        failed++;
        goto error;
    }

    parse_ret = parse_edid(edid_buf);    
    if (parse_ret == false) {
        LOG_ERROR("parse edid failed\n");
        failed++;
        goto error;
    }

    failed = 0;

error:
    if (failed != 0 && failed < 4) {
        goto lrmi_start;
    }
    
    pthread_mutex_unlock(&g_reg_mutex);

    result = read_ret && parse_ret;
    LOG_INFO("read edid: \t%s\n", STRING_RESULT(read_ret));
    LOG_INFO("parse edid : \t%s\n", STRING_RESULT(parse_ret));

print:
    LOG_INFO("edid test result: \t%s\n", STRING_RESULT(result));
    return result;
}

/* get edid theoretical num */
int EdidTest::get_edid_num(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseinfo is null");
        return 0;
    }
    
    int vga = 0, hdmi = 0;
    
    if (baseInfo->vga_exist != "" && baseInfo->vga_exist != "0") {
        vga = get_int_value(baseInfo->vga_exist);
    }

    if (baseInfo->hdmi_exist != "" && baseInfo->hdmi_exist != "0") {
        hdmi = get_int_value(baseInfo->hdmi_exist);
    }
    
    return vga + hdmi;
}

void* EdidTest::test_all(void *arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is null");
        return NULL;
    }
    Control *control = Control::get_control();
    BaseInfo* baseInfo = (BaseInfo *)arg;
    
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_EDID], false);
    screen_log_black = "";
    screen_log_red = "";
    
    screen_log_black += "==================== " + INTERFACE_TEST_NAME[I_EDID] + " ====================\n";
    int edid_num = get_edid_num(baseInfo);

    bool is_pass;
    if (edid_num <= 0) {
        LOG_ERROR("edid num=%d is wrong", edid_num);
        is_pass = false;
    } else {
        LOG_INFO("edid num: %d", edid_num);
        is_pass = edid_test_all(edid_num);
    }
    
    if (is_pass) {
        screen_log_black += INTERFACE_TEST_NAME[I_EDID] + "结果:\t\t\t成功\n\n";
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_EDID], true);
    } else {
        screen_log_red = INTERFACE_TEST_NAME[I_EDID] + "结果:\t\t\t失败\n\n" + screen_log_red;
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_EDID], false);
    }
    control->update_color_screen_log(screen_log_black, "black");
    if (screen_log_red != "") {
        control->update_color_screen_log(screen_log_red, "red");
    }
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_EDID], true);
    return NULL;
}

void EdidTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}


