#include "FuncTest.h"

pthread_mutex_t g_next_process_lock;

bool CpuTest::is_cpu_test_pass(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is NULL");
        return false;
    }

    Control *control = Control::get_control();
    string hw_cpu_type = control->get_hw_info()->cpu_type; // current system cpu type
    string base_cpu_type = baseInfo->cpu_type; //theoretical cpu type
    /* if hw_cpu_type comtains base_cpu_type,cpu test pass */
    int idx = hw_cpu_type.find(base_cpu_type);
    if (idx != -1 && base_cpu_type != "") {
        screen_log_black += "current cpu type is " + hw_cpu_type + "\n\n";
        LOG_INFO("current cpu type is %s\n", hw_cpu_type.c_str());
        return true;
    } else {
        screen_log_red += "\t错误：CPU型号应为 " + base_cpu_type + ", 但检测到CPU型号为 " + hw_cpu_type + "\n";
        screen_log_black += "cpu type should be\t\t" + base_cpu_type + "\nbut current is\t\t" + hw_cpu_type + "\n";
        LOG_ERROR("cpu type should be %s\tbut current is %s\n", base_cpu_type.c_str(), hw_cpu_type.c_str());
        return false;
    }
}

void CpuTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is NULL");
        return;
    }

    Control *control = Control::get_control();
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_CPU], false);
    screen_log_black = "";
    screen_log_red = "";
    screen_log_black += "==================== " + INTERFACE_TEST_NAME[I_CPU] + " ====================\n";
    if (is_cpu_test_pass(baseInfo)) {
        LOG_INFO("cpu test result:\tPASS\n");
        screen_log_black += INTERFACE_TEST_NAME[I_CPU] + "结果:\t\t\t成功\n\n";
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_CPU], true);
    } else {
        LOG_INFO("cpu test result:\tFAIL\n");
        screen_log_red = INTERFACE_TEST_NAME[I_CPU] + "结果:\t\t\t失败\n\n" + screen_log_red;
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_CPU], false); 
    }
    control->update_color_screen_log(screen_log_black, "black");
    if (screen_log_red != "") {
        control->update_color_screen_log(screen_log_red, "red");
    }
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_CPU], true);
}


string FanTest::screen_log_black = "";
string FanTest::screen_log_red = "";

string FanTest::fan_speed_test(string speed)
{
    if (speed == "") {
        LOG_ERROR("fan speed is null");
        return "FAIL";
    }
    string fan_result = execute_command("bash " + FACTORY_PATH + "fan_test.sh " + speed, true);
    return fan_result;
}

void* FanTest::test_all(void *arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is null");
        return NULL;
    }
    Control *control = Control::get_control();
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_FAN], false);
    BaseInfo* baseInfo = (BaseInfo*)arg;
    
    screen_log_black = "";
    screen_log_red = "";
    screen_log_black += "==================== " + INTERFACE_TEST_NAME[I_FAN] + " ====================\n";
    
    string result = fan_speed_test(baseInfo->fan_speed);
    if (result == "SUCCESS") {
        LOG_INFO("fan test result:\tPASS\n");
        screen_log_black += INTERFACE_TEST_NAME[I_FAN] + "结果:\t\t\t成功\n\n";
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_FAN], true);
    } else {
        screen_log_black += "fan speed should be " + baseInfo->fan_speed + "\tbut current is " + result + "\n\n";
        LOG_ERROR("fan speed should be %s\tbut current is %s\n", (baseInfo->fan_speed).c_str(), result.c_str());
        LOG_INFO("fan test result:\tFAIL\n");
        screen_log_red += "\t错误：风扇转速应达到" + baseInfo->fan_speed + "，但测试只达到" + result + "\n";
        screen_log_red = INTERFACE_TEST_NAME[I_FAN] + "结果:\t\t\t失败\n\n" + screen_log_red;
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_FAN], false);
    }
    control->update_color_screen_log(screen_log_black, "black");
    if (screen_log_red != "") {
        control->update_color_screen_log(screen_log_red, "red");
    }
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_FAN], true);
    return NULL;
}

void FanTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}

