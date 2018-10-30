#include "../../inc/FuncTest.h"
#include "../../inc/fac_log.h"

pthread_mutex_t g_next_process_lock;

bool CpuTest::is_cpu_test_pass(BaseInfo* baseInfo)
{
    Control *control = Control::get_control();
    string hw_cpu_type = control->get_hw_info()->cpu_type;
    string base_cpu_type = baseInfo->cpu_type;
    string::size_type idx;
    idx = hw_cpu_type.find(base_cpu_type);
    if (idx != string::npos && base_cpu_type != "") {
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
    Control *control = Control::get_control();
    control->set_interface_test_status(CPU_TEST_NAME, false);
    screen_log_black = "";
    screen_log_red = "";
    screen_log_black += "==================== " + CPU_TEST_NAME + " ====================\n";
    if (is_cpu_test_pass(baseInfo)) {
        LOG_INFO("cpu test result:\tPASS\n");
        screen_log_black += CPU_TEST_NAME + "结果:\t\t\t成功\n\n";
        control->set_interface_test_result(CPU_TEST_NAME, true);
    } else {
        LOG_INFO("cpu test result:\tFAIL\n");
        screen_log_red = CPU_TEST_NAME + "结果:\t\t\t失败\n\n" + screen_log_red;
        control->set_interface_test_result(CPU_TEST_NAME, false); 
    }
    control->update_screen_log(screen_log_black);
    if (screen_log_red != "") {
        control->update_color_screen_log(screen_log_red, "red");
    }
    control->set_interface_test_status(CPU_TEST_NAME, true);
}


string FanTest::screen_log_black = "";
string FanTest::screen_log_red = "";

string FanTest::fan_speed_test(string speed)
{
    string fan_result = execute_command("bash " + FACTORY_PATH + "fan_test.sh " + speed);
    return fan_result;
}

void* FanTest::test_all(void *arg)
{
    Control *control = Control::get_control();
    control->set_interface_test_status(FAN_TEST_NAME, false);
    BaseInfo* baseInfo = (BaseInfo*)arg;
    
    screen_log_black = "";
    screen_log_red = "";
    screen_log_black += "==================== " + FAN_TEST_NAME + " ====================\n";
    
    string result = fan_speed_test(baseInfo->fan_speed);
    if (result == "SUCCESS") {
        LOG_INFO("fan test result:\tPASS\n");
        screen_log_black += FAN_TEST_NAME + "结果:\t\t\t成功\n\n";
        control->set_interface_test_result(FAN_TEST_NAME, true);
    } else {
        screen_log_black += "fan speed should be " + baseInfo->fan_speed + "\tbut current is " + result + "\n\n";
        LOG_ERROR("fan speed should be %s\tbut current is %s\n", (baseInfo->fan_speed).c_str(), result.c_str());
        LOG_INFO("fan test result:\tFAIL\n");
        screen_log_red += "\t错误：风扇转速应达到" + baseInfo->fan_speed + "，但测试只达到" + result + "\n";
        screen_log_red = FAN_TEST_NAME + "结果:\t\t\t失败\n\n" + screen_log_red;
        control->set_interface_test_result(FAN_TEST_NAME, false);
    }
    control->update_screen_log(screen_log_black);
    if (screen_log_red != "") {
        control->update_color_screen_log(screen_log_red, "red");
    }
    control->set_interface_test_status(FAN_TEST_NAME, true);
    return NULL;
}

void FanTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}


int  StressTest::mem_stress_num = 0;
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

    result = execute_command("cat /proc/cpuinfo| grep \"processor\"| wc -l");
    if (result != "error") {
        LOG_INFO("cpuprocessor num is %s\n", result.c_str());
        processornum = strtoul(result.c_str(), NULL, 16);
    }

    stop_cpuburn_stress();

    if (processornum > 32) {
        processornum = 32;
    }
    while(processornum-- > 0) {
        char cmd_burn[CMD_BUF_SIZE];
        memset(cmd_burn, 0, CMD_BUF_SIZE);
        sprintf (cmd_burn, "taskset 0x%d burnP6 &", (1 << (processornum)));
        LOG_INFO("cmd:%s\n", cmd_burn);
        ret = system(cmd_burn);
 
        if (ret < 0) {
            LOG_ERROR("cmd run \"burnP6 &\" error!!!\n");
            return false;
        }
    }
    return true;
}

