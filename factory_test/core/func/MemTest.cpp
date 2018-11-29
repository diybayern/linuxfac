#include "FuncTest.h"
#include <math.h>

string MemTest::screen_log_black = "";
string MemTest::screen_log_red = "";

bool MemTest::compare_men_cap(int mem_cap)
{
    if (mem_cap <= 0) {
        LOG_ERROR("mem cap is wrong");
        return false;
    }
    float mem_cap_min = mem_cap * 1024 * MEM_CAP_MIN_PERCENT;  // mem cap range should be the theoretical mem_cap * [0.9 ~ 1] 
    float mem_cap_max = mem_cap * 1024;  // unit: M
    string real_mem_cap = execute_command("free -m | awk '/Mem/ {print $2}'", true);
    if (real_mem_cap == "error") {
        LOG_ERROR("get real mem cap failed");
        screen_log_black += "ERROR: get real mem cap failed\n";
        screen_log_red += "\t错误：获取系统内存大小失败\n";
        return false;
    }
    if (get_int_value(real_mem_cap) > mem_cap_min  && get_int_value(real_mem_cap) < mem_cap_max){
        screen_log_black += "current mem cap is " + real_mem_cap + "M\n\n";
        LOG_INFO("current mem cap is %sM\n", real_mem_cap.c_str());
        return true;
    } else {
        screen_log_red += "\t错误：内存大小应为" + to_string(mem_cap * 1024) + "M，但只检测到" + real_mem_cap + "M\n";
        screen_log_black += "ERROR: mem cap should be " + to_string(mem_cap * 1024) + "M but current is " + real_mem_cap + "M\n\n";
        LOG_ERROR("ERROR: mem cap should be %dG but current is %sM\n\n", mem_cap, real_mem_cap.c_str());
        return false;
    }
}

bool MemTest::mem_stability_test()
{
    string stable_result;
    stable_result = execute_command("sh " + MEM_TEST_SCRIPT + " " + MEM_TEST_CAP, true);
    LOG_INFO("stable_result is:%s", stable_result.c_str());
    if (stable_result == "SUCCESS") {
        return true;
    } else {
        screen_log_red += "\t错误：内存稳定性测试失败\n"; // update file log in function "test_all()"
        return false;
    }
}

void* MemTest::test_all(void *arg)
{
    //LOG_DEBUG("mem test all");
    if (arg == NULL) {
        LOG_ERROR("arg is null");
        return NULL;
    }
    Control *control = Control::get_control();
    BaseInfo* baseInfo = (BaseInfo*)arg;
    bool is_pass = true;
    
    pthread_detach(pthread_self());
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_MEM], false);
    screen_log_black = "";
    screen_log_red = "";
    screen_log_black += "==================== " + INTERFACE_TEST_NAME[I_MEM] + " ====================\n";
    
    if (!control->get_third_product_state()) {
        is_pass = compare_men_cap(get_int_value(baseInfo->mem_cap));
    }
    
    is_pass &= mem_stability_test();
    string stability_result = execute_command("cat " + MEM_UI_LOG, true);
    screen_log_black += stability_result + "\n\n";
    LOG_INFO("mem stability test result:%s\n", stability_result.c_str()); // stability test result log

    if (is_pass) {
        LOG_INFO("mem test result:\tPASS\n");
        screen_log_black += INTERFACE_TEST_NAME[I_MEM] + "结果：\t\t\t成功\n\n";
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_MEM], true); 
    } else {
        LOG_INFO("mem test result:\tFAIL\n");
        screen_log_red = INTERFACE_TEST_NAME[I_MEM] + "结果：\t\t\t失败\n\n" + screen_log_red;
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_MEM], false); 
    }
    remove_local_file(MEM_UI_LOG);
    control->update_color_screen_log(screen_log_black, "black");
    if (screen_log_red != "") {
        control->update_color_screen_log(screen_log_red, "red");
    }
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_MEM], true);
    return NULL;
}


void MemTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    //LOG_DEBUG("mem thread create start");
    pthread_t tid;
    int err = pthread_create(&tid, NULL, test_all, baseInfo);
    if (err != 0) {
        LOG_ERROR("mem test create thread error: %s", strerror(err));
    }
    /*else {
        LOG_DEBUG("mem thread create end");
    }*/
}


