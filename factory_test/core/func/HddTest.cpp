#include "../../inc/HddTest.h"
#include "../../inc/fac_log.h"

string hdd_screen_log = "";

HddTest::HddTest()
{
    
}

string HddTest::hdd_test_all(string hdd_cap)
{
    execute_command("bash " + HDD_TEST_SCRIPT + " " + hdd_cap);
	if(check_if_hdd_pass() == true)
		return "SUCCESS";
	return "FAIL";
}

bool HddTest::check_if_hdd_pass()
{
    char hdd_status[CMD_BUF_SIZE];
	
	memset(hdd_status, 0, CMD_BUF_SIZE);
	int size = 0;
	if(!get_file_size("/tmp/hdd.status",&size)) {
		LOG_ERROR("/tmp/hdd.status is null\n");
		hdd_screen_log += "/tmp/hdd.status is null\n\n";
		return false;
	}
	if(!read_local_data("/tmp/hdd.status", hdd_status, size)) {
		hdd_screen_log += "read /tmp/hdd.status fail\n\n";
		return false;
	}
    if (!strcmp(delNL(hdd_status), "SUCCESS")) {
        LOG_INFO("HDD Test result: \t%s\n", "PASS");
        return true;
    } else {
        LOG_ERROR("HDD test failed: \t%s\n", hdd_status);
		hdd_screen_log += "HDD test failed:\t\t" + (string)hdd_status + "\n\n";
        return false;
    }
}

void* HddTest::test_all(void *arg)
{
	Control *control = Control::get_control();
	control->set_interface_test_status(HDD_TEST_NAME, false);
	hdd_screen_log += "==================== hdd test ====================\n";
	BaseInfo* baseInfo = (BaseInfo *)arg;
	string result = hdd_test_all(baseInfo->hdd_cap);
	hdd_screen_log += "hdd test result:\t\t\t" + result + "\n\n";
	if (result == "SUCCESS") {
		control->set_interface_test_result(HDD_TEST_NAME, true); 
	} else {
		control->set_interface_test_result(HDD_TEST_NAME, false); 
	}
	control->update_screen_log(hdd_screen_log);
	control->set_interface_test_status(HDD_TEST_NAME, true);
	hdd_screen_log = "";
	return NULL;
}

void HddTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}


