#include "FuncTest.h"
#include "fac_log.h"

string HddTest::screen_log_black = "";
string HddTest::screen_log_red = "";

bool HddTest::hdd_test_all(string hdd_cap)
{
    if (hdd_cap == "") {
        LOG_ERROR("hdd cap is null");
        return false;
    }
    string result = execute_command("bash " + HDD_TEST_SCRIPT + " " + hdd_cap, true);
    if (result == "error") {
        LOG_ERROR("%s run error", HDD_TEST_SCRIPT.c_str());
        screen_log_black += "ERROR:hdd_test.sh run error\n";
        screen_log_red += "\t错误：HDD测试脚本运行失败\n";
    }
    if (check_if_hdd_pass()) {
        return true;
    }
    return false;
}

bool HddTest::check_if_hdd_pass()
{
    char hdd_status[CMD_BUF_SIZE];  //TODO: char[] hdd_status
    
    memset(hdd_status, 0, CMD_BUF_SIZE);
    int size = 0;
    if (!get_file_size(HDD_STATUS_FILE, &size)) {
        LOG_ERROR("%s is null\n", WIFI_SSID_FILE.c_str());
        screen_log_black += "ERROR: get hdd status error\n\n";
        screen_log_red += "\t错误：HDD状态获取失败\n";
        return false;
    }
    if (!read_local_data(HDD_STATUS_FILE, hdd_status, size)) {
        LOG_ERROR("%s read failed\n", HDD_STATUS_FILE.c_str());
        screen_log_black += "ERROR: get hdd status error\n\n";
        screen_log_red += "\t错误：HDD状态获取失败\n";
        return false;
    }
    if (!(delNL(hdd_status).compare("SUCCESS"))) {
        LOG_INFO("HDD Test result: \tPASS\n");
        return true;
    } else {
        LOG_ERROR("HDD test failed: \t%s\n", hdd_status);
        screen_log_black += "HDD test failed:\t" + (string)hdd_status + "\n\n";
        screen_log_red += "\t错误：" + (string)hdd_status + "\n";
        return false;
    }
}

void* HddTest::test_all(void *arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is null");
        return NULL;
    }
    Control *control = Control::get_control();
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_HDD], false);

    screen_log_black = "";
    screen_log_red = "";
    screen_log_black += "==================== " + INTERFACE_TEST_NAME[I_HDD] + " ====================\n";
    BaseInfo* baseInfo = (BaseInfo *)arg;
    bool result = hdd_test_all(baseInfo->hdd_cap);
    if (result) {
        screen_log_black += INTERFACE_TEST_NAME[I_HDD] + "结果：\t\t\t成功\n\n";
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_HDD], true); 
    } else {
        screen_log_red = INTERFACE_TEST_NAME[I_HDD] + "结果：\t\t\t失败\n\n" + screen_log_red;
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_HDD], false); 
    }
    control->update_screen_log(screen_log_black);
    if (screen_log_red != "") {
        control->update_color_screen_log(screen_log_red, "red");
    }
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_HDD], true);
    return NULL;
}

void HddTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}


