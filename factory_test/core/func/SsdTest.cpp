#include "SsdTest.h"
#include "fac_log.h"

string SsdTest::screen_log_black = "";
string SsdTest::screen_log_red = "";

bool SsdTest::ssd_test_all(string ssd_cap)
{
    string result = execute_command("bash " + SSD_TEST_SCRIPT + " " + ssd_cap, true);
    if (result == "error") {
        LOG_ERROR("%s run error", SSD_TEST_SCRIPT.c_str());
        screen_log_black += "ERROR:ssd_test.sh run error\n";
        screen_log_red += "\t错误：HDD测试脚本运行失败\n";
    }
    if (check_if_ssd_pass()) {
        return true;
    }
    return false;
}

bool SsdTest::check_if_ssd_pass()   //TODO
{
    char ssd_status[CMD_BUF_SIZE];
    
    memset(ssd_status, 0, CMD_BUF_SIZE);
    int size = 0;
    if (!get_file_size(SSD_STATUS_FILE, &size)) {
        LOG_ERROR("%s is null\n", SSD_STATUS_FILE.c_str());
        screen_log_black += "ERROR:get ssd status error\n\n";
        screen_log_red += "\t错误：HDD状态获取失败\n";
        return false;
    }
    if (!read_local_data(SSD_STATUS_FILE, ssd_status, size)) {
        LOG_ERROR("%s read failed\n", SSD_STATUS_FILE.c_str());
        screen_log_black += "ERROR:get ssd status error\n\n";
        screen_log_red += "\t错误：HDD状态获取失败\n";
        return false;
    }
    if (!(delNL(ssd_status).compare("SUCCESS"))) {
        LOG_INFO("SSD Test result: \tPASS\n");
        return true;
    } else {
        LOG_ERROR("SSD test failed: \t%s\n", ssd_status);
        screen_log_black += "SSD test failed:\t" + (string)ssd_status + "\n\n";
        screen_log_red += "\t错误：" + (string)ssd_status + "\n";
        return false;
    }
}

void* SsdTest::test_all(void *arg)
{
    Control *control = Control::get_control();
    control->set_interface_test_status(SSD_TEST_NAME, false);

    screen_log_black = "";
    screen_log_red = "";
    screen_log_black += "==================== " + SSD_TEST_NAME + " ====================\n";
    BaseInfo* baseInfo = (BaseInfo *)arg;
    bool result = ssd_test_all(baseInfo->ssd_cap);
    if (result) {
        screen_log_black += SSD_TEST_NAME + "结果：\t\t\t成功\n\n";
        control->set_interface_test_result(SSD_TEST_NAME, true); 
    } else {
        screen_log_red = SSD_TEST_NAME + "结果：\t\t\t失败\n\n" + screen_log_red;
        control->set_interface_test_result(SSD_TEST_NAME, false); 
    }
    control->update_screen_log(screen_log_black);
    if (screen_log_red != "") {
        control->update_color_screen_log(screen_log_red, "red");
    }
    control->set_interface_test_status(SSD_TEST_NAME, true);
    return NULL;
}

void SsdTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}