void* InterfaceTest::test_all(void *arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is null");
        return NULL;
    }
    BaseInfo* baseInfo = (BaseInfo*)arg;
    Control *control = Control::get_control();
    
    bool* interfaceTestSelectStatus = control->get_infc_func_select_status();
    bool* interfaceTestResult       = control->get_interface_test_result();
    bool* interfaceTestOver         = control->get_interface_test_over();
    bool* interfaceTestFinish       = control->get_interface_test_finish();
    bool* funcFinishStatus          = control->get_func_finish_status();
    
    /* if interface test is running, stop it */
    if (control->get_interface_run_status() == INF_RUNNING) {
        LOG_INFO("******************** stop interface test ********************");
        control->set_interface_run_status(INF_BREAK);
        control->get_ui_handle()->ui_set_interface_test_state(INF_BREAK);
        return NULL;
    }
    
    if (funcFinishStatus[F_INTERFACE]) {
        LOG_INFO("interface test has finished, do not need test again");
        return NULL;
    }

    /* if conf file is wrong, not test */
    if (!control->get_third_product_state()) {
        if (control->get_auto_upload_mes_status() && control->get_fac_config_status() != 0) {
            control->get_ui_handle()->confirm_test_result_warning("配置文件有误");
            return NULL;
        }
    }

    if (control->get_interface_run_status() == INF_RUNEND) {
        LOG_INFO("******************** start interface test ********************");
        control->set_interface_run_status(INF_RUNNING);
        control->get_ui_handle()->ui_set_interface_test_state(INF_RUNNING);
    }    
    
    FuncBase** FuncBase = control->get_funcbase();

    int interfaceTestFailNum[INTERFACE_TEST_NUM] = {0, };
    
    int test_num = control->get_interface_test_times();
    int real_test_num = 0;
    int interface_run_status = INF_RUNNING;
    for (int i = 0; i < test_num || test_num == 0; i++) {
        interface_run_status = control->get_interface_run_status();
        if (interface_run_status == INF_BREAK) {
            break;
        }

        real_test_num = i + 1;
        string loop = "\n******************** LOOP: " + to_string(i + 1) + " ********************";
        control->update_color_screen_log(loop, "black");
        
        for (int j = 0; j < INTERFACE_TEST_NUM; j++) {
            if (interfaceTestSelectStatus[j]) {
                string log_info = "---------- start " + INTERFACE_TEST_MES_TAG[j] + " test ----------\n";
                LOG_INFO(log_info.c_str());
                FuncBase[j]->start_test(baseInfo);
            }
        }
        
        while (1) {
            sleep(1);
            bool tmp_test_over = true;
            for (int i = 0; i < INTERFACE_TEST_NUM; i++) {
                tmp_test_over &= interfaceTestOver[i];
                if (!tmp_test_over) {
                    break;
                }
            }
            if (tmp_test_over) {
                for (int i = 0; i < INTERFACE_TEST_NUM; i++) {
                    if (!interfaceTestResult[i]) {
                        interfaceTestFailNum[i]++;
                    }
                }
                break;
            }
        }
    }

    string total_result = "=============== " + FUNC_TEST_NAME[F_INTERFACE] + "结果 ===============\n";
    for (int i = 0; i < INTERFACE_TEST_NUM; i++) {
        if (interfaceTestSelectStatus[i]) {
            if (interfaceTestFailNum[i] == 0) {
                total_result += INTERFACE_TEST_MES_TAG[i] + "\tPASS (Time:" + to_string(real_test_num) + ",ERROR:0)\n";
                control->update_mes_log(INTERFACE_TEST_MES_TAG[i], "PASS");
                control->set_func_test_result(INTERFACE_TEST_NAME[i], "PASS");
                control->set_interface_test_finish(INTERFACE_TEST_NAME[i], true);
            } else {
                control->update_mes_log(INTERFACE_TEST_MES_TAG[i], "FAIL");
                control->set_func_test_result(INTERFACE_TEST_NAME[i], "FAIL");
                control->set_interface_test_finish(INTERFACE_TEST_NAME[i], false);
                total_result += INTERFACE_TEST_MES_TAG[i] + "\tFAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                                   + to_string(interfaceTestFailNum[i]) + ")\n";
            }
        }
    }
    total_result += "===============================================";
    control->update_color_screen_log(total_result, "black");
    
    /* all interface func test finish,interface test finish */
    bool tmp_test_finish = true;
    for (int i = 0; i < INTERFACE_TEST_NUM; i++) {
        tmp_test_finish &= interfaceTestFinish[i];
        if (!tmp_test_finish) {
            break;
        }
    }
    if (tmp_test_finish) {
         funcFinishStatus[F_INTERFACE] = true;
    }
    
    control->set_interface_run_status(INF_RUNEND);
    control->get_ui_handle()->ui_set_interface_test_state(INF_RUNEND);
    return NULL;
}

void InterfaceTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}

void* PowerTest::test_all(void*)
{
    Control *control = Control::get_control();
    control->update_color_screen_log("==================== " + FUNC_TEST_NAME[F_POWER] + " ====================", "black");

    control->show_test_confirm_dialog(FUNC_TEST_NAME[F_POWER]);
    
    return NULL;
}

void PowerTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}

int  StressTest::mem_stress_test_num = 0;
bool StressTest::mem_stress_status = false;
bool StressTest::mem_stress_result = true;
string StressTest::stress_result = "";

void* StressTest::gpu_stress_test(void*)
{
    pthread_detach(pthread_self());
    stop_gpu_stress_test();
    if (system("cd /usr/bin/Unigine_Heaven-4.0; ./heaven") < 0) {
        LOG_ERROR("system cmd run error\n");
    }
    return NULL;
}

void* StressTest::camera_stress_test(void* arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is NULL");
        return NULL;
    }
    pthread_detach(pthread_self());
    CameraTest* camera = (CameraTest*)arg;
    camera->start_camera_xawtv_on_stress();
    return NULL;
}

bool StressTest::start_cpuburn_stress()
{
    string result;
    int ret;
    int processornum = 0;

    LOG_INFO("start cpuburn");
    result = execute_command("cat /proc/cpuinfo| grep \"processor\"| wc -l", true);
    if (result != "error") {
        LOG_INFO("cpuprocessor num is %s\n", result.c_str());
        processornum = strtoul(result.c_str(), NULL, 16);
    }

    stop_cpuburn_stress();

    if (processornum > 32) {
        processornum = 32;  //TODO: processornum is less than 32
    }
    while(processornum-- > 0) {
        string cmd_burn = "";
        cmd_burn = "taskset 0x" + to_string(1 << (processornum)) + " burnP6 &";
        LOG_INFO("cmd:%s\n", cmd_burn.c_str());
        ret = system(cmd_burn.c_str());
 
        if (ret < 0) {
            LOG_ERROR("cmd run \"burnP6 &\" error!!!\n");
            return false;
        }
    }
    return true;
}

void StressTest::stop_cpuburn_stress()
{
    LOG_INFO("stop cpuburn");
    int ret = 0;
    ret = system("killall burnP6");
    if (ret < 0) {
        LOG_ERROR("cmd run \"killall burnP6\" error!!!\n");
    }
}

void* StressTest::mem_stress_test(void*)
{
    pthread_detach(pthread_self());
    mem_stress_status = true; // flag that is mem stress test starting
    mem_stress_test_num++;
    LOG_INFO("---------- start stress mem test NO.%d ----------\n", mem_stress_test_num);
    stop_mem_stress_test();
    
    string free_mem_cap = execute_command("free -m | awk '/Mem/ {print $4}'", true);
    if (free_mem_cap == "error") {
        LOG_ERROR("get free mem cap error\n");
        mem_stress_result = false;
        mem_stress_status = false;
        return NULL;
    }
    
    int test_mem_cap = (int)(get_int_value(free_mem_cap.c_str()) * STRESS_MEM_PERCENT);
    if (test_mem_cap > STRESS_MEM_CAP_MAX) {
        test_mem_cap = STRESS_MEM_CAP_MAX;
    }
    
    string result = execute_command("bash " + MEM_TEST_SCRIPT + " " + to_string(test_mem_cap) + "M", true);
    if (result == "SUCCESS") {
        LOG_INFO("mem stress test result:\tPASS\n");
    } else {
        LOG_ERROR("mem stress test result:\tfailed\n");
        mem_stress_result = false;
    }
    mem_stress_status = false; // mem stress test finished
    
    return NULL;
}

