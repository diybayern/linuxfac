#include "../../inc/MemTest.h"
#include "../../inc/fac_log.h"
#include "../../inc/fac_utils.h"
#include <math.h>

string mem_screen_log = "";

MemTest::MemTest()
{
}

bool MemTest::compare_men_cap(int mem_cap)
{
	float mem_cap_min = mem_cap * 1024 * 0.9;
	float mem_cap_max = mem_cap * 1024;
    string real_mem_cap = execute_command("free -m | awk '/Mem/ {print $2}'");
    if (get_int_value(real_mem_cap) > mem_cap_min  && get_int_value(real_mem_cap) < mem_cap_max){
		mem_screen_log += "mem cap is right\n";
		return true;
    } else {
    	mem_screen_log += "ERROR: mem cap should be " + to_string(mem_cap) + "G but current is " + real_mem_cap + "K\n\n";
        return false;
    }
}

bool MemTest::mem_stability_test()
{
	string stable_result;
	stable_result = execute_command("sh " + MEM_TEST_SCRIPT);
	LOG_INFO("stable_result is:%s",stable_result.c_str());
	if (stable_result == "SUCCESS") {
		return true;
	} else {
		return false;
	}
}

void* MemTest::test_all(void *arg)
{
    Control *control = Control::get_control();
	control->set_interface_test_status(MEM_TEST_NAME, false);
	BaseInfo* baseInfo = (BaseInfo *)arg;
	bool is_pass;
	mem_screen_log += "==================== mem test ====================\n";
	is_pass    = compare_men_cap(get_int_value(baseInfo->mem_cap));
	is_pass   &= mem_stability_test();
	mem_screen_log += execute_command("cat " + MEM_UI_LOG) + "\n\nmem test result:\t\t\t";
	if (is_pass) {
		mem_screen_log += "SUCCESS\n\n";
        control->set_interface_test_result(MEM_TEST_NAME, true); 
	} else {
		mem_screen_log += "FAIL\n\n";
        control->set_interface_test_result(MEM_TEST_NAME, false); 
	}
	control->update_screen_log(mem_screen_log);
	control->set_interface_test_status(MEM_TEST_NAME, true);
    remove_local_file(MEM_UI_LOG.c_str());
	mem_screen_log = "";
	return NULL;
}


void MemTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}


