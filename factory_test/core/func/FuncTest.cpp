#include "../../inc/FuncTest.h"
#include "../../inc/fac_log.h"

string cpu_screen_log = "";
string cpu_screen_red = "";
string fan_screen_log = "";
string fan_screen_red = "";

static string stress_result = "";

pthread_mutex_t g_next_process_lock;

CpuTest::CpuTest()
{
}

bool CpuTest::is_cpu_test_pass(BaseInfo* baseInfo)
{
    Control *control = Control::get_control();
    string hw_cpu_type = control->get_hw_info()->cpu_type;
    string base_cpu_type = baseInfo->cpu_type;
    string::size_type idx;
    idx = hw_cpu_type.find(base_cpu_type);
    if (idx != string::npos && base_cpu_type != "") {
        cpu_screen_log += "current cpu type is " + hw_cpu_type + "\n\n";
        LOG_INFO("current cpu type is %s\n", hw_cpu_type.c_str());
        return true;
    } else {
        cpu_screen_red += "\t错误：CPU型号应为 " + base_cpu_type + ", 但检测到CPU型号为 " + hw_cpu_type + "\n";
        cpu_screen_log += "cpu type should be\t\t" + base_cpu_type + "\nbut current is\t\t" + hw_cpu_type + "\n";
        LOG_ERROR("cpu type should be %s\tbut current is %s\n",base_cpu_type.c_str(),hw_cpu_type.c_str());
        return false;
    }
}

void CpuTest::start_test(BaseInfo* baseInfo)
{
    Control *control = Control::get_control();
    control->set_interface_test_status(CPU_TEST_NAME, false);
    cpu_screen_log += "==================== " + CPU_TEST_NAME + " ====================\n";
    if (is_cpu_test_pass(baseInfo)) {
        LOG_INFO("cpu test result:\tPASS\n");
        cpu_screen_log += CPU_TEST_NAME + "结果:\t\t\t成功\n\n";
        control->set_interface_test_result(CPU_TEST_NAME, true);
    } else {
        LOG_INFO("cpu test result:\tFAIL\n");
        cpu_screen_red = CPU_TEST_NAME + "结果:\t\t\t失败\n\n" + cpu_screen_red;
        control->set_interface_test_result(CPU_TEST_NAME, false); 
    }
    control->update_screen_log(cpu_screen_log);
    cpu_screen_log = "";
    if (cpu_screen_red != "") {
        control->update_color_screen_log(cpu_screen_red, "red");
        cpu_screen_red = "";
    }
    control->set_interface_test_status(CPU_TEST_NAME, true);
}


FanTest::FanTest()
{
    
}

string FanTest::fan_speed_test(string speed)
{
    string fan_result = execute_command("bash " + FACTORY_PATH + "fan_test.sh " + speed);
    return fan_result;
}

void* FanTest::test_all(void *arg)
{
    Control *control = Control::get_control();
    control->set_interface_test_status(FAN_TEST_NAME, false);
    fan_screen_log += "==================== " + FAN_TEST_NAME + " ====================\n";
    BaseInfo* baseInfo = (BaseInfo *)arg;
    string result = fan_speed_test(baseInfo->fan_speed);
    if (result == "SUCCESS") {
        LOG_INFO("fan test result:\tPASS\n");
        fan_screen_log += FAN_TEST_NAME + "结果:\t\t\t成功\n\n";
        control->set_interface_test_result(FAN_TEST_NAME, true);
    } else {
        fan_screen_log += "fan speed should be " + baseInfo->fan_speed + "\tbut current is " + result + "\n\n";
        LOG_ERROR("fan speed should be %s\tbut current is %s\n",(baseInfo->fan_speed).c_str(),result.c_str());
        LOG_INFO("fan test result:\tFAIL\n");
        fan_screen_red += "\t错误：风扇转速应达到" + baseInfo->fan_speed + "，但测试只达到" + result + "\n";
        fan_screen_red = FAN_TEST_NAME + "结果:\t\t\t失败\n\n" + fan_screen_red;
        control->set_interface_test_result(FAN_TEST_NAME, false);
    }
    control->update_screen_log(fan_screen_log);
    fan_screen_log = "";
    control->set_interface_test_status(FAN_TEST_NAME, true);
    return NULL;
}

void FanTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}

StressTest::StressTest()
{
}

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

    if (processornum > 32) processornum = 32;
    while(processornum-- > 0) {
        char cmd_burn[CMD_BUF_SIZE];
        memset(cmd_burn, 0, CMD_BUF_SIZE);
        sprintf (cmd_burn,"taskset 0x%d burnP6 &",(1<<(processornum)));
        LOG_INFO("cmd:%s\n",cmd_burn);
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

void* StressTest::mem_stress_test(void* arg)
{
    pthread_detach(pthread_self());
    LOG_INFO("start mem stress test\n");
    int *mem_status = (int*)arg;
    stop_mem_stress_test();
    *mem_status = 2;
    string free_mem_cap = execute_command("free -m | awk '/Mem/ {print $4}'");
	if (free_mem_cap == "error") {
		LOG_ERROR("get free mem cap error\n");
	}
	int test_mem_cap = (int)(get_int_value(free_mem_cap.c_str())* 0.7);

	test_mem_cap = 10;//TODO:MEM TEST CAP��delete after��
	
	LOG_INFO("stress test mem cap is %dM", test_mem_cap);
    string result = execute_command("bash " + MEM_TEST_SCRIPT + " " + to_string(test_mem_cap) + "M");
    LOG_INFO("MEM STRESS TEST RESULT:%s",result.c_str());
    if (result == "SUCCESS") {
        LOG_INFO("mem stress test result:\tPASS\n");
        *mem_status = SUCCESS;
    } else {
        LOG_ERROR("mem stress test result:\tfailed\n");
        *mem_status = FAIL;
    }
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
    stress_result = "";
    BaseInfo* baseInfo = (BaseInfo*)arg;
    Control *control = Control::get_control();
    UiHandle* uihandle = UiHandle::get_uihandle();
    TimeInfo init_time = {0,0,0,0};
    TimeInfo tmp_dst = {0,0,0,0};
    char datebuf[CMD_BUF_SIZE] = {0};
    CpuStatus st_cpu = {0,0,0,0,0,0,0,0,0,0,0};
    pthread_t pid_t1, pid_t2;
    int encode = 0, decode = 0, mem_status = 3;
    string mem_result = "NULL";
    
    FuncBase** _funcBase = control->get_funcbase();
    CameraTest* camera = (CameraTest*)_funcBase[CAMERA];

    control->set_pcba_whole_lock_state(false);
    if (check_file_exit(STRESS_LOCK_FILE.c_str())) {
        string stress_stage = control->get_stress_test_stage();
        remove_local_file(STRESS_LOCK_FILE.c_str());
        if (stress_stage == WHOLE_LOCK || stress_stage == PCBA_LOCK) {
            control->set_pcba_whole_lock_state(true);
            LOG_INFO("last stress test exit error\n");            
        } else if (stress_stage == NEXT_LOCK) {
            LOG_INFO("next process -> stress test\n");
        } else {
            uihandle->confirm_test_result_warning("lock文件异常");
            LOG_ERROR("stress test lock file wrong\n");
            return NULL;
        }
    }
    
    if (control->get_whole_test_state()) {
        write_local_data(STRESS_LOCK_FILE.c_str(),"w+",(char*)WHOLE_LOCK,sizeof(WHOLE_LOCK));
    } else {
        write_local_data(STRESS_LOCK_FILE.c_str(),"w+",(char*)PCBA_LOCK,sizeof(PCBA_LOCK));
    }

    if(execute_command("sync") == "error") {
        uihandle->confirm_test_result_warning("cmd sync error");
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

    uihandle->update_stress_label_value("编码状态","PASS");
    uihandle->update_stress_label_value("产品型号",(control->get_hw_info())->product_name);
    uihandle->update_stress_label_value("硬件版本",(control->get_hw_info())->product_hw_version);
    uihandle->update_stress_label_value("SN序列号",(control->get_hw_info())->sn);
    uihandle->update_stress_label_value("MAC地址",(control->get_hw_info())->mac);
    uihandle->update_stress_label_value("解码状态","PASS");
    uihandle->update_stress_label_value("Mem压力测试",mem_result);

    if (baseInfo->platform == "IDV") {
        pthread_create(&pid_t2, NULL, gpu_stress_test, NULL);
    }
    start_cpuburn_stress();
    
    get_current_open_time(&init_time);
    while(true)
    {
        if (!control->is_stress_test_window_quit_safely()) {
            stress_result = "运行时间:" + to_string(tmp_dst.day) + "天" + to_string(tmp_dst.hour) +
                            "小时" + to_string(tmp_dst.minute) + "分" + to_string(tmp_dst.second) +
                            "秒  编码状态:" + (string)PRINT_RESULT1(encode) + "  解码状态:" +
                            (string)PRINT_RESULT1(decode) + "  Mem压力测试:" + mem_result + "\n";            

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
        
        encode = 0;
        decode = control->get_decode_status();

        get_current_open_time(&tmp_dst);
        diff_running_time(&tmp_dst, &init_time);
        if (tmp_dst.day == 0 && tmp_dst.hour == 0 && tmp_dst.minute == 2 && tmp_dst.second >= 0 && tmp_dst.second <= 1) {
            remove_local_file(STRESS_LOCK_FILE.c_str());
            if (encode || decode || mem_status) {
                uihandle->set_stress_test_pass_or_fail("FAIL");
            } else {
                uihandle->set_stress_test_pass_or_fail("PASS");
            }
        }
        
        if (control->get_pcba_whole_lock_state() && tmp_dst.day == 0 && tmp_dst.hour == 0 && 
                                tmp_dst.minute == 0 && tmp_dst.second >= 3 && tmp_dst.second <= 4) {
            uihandle->confirm_test_result_warning("上次拷机退出异常");
        }

        if (mem_status == 3 && tmp_dst.day == 0 && tmp_dst.hour == 0 && tmp_dst.minute == 1 &&
                                tmp_dst.second >= 0 && tmp_dst.second <= 1) {
            pthread_create(&pid_t1, NULL, mem_stress_test, &mem_status);
        }

        if (decode) {
            uihandle->update_stress_label_value("解码状态","FAIL");
            uihandle->set_stress_test_pass_or_fail("FAIL");
        }

        if (mem_status == 0) {
            mem_result = "PASS";
            uihandle->update_stress_label_value("Mem压力测试", mem_result);
        } else if (mem_status == 1){
            mem_result = "FAIL";
            uihandle->update_stress_label_value("Mem压力测试", mem_result);
            uihandle->set_stress_test_pass_or_fail("FAIL");
        }
        
        snprintf(datebuf, CMD_BUF_SIZE, "%d天%d时%d分%d秒", tmp_dst.day, tmp_dst.hour, tmp_dst.minute, tmp_dst.second);
        uihandle->update_stress_label_value("运行时间", datebuf);
        
        uihandle->update_stress_label_value("CPU温度",execute_command_err_log("bash " + GET_CPU_TEMP_SCRIPT));
        
        uihandle->update_stress_label_value("CPU频率",get_current_cpu_freq());
        uihandle->update_stress_label_value("Mem",get_mem_info());
        uihandle->update_stress_label_value("Cpu",get_cpu_info(&st_cpu));

        sleep(1);
    }    
    return NULL;
}

void StressTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}

string StressTest::get_stress_result_record()
{
    return stress_result;
}

NextProcess::NextProcess()
{
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
    int next_process_f = -1;
    UiHandle* uihandle = UiHandle::get_uihandle();
    Control* control = Control::get_control();
    
    pthread_detach(pthread_self());    
    if (pthread_mutex_trylock(&g_next_process_lock)) {
        LOG_ERROR("g_next_process_lock has been locked\n");
        return;
    }

    uihandle->confirm_test_result_waiting("正在处理，请等待...");

    if (baseInfo->emmc_cap != "0" && baseInfo->emmc_cap != "") {
        next_process_f = system("bash /etc/diskstatus_mgr.bash --product-detach");
        usleep(1000000);
        LOG_INFO("bache check result value is %d\n",WEXITSTATUS(next_process_f));
    }
    pthread_mutex_unlock(&g_next_process_lock);
    
    if (WEXITSTATUS(next_process_f) == 0) {
        if (!create_stress_test_lock()) {
            LOG_ERROR("create stress test lock fail!\n");
            uihandle->confirm_test_result_warning("EMMC异常，无法关机！");
            control->update_screen_log("EMMC异常，无法关机！");
        } else if (execute_command("shutdown -h now") == "error") {
            LOG_ERROR("shutdown cmd run error\n");
            uihandle->confirm_test_result_warning("终端异常，无法关机！");
            control->update_screen_log("终端异常，无法关机！");
        }
    } else if (WEXITSTATUS(next_process_f) > 0) {
        LOG_ERROR("The disk is abnormal and cannot enter the next process.\n");
        uihandle->confirm_test_result_warning("磁盘异常，无法进入下道工序");
        control->update_screen_log("磁盘异常，无法进入下道工序");
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
    pthread_create(&tid,NULL,test_all,baseInfo);
}

void NextProcess::init(){
    pthread_mutex_init(&g_next_process_lock, NULL);
}

UploadMesLog::UploadMesLog()
{
}

void UploadMesLog::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}

void* UploadMesLog::test_all(void *)
{
    Control *control = Control::get_control();
    control->upload_mes_log();
    return NULL;
}

InterfaceTest::InterfaceTest()
{
}


void* InterfaceTest::test_all(void *arg)
{
    BaseInfo* baseInfo = (BaseInfo *)arg;
    Control *control = Control::get_control();

    if (control->get_interface_run_status() == INF_RUNNING)
    {
        control->set_interface_run_status(INF_BREAK);
        control->get_ui_handle()->ui_set_interface_test_state(INF_BREAK);
        return NULL;
    }

    FuncFinishStatus* funcFinishStatus = control->get_func_finish_status();
    if (funcFinishStatus->interface_finish) {
        LOG_INFO("interface test has finished,do not need test again");
        return NULL;
    }

    if (control->get_auto_upload_mes_status() && control->get_fac_config_status() != 0) {
        control->get_ui_handle()->confirm_test_result_warning("配置文件有误");
        return NULL;
    }

    if (control->get_interface_run_status() == INF_RUNEND)
    {
        control->set_interface_run_status(INF_RUNNING);
        control->get_ui_handle()->ui_set_interface_test_state(INF_RUNNING);
    }    
    
    FuncBase** FuncBase = control->get_funcbase();
    InterfaceSelectStatus* interfaceSelectStatus = control->get_interface_select_status();
    InterfaceTestStatus* interfaceTestStatus = control->get_interface_test_status();
    InterfaceTestResult* interfaceTestResult = control->get_interface_test_result();

    InterfaceTestFailNum* interfaceTestFailNum = new InterfaceTestFailNum;
    interfaceTestFailNum->cpu_test_fail_num = 0;
    interfaceTestFailNum->mem_test_fail_num = 0;
    interfaceTestFailNum->usb_test_fail_num = 0;
    interfaceTestFailNum->net_test_fail_num = 0;
    interfaceTestFailNum->edid_test_fail_num = 0;
    interfaceTestFailNum->hdd_test_fail_num = 0;
    interfaceTestFailNum->fan_test_fail_num = 0;
    interfaceTestFailNum->wifi_test_fail_num = 0;
    
    int test_num = control->get_interface_test_times();
    int real_test_num = 0;
    int interface_run_status = INF_RUNNING;
    for (int i = 0; i < test_num || test_num == 0; i++) {
        interface_run_status = control->get_interface_run_status();
        if (interface_run_status == INF_BREAK) {
            break;
        }

        real_test_num = i + 1;
        string loop = "\n******************** LOOP: " + to_string(i+1) + " ********************";
        control->update_screen_log(loop);
        if (interfaceSelectStatus->mem_select) {
            FuncBase[MEM]->start_test(baseInfo);
        }
        
        if (interfaceSelectStatus->usb_select) {
            FuncBase[USB]->start_test(baseInfo);
        }

        if (interfaceSelectStatus->net_select) {
            FuncBase[NET]->start_test(baseInfo);
        }
        
        if (interfaceSelectStatus->edid_select) {
            FuncBase[EDID]->start_test(baseInfo);
        }
        
        if (interfaceSelectStatus->cpu_select) {
            FuncBase[CPU]->start_test(baseInfo);
        }

        if (baseInfo->hdd_cap != "0" && baseInfo->hdd_cap != "") {
            if (interfaceSelectStatus->hdd_select) {
                FuncBase[HDD]->start_test(baseInfo);
            }
        }
        if (baseInfo->fan_speed != "0" && baseInfo->fan_speed!= "") {
            if (interfaceSelectStatus->fan_select) {
                FuncBase[FAN]->start_test(baseInfo);
            }
        }
        
        if (baseInfo->wifi_exist!= "0" && baseInfo->wifi_exist!= "") {
            if (interfaceSelectStatus->wifi_select) {
                FuncBase[WIFI]->start_test(baseInfo);
            }
        }

        while(1) {
            sleep(1);
            if (interfaceTestStatus->mem_test_over
                && interfaceTestStatus->usb_test_over
                && interfaceTestStatus->edid_test_over
                && interfaceTestStatus->cpu_test_over
                && interfaceTestStatus->net_test_over
                && interfaceTestStatus->hdd_test_over
                && interfaceTestStatus->fan_test_over
                && interfaceTestStatus->wifi_test_over){

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
                break;
            }
        }
    }

    control->update_screen_log("=============== " + INTERFACE_TEST_NAME + "结果 ===============");
    
    if (interfaceSelectStatus->mem_select) {
        string mem_total_result = "MEM\t";
        if (interfaceTestFailNum->mem_test_fail_num == 0) {
            mem_total_result = mem_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
            control->update_mes_log("MEM","PASS");
            control->set_func_test_result(MEM_TEST_NAME,"PASS");
            control->set_interface_test_finish(MEM_TEST_NAME);
        } else {
            control->update_mes_log("MEM","FAIL");
            control->set_func_test_result(MEM_TEST_NAME,"FAIL");
            mem_total_result = mem_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->mem_test_fail_num) + ")";
        }
        control->update_screen_log(mem_total_result);
    }
        
    if (interfaceSelectStatus->usb_select) {
        string usb_total_result = "USB\t";
        if (interfaceTestFailNum->usb_test_fail_num == 0) {
            usb_total_result = usb_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
            control->update_mes_log("USB","PASS");
            control->set_func_test_result(USB_TEST_NAME,"PASS");
            control->set_interface_test_finish(USB_TEST_NAME);
        } else {
            control->update_mes_log("USB","FAIL");
            control->set_func_test_result(USB_TEST_NAME,"FAIL");
            usb_total_result = usb_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->usb_test_fail_num) + ")";
        }
        control->update_screen_log(usb_total_result);
    }
    
    if (interfaceSelectStatus->net_select) {
        string net_total_result = "NET\t";
        if (interfaceTestFailNum->net_test_fail_num == 0) {
            control->update_mes_log("NET","PASS");
            control->set_func_test_result(NET_TEST_NAME,"PASS");
            control->set_interface_test_finish(NET_TEST_NAME);
            net_total_result = net_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
        } else {
            control->update_mes_log("NET","FAIL");
            control->set_func_test_result(NET_TEST_NAME,"FAIL");
            net_total_result = net_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->net_test_fail_num) + ")";
        }
        control->update_screen_log(net_total_result);
    }
    
    if (interfaceSelectStatus->edid_select) {
        string edid_total_result = "EDID\t";
        if (interfaceTestFailNum->edid_test_fail_num == 0) {
            control->update_mes_log("EDID","PASS");
            control->set_func_test_result(EDID_TEST_NAME,"PASS");
            control->set_interface_test_finish(EDID_TEST_NAME);
            edid_total_result = edid_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
        } else {
            control->update_mes_log("EDID","FAIL");
            control->set_func_test_result(EDID_TEST_NAME,"FAIL");
            edid_total_result = edid_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->edid_test_fail_num) + ")";
        }
        control->update_screen_log(edid_total_result);
    }
    
    if (interfaceSelectStatus->cpu_select) {
        string cpu_total_result = "CPU\t";
        if (interfaceTestFailNum->cpu_test_fail_num == 0) {
            control->update_mes_log("CPU","PASS");
            control->set_func_test_result(CPU_TEST_NAME,"PASS");
            control->set_interface_test_finish(CPU_TEST_NAME);
            cpu_total_result = cpu_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
        } else {
            control->update_mes_log("CPU","FAIL");
            control->set_func_test_result(CPU_TEST_NAME,"FAIL");
            cpu_total_result = cpu_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->cpu_test_fail_num) + ")";
        }
        control->update_screen_log(cpu_total_result);
    }
    
    if (baseInfo->hdd_cap != "0" && baseInfo->hdd_cap != "") {
        if (interfaceSelectStatus->hdd_select) {
            string hdd_total_result = "HDD\t";
            if (interfaceTestFailNum->hdd_test_fail_num == 0) {
                control->update_mes_log("HDD","PASS");
                control->set_func_test_result(HDD_TEST_NAME,"PASS");
                control->set_interface_test_finish(HDD_TEST_NAME);
                hdd_total_result = hdd_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
            } else {
                control->update_mes_log("HDD","FAIL");
                control->set_func_test_result(HDD_TEST_NAME,"FAIL");
                hdd_total_result = hdd_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->hdd_test_fail_num) + ")";
            }
            control->update_screen_log(hdd_total_result);
        }
    }
    
    if (baseInfo->fan_speed != "0" && baseInfo->fan_speed!= "") {
        if (interfaceSelectStatus->fan_select) {
            string fan_total_result = "FAN\t";
            if (interfaceTestFailNum->fan_test_fail_num == 0) {
                control->update_mes_log("FAN","PASS");
                control->set_func_test_result(FAN_TEST_NAME,"PASS");
                control->set_interface_test_finish(FAN_TEST_NAME);
                fan_total_result = fan_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
            } else {
                control->update_mes_log("FAN","FAIL");
                control->set_func_test_result(FAN_TEST_NAME,"FAIL");
                fan_total_result = fan_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->fan_test_fail_num) + ")";
            }
            control->update_screen_log(fan_total_result);
        }
    }
    
    if (baseInfo->wifi_exist!= "0" && baseInfo->wifi_exist!= "") {
        if (interfaceSelectStatus->wifi_select) {
            string wifi_total_result = "WIFI\t";
            if (interfaceTestFailNum->wifi_test_fail_num == 0) {
                control->update_mes_log("WIFI","PASS");
                control->set_func_test_result(WIFI_TEST_NAME,"PASS");
                control->set_interface_test_finish(WIFI_TEST_NAME);
                wifi_total_result = wifi_total_result + "PASS (Time:" + to_string(real_test_num) + ",ERROR:0)";
            } else {
                control->update_mes_log("WIFI","FAIL");
                control->set_func_test_result(WIFI_TEST_NAME,"FAIL");
                wifi_total_result = wifi_total_result + "FAIL (Time:" + to_string(real_test_num) + ",ERROR:" 
                               + to_string(interfaceTestFailNum->wifi_test_fail_num) + ")";
            }
            control->update_screen_log(wifi_total_result);
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
      && funcFinishStatus->wifi_finish) {
         funcFinishStatus->interface_finish = true;
    }
    
    control->set_interface_run_status(INF_RUNEND);
    control->get_ui_handle()->ui_set_interface_test_state(INF_RUNEND);
    return NULL;
}

void InterfaceTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}