void StressTest::stop_mem_stress_test()
{
    if (system("killall -s 9 memtester") < 0) {
        LOG_ERROR("kill memtester error");
    }
}

bool StressTest::create_abnormal_exit_num_file(int* num)
{
    if (num == NULL) {
        LOG_ERROR("num is NUMM");
        return false;
    }

    if (check_file_exit(STRESS_DOWN_NUM)) {
        string exit_num = execute_command("cat " + STRESS_DOWN_NUM, true);
        if (exit_num == "error") {
            LOG_ERROR("read abnormal exit");
            return false;
        }
        *num = get_int_value(exit_num) + 1;
    } else {
        (*num)++;
    }
    
    string chr_num = to_string(*num);
    if (!write_local_data(STRESS_DOWN_NUM, "w+", chr_num.c_str(), chr_num.size())) {
        LOG_ERROR("write abnormal exit num file error");
        return false;
    }
    
    if (system("sync") < 0) {
        LOG_ERROR("cmd sync error\n");
        return false;
    }
    
    if (check_file_exit(STRESS_DOWN_NUM)) {
        LOG_INFO("create stress abnormal exit num file success\n");
        return true;
    } else {
        LOG_ERROR("create stress abnormal exit num file failed\n");
        return false;
    }

}

void* StressTest::test_all(void* arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is null");
        return NULL;
    }
    BaseInfo* baseInfo   = (BaseInfo*)arg;
    Control*  control    = Control::get_control();
    UiHandle* uihandle   = UiHandle::get_uihandle();
    
    TimeInfo init_time   = {0,0,0,0};
    TimeInfo tmp_dst     = {0,0,0,0};
    TimeInfo mem_src     = {0,0,0,0};
    TimeInfo mem_dst     = {0,0,0,0};
    TimeInfo cpuburn_src = {0,0,0,0};
    TimeInfo cpuburn_dst = {0,0,0,0};
    
    string datebuf = "";
    CpuStatus st_cpu = {0,0,0,0,0,0,0,0,0,0,0};
    pthread_t pid_camera, pid_gpu, pid_mem;

    stress_result = ""; 
    string mem_result_str = "NULL";
    mem_stress_test_num = 0;
    mem_stress_status = false;
    mem_stress_result = true;
    bool encode = true;
    bool decode = true;

    int abnormal_exit = 0;
    bool cpuburn_test = true;
    FuncBase** _funcBase = control->get_funcbase();
    CameraTest* camera = (CameraTest*)_funcBase[CAMERA];

    control->set_pcba_whole_lock_state(false); //
    /* confirm if last stress abnormal exited */
    if (check_file_exit(STRESS_LOCK_FILE)) {
        string stress_lock_state = execute_command("cat " + STRESS_LOCK_FILE, true); // confirm data in lock file
        LOG_INFO("auto stress lock file is: %s", stress_lock_state.c_str());
        remove_local_file(STRESS_LOCK_FILE);
        
        if (stress_lock_state == WHOLE_LOCK || stress_lock_state == PCBA_LOCK) { // abnormal exited 
            control->set_pcba_whole_lock_state(true);
            /* create file to save abnormal exit number */
            if (!create_abnormal_exit_num_file(&abnormal_exit)) {
                uihandle->confirm_test_result_warning("计数文件创建异常");
                LOG_ERROR("create stress abnormal exit num file failed\n");
                return NULL;
            }
            LOG_INFO("stress test exit error: %d times\n", abnormal_exit);

        } else if (stress_lock_state == NEXT_LOCK) { // next process exited
            LOG_INFO("next process -> stress test\n");
        } else { // unknown lock file
            uihandle->confirm_test_result_warning("lock文件异常");
            LOG_ERROR("stress test lock file wrong\n");
            return NULL;
        }
    }
    /* confirm if lock file create success */
    if (!create_stress_test_lock(false)) {
        uihandle->confirm_test_result_warning("lock文件创建异常");
        LOG_ERROR("create stress test lock failed\n");
        return NULL;
    }
    control->set_stress_test_window_quit_status(true); // show stress window
    uihandle->show_stress_test_ui();
    
    if (get_int_value(baseInfo->camera_exist) == 1) {
        pthread_create(&pid_camera, NULL, camera_stress_test, camera); // if camera exist, open camera window
    }

    uihandle->update_stress_label_value("产品型号", (control->get_hw_info())->product_name);
    uihandle->update_stress_label_value("硬件版本", (control->get_hw_info())->product_hw_version);
    uihandle->update_stress_label_value("SN序列号", (control->get_hw_info())->sn);
    uihandle->update_stress_label_value("MAC地址", (control->get_hw_info())->mac);
    uihandle->update_stress_label_value("Mem压力测试", mem_result_str);
    uihandle->update_stress_label_value("编码状态", STRING_RESULT(encode));
    uihandle->update_stress_label_value("解码状态", STRING_RESULT(decode));

    if (baseInfo->platform == "IDV") {
        pthread_create(&pid_gpu, NULL, gpu_stress_test, NULL); // IDV need test GPU
    }
    start_cpuburn_stress();
    
    get_current_open_time(&init_time);
    cpuburn_src = init_time;
    mem_src     = init_time;
    while(true) {
        /* Press the right mouse button to exit stress test */
        if (!control->is_stress_test_window_quit_safely()) {
            if (baseInfo->platform == "IDV") {
                stop_gpu_stress_test();
            }
            stop_cpuburn_stress();
            stop_mem_stress_test();
            if (get_int_value(baseInfo->camera_exist) == 1) {
                camera->close_xawtv_window();
            }
            if (check_file_exit(STRESS_LOCK_FILE)) {
                remove_local_file(STRESS_LOCK_FILE); // delete lock file
            }
            if (check_file_exit(STRESS_DOWN_NUM)) {
                remove_local_file(STRESS_DOWN_NUM); // delete power down number file
            }
            break;
        }
        
        get_current_open_time(&tmp_dst);
        cpuburn_dst = tmp_dst;
        mem_dst     = tmp_dst;
        
        diff_running_time(&tmp_dst, &init_time); // get stress test time
        if (STRESS_TIME_ENOUGH(tmp_dst)) { // after 4 hour, show stress result, delete the lock file and power down number file
            remove_local_file(STRESS_LOCK_FILE);
            if (check_file_exit(STRESS_DOWN_NUM)) {
                remove_local_file(STRESS_DOWN_NUM); // delete power down number file
            }
            if (encode && decode && mem_stress_result) {
                uihandle->set_stress_test_pass_or_fail("PASS");
            } else {
                uihandle->set_stress_test_pass_or_fail("FAIL");
            }
        }
        
        /* If the last stress test is abnormally powered down, a warning box is displayed. */
        if (control->get_pcba_whole_lock_state() && STRESS_ERROR_TIME(tmp_dst)) {
            uihandle->confirm_test_result_warning("拷机异常退出：" + to_string(abnormal_exit) + "次");
        }

        /* cpuburn test again every 30 mins */
        diff_running_time(&cpuburn_dst, &cpuburn_src); // get stress test time
        if (STRESS_HALF_HOUR_TEST(cpuburn_dst)) {
            if (cpuburn_test) {
                cpuburn_test = false;
                cpuburn_src = mem_dst; // get current time
                mem_src = mem_dst;
                stop_cpuburn_stress(); // stop cpuburn
                pthread_create(&pid_mem, NULL, mem_stress_test, NULL); // start memtester
            } else {
                cpuburn_test = true;
                cpuburn_src = mem_dst; // get current time
                stop_mem_stress_test();
                start_cpuburn_stress();
            }
        } 

        /* memtester test again every 10 mins after cpuburn test stoped */
        diff_running_time(&mem_dst, &mem_src);
        if (!cpuburn_test && STRESS_MEMTEST_ITV(mem_dst) && !mem_stress_status) {
            get_current_open_time(&mem_src);
            pthread_create(&pid_mem, NULL, mem_stress_test, NULL);
        }
        
        /* After the first test, update the mem test result NULL to PASS/FAIL */
        if (mem_stress_test_num == 1 && !mem_stress_status && mem_stress_result && mem_result_str == "NULL") {
            mem_result_str = "PASS";
            uihandle->update_stress_label_value("Mem压力测试", mem_result_str);
        } else if (!mem_stress_result && mem_result_str != "FAIL"){
            /* If the mem test fails, set the mem and the stress test result to fail */
            mem_result_str = "FAIL";
            uihandle->update_stress_label_value("Mem压力测试", mem_result_str);
            uihandle->set_stress_test_pass_or_fail("FAIL");
        }
        
        encode = true;
        decode = control->get_decode_status(); // get decode status
        /* If the decoding fails, set the decoding and the stress test result to fail */
        if (!decode) {
            uihandle->update_stress_label_value("解码状态", "FAIL");
            uihandle->set_stress_test_pass_or_fail("FAIL");
        }
        /* updating stress test result */
        datebuf = to_string(tmp_dst.day) + "天" + to_string(tmp_dst.hour) + "小时"
                + to_string(tmp_dst.minute) + "分" + to_string(tmp_dst.second) + "秒";
        stress_result = "运行时间:" + datebuf + "  编码状态:" + (string)STRING_RESULT(encode) + "  解码状态:"
                + (string)STRING_RESULT(decode) + "  Mem压力测试:" + mem_result_str + "\n";
        /* updating system info */
        uihandle->update_stress_label_value("运行时间", datebuf);
        uihandle->update_stress_label_value("CPU温度", execute_command("bash " + GET_CPU_TEMP_SCRIPT, false));
        uihandle->update_stress_label_value("CPU频率", get_current_cpu_freq());
        uihandle->update_stress_label_value("Mem", get_mem_info());
        uihandle->update_stress_label_value("Cpu", get_cpu_info(&st_cpu));
        
        sleep(1);
    }
    return NULL;
}

void StressTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}

string StressTest::get_stress_result_record()
{
    return stress_result;
}

void UploadMesLog::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}

void* UploadMesLog::test_all(void*)
{
    Control* control = Control::get_control();
    control->upload_mes_log();
    return NULL;
}

void NextProcess::next_process_handle(BaseInfo* baseInfo) 
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is NULL");
        return;
    }
    Control* control = Control::get_control();
    UiHandle* uihandle = UiHandle::get_uihandle();
    int next_process_f = -1;
    
    pthread_detach(pthread_self());    
    if (pthread_mutex_trylock(&g_next_process_lock)) {
        LOG_ERROR("g_next_process_lock has been locked\n");
        return;
    }

    uihandle->confirm_test_result_waiting("正在处理，请等待..."); // waiting box
    /* When it is IDV and emmc exists, handle bache */
    if (baseInfo->platform == "IDV" && baseInfo->emmc_cap != "0" && baseInfo->emmc_cap != "") {
        next_process_f = system("bash /etc/diskstatus_mgr.bash --product-detach");
        usleep(1000000);
        LOG_INFO("bache check result value is %d\n", WEXITSTATUS(next_process_f));

        if (WEXITSTATUS(next_process_f) > 0) {
            LOG_ERROR("The disk is abnormal and cannot enter the next process.\n");
            uihandle->confirm_test_result_warning("磁盘异常，无法进入下道工序");
            control->update_color_screen_log("磁盘异常，无法进入下道工序", "red");
            pthread_mutex_unlock(&g_next_process_lock);
            return;
        }
    }
    pthread_mutex_unlock(&g_next_process_lock);
    /* create stress lock file, and then shoutdown */
    if (!create_stress_test_lock(true)) {
        LOG_ERROR("create stress test lock fail!\n");
        uihandle->confirm_test_result_warning("文件异常，无法关机！");
        control->update_color_screen_log("文件异常，无法关机！", "red");
    } else if (execute_command("shutdown -h now", true) == "error") {
        LOG_ERROR("shutdown cmd run error\n");
        uihandle->confirm_test_result_warning("终端异常，无法关机！");
        control->update_color_screen_log("终端异常，无法关机！", "red");
    }
 
    return;
}

void* NextProcess::test_all(void* arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is null");
        return NULL;
    }
    BaseInfo* baseInfo = (BaseInfo*)arg;
    next_process_handle(baseInfo);
    return NULL;
}

void NextProcess::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is NULL");
        return;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}

void NextProcess::init(){
    pthread_mutex_init(&g_next_process_lock, NULL);
}


