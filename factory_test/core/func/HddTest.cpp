#include "../../inc/HddTest.h"
#include "../../inc/fac_log.h"

string hdd_screen_log = "";
string hdd_screen_red = "";

HddTest::HddTest()
{
}

bool HddTest::hdd_test_all(string hdd_cap)
{
    string result = execute_command("bash " + HDD_TEST_SCRIPT + " " + hdd_cap);
    if (result == "error") {
        LOG_ERROR("%s run error", HDD_TEST_SCRIPT.c_str());
        hdd_screen_log += "ERROR:hdd_test.sh run error\n";
        hdd_screen_red += "\t错误：HDD测试脚本运行失败\n";
    }
    if (check_if_hdd_pass())
        return true;
    return false;
}

bool HddTest::check_if_hdd_pass()
{
    char hdd_status[CMD_BUF_SIZE];
    
    memset(hdd_status, 0, CMD_BUF_SIZE);
    int size = 0;
    if (!get_file_size("/tmp/hdd.status",&size)) {
        LOG_ERROR("/tmp/hdd.status is null\n");
        hdd_screen_log += "ERROR:get hdd status error\n\n";
        hdd_screen_red += "\t错误：HDD状态获取失败\n";
        return false;
    }
    if (!read_local_data("/tmp/hdd.status", hdd_status, size)) {
        LOG_ERROR("/tmp/hdd.status read failed\n");
        hdd_screen_log += "ERROR:get hdd status error\n\n";
        hdd_screen_red += "\t错误：HDD状态获取失败\n";
        return false;
    }
    if (!strcmp(delNL(hdd_status), "SUCCESS")) {
        LOG_INFO("HDD Test result: \tPASS\n");
        return true;
    } else {
        LOG_ERROR("HDD test failed: \t%s\n", hdd_status);
        hdd_screen_log += "HDD test failed:\t" + (string)hdd_status + "\n\n";
        hdd_screen_red += "\t错误：" + (string)hdd_status + "\n";
        return false;
    }
}

void* HddTest::test_all(void *arg)
{
    Control *control = Control::get_control();
    control->set_interface_test_status(HDD_TEST_NAME, false);
    hdd_screen_log += "==================== " + HDD_TEST_NAME + " ====================\n";
    BaseInfo* baseInfo = (BaseInfo *)arg;
    bool result = hdd_test_all(baseInfo->hdd_cap);
    if (result) {
        hdd_screen_log += HDD_TEST_NAME + "结果：\t\t\t成功\n\n";
        control->set_interface_test_result(HDD_TEST_NAME, true); 
    } else {
        hdd_screen_red = HDD_TEST_NAME + "结果：\t\t\t失败\n\n" + hdd_screen_red;
        control->set_interface_test_result(HDD_TEST_NAME, false); 
    }
    control->update_screen_log(hdd_screen_log);
    hdd_screen_log = "";
    if (hdd_screen_red != "") {
        control->update_color_screen_log(hdd_screen_red, "red");
        hdd_screen_red = "";
    }
    control->set_interface_test_status(HDD_TEST_NAME, true);
    return NULL;
}

void HddTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}