void StressTest::stop_cpuburn_stress()
{
    int ret;
    ret = system("killall burnP6");
    if (ret < 0) {
        LOG_ERROR("cmd run \"killall burnP6\" error!!!\n");
    }
}

void* StressTest::mem_stress_test(void*)
{
    pthread_detach(pthread_self());
    mem_stress_status = true;
    mem_stress_num++;
    LOG_INFO("---------- start stress mem test NO.%d ----------\n", mem_stress_num);
    stop_mem_stress_test();
    
    string free_mem_cap = execute_command("free -m | awk '/Mem/ {print $4}'");
    if (free_mem_cap == "error") {
        LOG_ERROR("get free mem cap error\n");
        mem_stress_result &= false;
        mem_stress_status = false;
        return NULL;
    }
    
    int test_mem_cap = (int)(get_int_value(free_mem_cap.c_str()) * STRESS_MEM_PERCENT);
    if (test_mem_cap > STRESS_MEM_CAP_MAX) {
        test_mem_cap = STRESS_MEM_CAP_MAX;
    }
    
    string result = execute_command("bash " + MEM_TEST_SCRIPT + " " + to_string(test_mem_cap) + "M");
    if (result == "SUCCESS") {
        LOG_INFO("mem stress test result:\tPASS\n");
        mem_stress_result &= true;
    } else {
        LOG_ERROR("mem stress test result:\tfailed\n");
        mem_stress_result &= false;
    }
    mem_stress_status = false;
    
    return NULL;
}

void StressTest::stop_mem_stress_test()
{
    if (system("killall -s 9 memtester") < 0) {
        LOG_ERROR("kill memtester error");
    }
}

void* StressTest::test_all(void* arg)
{
    BaseInfo* baseInfo = (BaseInfo*)arg;
    Control*  control  = Control::get_control();
    UiHandle* uihandle = UiHandle::get_uihandle();
    
    TimeInfo init_time = {0,0,0,0};
    TimeInfo tmp_dst   = {0,0,0,0};
    TimeInfo mem_src   = {0,0,0,0};
    TimeInfo mem_dst   = {0,0,0,0};
    
    char datebuf[CMD_BUF_SIZE] = {0};
    CpuStatus st_cpu = {0,0,0,0,0,0,0,0,0,0,0};
    pthread_t pid_t1, pid_t2;

    stress_result = ""; 
    string mem_result_str = "NULL";
    mem_stress_num = 0;
    mem_stress_status = false;
    mem_stress_result = true;
    bool encode = true;
    bool decode = true;
    
    FuncBase** _funcBase = control->get_funcbase();
    CameraTest* camera = (CameraTest*)_funcBase[CAMERA];

    control->set_pcba_whole_lock_state(false);
    if (check_file_exit(STRESS_LOCK_FILE.c_str())) {
        string stress_lock_state = execute_command("cat " + STRESS_LOCK_FILE);
        LOG_INFO("auto stress lock file is: %s", stress_lock_state.c_str());
        remove_local_file(STRESS_LOCK_FILE.c_str());
        if (stress_lock_state == WHOLE_LOCK || stress_lock_state == PCBA_LOCK) {
            control->set_pcba_whole_lock_state(true);
            LOG_INFO("last stress test exit error\n");            
        } else if (stress_lock_state == NEXT_LOCK) {
            LOG_INFO("next process -> stress test\n");
        } else {
            uihandle->confirm_test_result_warning("lock文件异常");
            LOG_ERROR("stress test lock file wrong\n");
            return NULL;
        }
    }
    
    if (check_file_exit(WHOLE_TEST_FILE)) {
        write_local_data(STRESS_LOCK_FILE.c_str(), "w+", (char*)WHOLE_LOCK, sizeof(WHOLE_LOCK));
    } else {
        write_local_data(STRESS_LOCK_FILE.c_str(), "w+", (char*)PCBA_LOCK, sizeof(PCBA_LOCK));
    }

    if (execute_command("sync") == "error") {
        uihandle->confirm_test_result_warning("系统同步失败");
        LOG_ERROR("cmd sync error\n");
        return NULL;
    }
    
    if (!check_file_exit(STRESS_LOCK_FILE.c_str())) {
        uihandle->confirm_test_result_warning("lock文件创建异常");
        LOG_ERROR("create stress test lock failed\n");
        return NULL;
    }
    control->set_stress_test_window_quit_status(true);
    uihandle->show_stress_test_ui();
    
    if (get_int_value(baseInfo->camera_exist) == 1) {
        pthread_create(&pid_t1, NULL, camera_stress_test, camera);
    }

    uihandle->update_stress_label_value("产品型号", (control->get_hw_info())->product_name);
    uihandle->update_stress_label_value("硬件版本", (control->get_hw_info())->product_hw_version);
    uihandle->update_stress_label_value("SN序列号", (control->get_hw_info())->sn);
    uihandle->update_stress_label_value("MAC地址", (control->get_hw_info())->mac);
    uihandle->update_stress_label_value("Mem压力测试", mem_result_str);
    uihandle->update_stress_label_value("编码状态", PRINT_RESULT_STR(encode));
    uihandle->update_stress_label_value("解码状态", PRINT_RESULT_STR(decode));

    if (baseInfo->platform == "IDV") {
        pthread_create(&pid_t2, NULL, gpu_stress_test, NULL);
    }
    start_cpuburn_stress();
    
    get_current_open_time(&init_time);
    while(true) {
        if (!control->is_stress_test_window_quit_safely()) {
            if (baseInfo->platform == "IDV") {
                stop_gpu_stress_test();
            }
            stop_cpuburn_stress();
            stop_mem_stress_test();
            if (get_int_value(baseInfo->camera_exist) == 1) {
                camera->close_xawtv_window();
            }
            if (check_file_exit(STRESS_LOCK_FILE.c_str())) {
                remove_local_file(STRESS_LOCK_FILE.c_str());
            }

            break;
        }
        
        get_current_open_time(&tmp_dst);
        mem_dst = tmp_dst;
        diff_running_time(&tmp_dst, &init_time);
        if (STRESS_TIME_ENOUGH(tmp_dst)) {
            remove_local_file(STRESS_LOCK_FILE.c_str());
            if (encode && decode && mem_stress_result) {
                uihandle->set_stress_test_pass_or_fail("PASS");
            } else {
                uihandle->set_stress_test_pass_or_fail("FAIL");
            }
        }
        
        if (control->get_pcba_whole_lock_state() && STRESS_ERROR_TIME(tmp_dst)) {
            uihandle->confirm_test_result_warning("上次拷机退出异常");
        }

        if (STRESS_MEMTEST_START(tmp_dst) && !mem_stress_status) {
            get_current_open_time(&mem_src);
            pthread_create(&pid_t1, NULL, mem_stress_test, NULL);
        }

        diff_running_time(&mem_dst, &mem_src);
        if (STRESS_MEMTEST_ITV(mem_dst) && !mem_stress_status) {
            get_current_open_time(&mem_src);
            pthread_create(&pid_t1, NULL, mem_stress_test, NULL);
        }

        if (mem_stress_num == 1 && !mem_stress_status && mem_stress_result) {
            mem_result_str = "PASS";
            uihandle->update_stress_label_value("Mem压力测试", mem_result_str);
        } else if (!mem_stress_result){
            mem_result_str = "FAIL";
            uihandle->update_stress_label_value("Mem压力测试", mem_result_str);
            uihandle->set_stress_test_pass_or_fail("FAIL");
        }
        
        encode = true;
        decode = control->get_decode_status();

        if (!decode) {
            uihandle->update_stress_label_value("解码状态","FAIL");
            uihandle->set_stress_test_pass_or_fail("FAIL");
        }

        stress_result = "运行时间:" + to_string(tmp_dst.day) + "天" + to_string(tmp_dst.hour)
                        + "小时" + to_string(tmp_dst.minute) + "分" + to_string(tmp_dst.second)
                        + "秒  编码状态:" + (string)PRINT_RESULT_STR(encode) + "  解码状态:"
                        + (string)PRINT_RESULT_STR(decode) + "  Mem压力测试:" + mem_result_str + "\n";          

        snprintf(datebuf, CMD_BUF_SIZE, "%d天%d时%d分%d秒", tmp_dst.day, tmp_dst.hour, tmp_dst.minute, tmp_dst.second);
        uihandle->update_stress_label_value("运行时间", datebuf);
        
        uihandle->update_stress_label_value("CPU温度", execute_command_err_log("bash " + GET_CPU_TEMP_SCRIPT));
        uihandle->update_stress_label_value("CPU频率", get_current_cpu_freq());
        uihandle->update_stress_label_value("Mem", get_mem_info());
        uihandle->update_stress_label_value("Cpu", get_cpu_info(&st_cpu));
                
        sleep(1);
    }    
    return NULL;
}

void StressTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}

string StressTest::get_stress_result_record()
{
    return stress_result;
}


bool NextProcess::create_stress_test_lock() 
{
    LOG_INFO("start creating stress lock\n");
    write_local_data(STRESS_LOCK_FILE.c_str(), "w+", (char*)NEXT_LOCK, sizeof(NEXT_LOCK));

    if (check_file_exit(STRESS_LOCK_FILE.c_str())) {
        LOG_INFO("create stress test lock success\n");
        return true;
    } else {
        LOG_ERROR("create stress test lock failed\n");
        return false;
    }
}

void NextProcess::next_process_handle(BaseInfo* baseInfo) 
{
    Control* control = Control::get_control();
    UiHandle* uihandle = UiHandle::get_uihandle();
    int next_process_f = -1;
    
    pthread_detach(pthread_self());    
    if (pthread_mutex_trylock(&g_next_process_lock)) {
        LOG_ERROR("g_next_process_lock has been locked\n");
        return;
    }

    uihandle->confirm_test_result_waiting("正在处理，请等待...");

    if (baseInfo->emmc_cap != "0" && baseInfo->emmc_cap != "") {
        next_process_f = system("bash /etc/diskstatus_mgr.bash --product-detach");
        usleep(1000000);
        LOG_INFO("bache check result value is %d\n", WEXITSTATUS(next_process_f));

        if (WEXITSTATUS(next_process_f) > 0) {
            LOG_ERROR("The disk is abnormal and cannot enter the next process.\n");
            uihandle->confirm_test_result_warning("磁盘异常，无法进入下道工序");
            control->update_screen_log("磁盘异常，无法进入下道工序");
            pthread_mutex_unlock(&g_next_process_lock);
            return;
        }
    } 
    pthread_mutex_unlock(&g_next_process_lock);
    
    if (!create_stress_test_lock()) {
        LOG_ERROR("create stress test lock fail!\n");
        uihandle->confirm_test_result_warning("EMMC异常，无法关机！");
        control->update_screen_log("EMMC异常，无法关机！");
    } else if (execute_command("shutdown -h now") == "error") {
        LOG_ERROR("shutdown cmd run error\n");
        uihandle->confirm_test_result_warning("终端异常，无法关机！");
        control->update_screen_log("终端异常，无法关机！");
    }
 
    return;
}

void* NextProcess::test_all(void* arg)
{
    BaseInfo* baseInfo = (BaseInfo*)arg;
    next_process_handle(baseInfo);
    return NULL;
}

void NextProcess::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}

void NextProcess::init(){
    pthread_mutex_init(&g_next_process_lock, NULL);
}


void UploadMesLog::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}

void* UploadMesLog::test_all(void*)
{
    Control* control = Control::get_control();
    control->upload_mes_log();
    return NULL;
}


void* InterfaceTest::test_all(void *arg)
{
    BaseInfo* baseInfo = (BaseInfo*)arg;
    Control *control = Control::get_control();

    if (control->get_interface_run_status() == INF_RUNNING) {
        LOG_INFO("******************** stop interface test ********************");
        control->set_interface_run_status(INF_BREAK);
        control->get_ui_handle()->ui_set_interface_test_state(INF_BREAK);
        return NULL;
    }
    
    FuncFinishStatus* funcFinishStatus = control->get_func_finish_status();
    if (funcFinishStatus->interface_finish) {
        LOG_INFO("interface test has finished, do not need test again");
        return NULL;
    }

    if (control->get_auto_upload_mes_status() && control->get_fac_config_status() != 0) {
        control->get_ui_handle()->confirm_test_result_warning("配置文件有误");
        return NULL;
    }

    if (control->get_interface_run_status() == INF_RUNEND) {
        LOG_INFO("******************** start interface test ********************");
        control->set_interface_run_status(INF_RUNNING);
        control->get_ui_handle()->ui_set_interface_test_state(INF_RUNNING);
    }    
    
    FuncBase** FuncBase = control->get_funcbase();
    InterfaceSelectStatus* interfaceSelectStatus = control->get_interface_select_status();
    InterfaceTestStatus* interfaceTestStatus = control->get_interface_test_status();
    InterfaceTestResult* interfaceTestResult = control->get_interface_test_result();

    InterfaceTestFailNum* interfaceTestFailNum = new InterfaceTestFailNum;
    interfaceTestFailNum->cpu_test_fail_num    = 0;
    interfaceTestFailNum->mem_test_fail_num    = 0;
    interfaceTestFailNum->usb_test_fail_num    = 0;
    interfaceTestFailNum->net_test_fail_num    = 0;
    interfaceTestFailNum->edid_test_fail_num   = 0;
    interfaceTestFailNum->hdd_test_fail_num    = 0;
    interfaceTestFailNum->fan_test_fail_num    = 0;
    interfaceTestFailNum->wifi_test_fail_num   = 0;
    interfaceTestFailNum->ssd_test_fail_num    = 0;
    
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
        control->update_screen_log(loop);
        if (interfaceSelectStatus->mem_select) {
            LOG_INFO("---------- start mem test ----------\n");
            FuncBase[MEM]->start_test(baseInfo);
        }
        
        if (interfaceSelectStatus->usb_select) {
            LOG_INFO("---------- start usb test ----------\n");
            FuncBase[USB]->start_test(baseInfo);
        }

        if (interfaceSelectStatus->net_select) {
            LOG_INFO("---------- start net test ----------\n");
            FuncBase[NET]->start_test(baseInfo);
        }
        
        if (interfaceSelectStatus->edid_select) {
            LOG_INFO("---------- start edid test ----------\n");
            FuncBase[EDID]->start_test(baseInfo);
        }
        
        if (interfaceSelectStatus->cpu_select) {
            LOG_INFO("---------- start cpu test ----------\n");
            FuncBase[CPU]->start_test(baseInfo);
        }

        if (baseInfo->hdd_cap != "0" && baseInfo->hdd_cap != "") {
            if (interfaceSelectStatus->hdd_select) {
                LOG_INFO("---------- start hdd test ----------\n");
                FuncBase[HDD]->start_test(baseInfo);
            }
        }
        
        if (baseInfo->fan_speed != "0" && baseInfo->fan_speed != "") {
            if (interfaceSelectStatus->fan_select) {
                LOG_INFO("---------- start fan test ----------\n");
                FuncBase[FAN]->start_test(baseInfo);
            }
        }
        
        if (baseInfo->wifi_exist != "0" && baseInfo->wifi_exist != "") {
            if (interfaceSelectStatus->wifi_select) {
                FuncBase[WIFI]->start_test(baseInfo);
            }
        }
        
        if (baseInfo->ssd_cap != "0" && baseInfo->ssd_cap != "") {
            if (interfaceSelectStatus->ssd_select) {
                LOG_INFO("---------- start ssd test ----------\n");
                FuncBase[SSD]->start_test(baseInfo);
            }
        }

        while (1) {
            sleep(1);
            if (interfaceTestStatus->mem_test_over
                && interfaceTestStatus->usb_test_over
                && interfaceTestStatus->edid_test_over
                && interfaceTestStatus->cpu_test_over
                && interfaceTestStatus->net_test_over
                && interfaceTestStatus->hdd_test_over
                && interfaceTestStatus->fan_test_over
                && interfaceTestStatus->wifi_test_over
                && interfaceTestStatus->ssd_test_over){

                if (!interfaceTestResult->mem_test_result) {
                   interfaceTestFailNum->mem_test_fail_num++;
                }

                if (!interfaceTestResult->usb_test_result) {
                   interfaceTestFailNum->usb_test_fail_num++;
                }

                if (!interfaceTestResult->net_test_result) {
                   interfaceTestFailNum->net_test_fail_num++;
                }

                if (!interfaceTestResult->edid_test_result) {
                   interfaceTestFailNum->edid_test_fail_num++;
                }

                if (!interfaceTestResult->cpu_test_result) {
                   interfaceTestFailNum->cpu_test_fail_num++;
                }

                if (!interfaceTestResult->hdd_test_result) {
                   interfaceTestFailNum->hdd_test_fail_num++;
                }

                if (!interfaceTestResult->fan_test_result) {
                   interfaceTestFailNum->fan_test_fail_num++;
                }

                if (!interfaceTestResult->wifi_test_result) {
                   interfaceTestFailNum->wifi_test_fail_num++;
                }
                
                if (!interfaceTestResult->ssd_test_result) {
                   interfaceTestFailNum->ssd_test_fail_num++;
                }
                break;
            }
        }
    }

    control->update_screen_log("=============== " + INTERFACE_TEST_NAME + "结果 ===============");
    
    if (interfaceSelectStatus->mem_select) {
        string mem_total_result = "MEM\t";
        if (interfaceTestFailNum->mem_test_fail_num == 0) {
            mem_total_result = mem_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
            control->update_mes_log("MEM", "PASS");
            control->set_func_test_result(MEM_TEST_NAME, "PASS");
            control->set_interface_test_finish(MEM_TEST_NAME);
        } else {
            control->update_mes_log("MEM", "FAIL");
            control->set_func_test_result(MEM_TEST_NAME, "FAIL");
            mem_total_result = mem_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->mem_test_fail_num) + ")";
        }
        control->update_screen_log(mem_total_result);
    }
        
    if (interfaceSelectStatus->usb_select) {
        string usb_total_result = "USB\t";
        if (interfaceTestFailNum->usb_test_fail_num == 0) {
            usb_total_result = usb_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
            control->update_mes_log("USB", "PASS");
            control->set_func_test_result(USB_TEST_NAME, "PASS");
            control->set_interface_test_finish(USB_TEST_NAME);
        } else {
            control->update_mes_log("USB", "FAIL");
            control->set_func_test_result(USB_TEST_NAME, "FAIL");
            usb_total_result = usb_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->usb_test_fail_num) + ")";
        }
        control->update_screen_log(usb_total_result);
    }
    
    if (interfaceSelectStatus->net_select) {
        string net_total_result = "NET\t";
        if (interfaceTestFailNum->net_test_fail_num == 0) {
            control->update_mes_log("NET", "PASS");
            control->set_func_test_result(NET_TEST_NAME, "PASS");
            control->set_interface_test_finish(NET_TEST_NAME);
            net_total_result = net_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
        } else {
            control->update_mes_log("NET", "FAIL");
            control->set_func_test_result(NET_TEST_NAME, "FAIL");
            net_total_result = net_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->net_test_fail_num) + ")";
        }
        control->update_screen_log(net_total_result);
    }
    
    if (interfaceSelectStatus->edid_select) {
        string edid_total_result = "EDID\t";
        if (interfaceTestFailNum->edid_test_fail_num == 0) {
            control->update_mes_log("EDID", "PASS");
            control->set_func_test_result(EDID_TEST_NAME, "PASS");
            control->set_interface_test_finish(EDID_TEST_NAME);
            edid_total_result = edid_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
        } else {
            control->update_mes_log("EDID", "FAIL");
            control->set_func_test_result(EDID_TEST_NAME, "FAIL");
            edid_total_result = edid_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->edid_test_fail_num) + ")";
        }
        control->update_screen_log(edid_total_result);
    }
    
    if (interfaceSelectStatus->cpu_select) {
        string cpu_total_result = "CPU\t";
        if (interfaceTestFailNum->cpu_test_fail_num == 0) {
            control->update_mes_log("CPU", "PASS");
            control->set_func_test_result(CPU_TEST_NAME, "PASS");
            control->set_interface_test_finish(CPU_TEST_NAME);
            cpu_total_result = cpu_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
        } else {
            control->update_mes_log("CPU", "FAIL");
            control->set_func_test_result(CPU_TEST_NAME, "FAIL");
            cpu_total_result = cpu_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->cpu_test_fail_num) + ")";
        }
        control->update_screen_log(cpu_total_result);
    }
    
    if (baseInfo->hdd_cap != "0" && baseInfo->hdd_cap != "") {
        if (interfaceSelectStatus->hdd_select) {
            string hdd_total_result = "HDD\t";
            if (interfaceTestFailNum->hdd_test_fail_num == 0) {
                control->update_mes_log("HDD", "PASS");
                control->set_func_test_result(HDD_TEST_NAME, "PASS");
                control->set_interface_test_finish(HDD_TEST_NAME);
                hdd_total_result = hdd_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
            } else {
                control->update_mes_log("HDD", "FAIL");
                control->set_func_test_result(HDD_TEST_NAME, "FAIL");
                hdd_total_result = hdd_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->hdd_test_fail_num) + ")";
            }
            control->update_screen_log(hdd_total_result);
        }
    }
    
    if (baseInfo->fan_speed != "0" && baseInfo->fan_speed != "") {
        if (interfaceSelectStatus->fan_select) {
            string fan_total_result = "FAN\t";
            if (interfaceTestFailNum->fan_test_fail_num == 0) {
                control->update_mes_log("FAN", "PASS");
                control->set_func_test_result(FAN_TEST_NAME, "PASS");
                control->set_interface_test_finish(FAN_TEST_NAME);
                fan_total_result = fan_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
            } else {
                control->update_mes_log("FAN", "FAIL");
                control->set_func_test_result(FAN_TEST_NAME, "FAIL");
                fan_total_result = fan_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->fan_test_fail_num) + ")";
            }
            control->update_screen_log(fan_total_result);
        }
    }
    
    if (baseInfo->wifi_exist != "0" && baseInfo->wifi_exist != "") {
        if (interfaceSelectStatus->wifi_select) {
            string wifi_total_result = "WIFI\t";
            if (interfaceTestFailNum->wifi_test_fail_num == 0) {
                control->update_mes_log("WIFI", "PASS");
                control->set_func_test_result(WIFI_TEST_NAME, "PASS");
                control->set_interface_test_finish(WIFI_TEST_NAME);
                wifi_total_result = wifi_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
            } else {
                control->update_mes_log("WIFI", "FAIL");
                control->set_func_test_result(WIFI_TEST_NAME,"FAIL");
                wifi_total_result = wifi_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->wifi_test_fail_num) + ")";
            }
            control->update_screen_log(wifi_total_result);
        }
    }
    
    if (baseInfo->ssd_cap != "0" && baseInfo->ssd_cap != "") {
        if (interfaceSelectStatus->ssd_select) {
            string ssd_total_result = "SSD\t";
            if (interfaceTestFailNum->ssd_test_fail_num == 0) {
                control->update_mes_log("SSD", "PASS");
                control->set_func_test_result(SSD_TEST_NAME, "PASS");
                control->set_interface_test_finish(SSD_TEST_NAME);
                ssd_total_result = ssd_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
            } else {
                control->update_mes_log("SSD", "FAIL");
                control->set_func_test_result(SSD_TEST_NAME, "FAIL");
                ssd_total_result = ssd_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->ssd_test_fail_num) + ")";
            }
            control->update_screen_log(ssd_total_result);
        }
    }
    control->update_screen_log("===============================================");
    if (funcFinishStatus->mem_finish
      && funcFinishStatus->usb_finish
      && funcFinishStatus->net_finish
      && funcFinishStatus->cpu_finish
      && funcFinishStatus->edid_finish
      && funcFinishStatus->hdd_finish
      && funcFinishStatus->fan_finish
      && funcFinishStatus->wifi_finish
      && funcFinishStatus->ssd_finish) {
         funcFinishStatus->interface_finish = true;
    }
    
    control->set_interface_run_status(INF_RUNEND);
    control->get_ui_handle()->ui_set_interface_test_state(INF_RUNEND);
    return NULL;
}

void InterfaceTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}



