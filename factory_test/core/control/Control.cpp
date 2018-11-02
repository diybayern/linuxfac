#include "Control.h"
#include "fac_log.h"

#include <algorithm>

Control::Control():QObject()
{
    _funcBase[INTERFACE]      = new InterfaceTest();
    _funcBase[MEM]            = new MemTest();
    _funcBase[USB]            = new UsbTest();
    _funcBase[CPU]            = new CpuTest();
    _funcBase[EDID]           = new EdidTest();
    _funcBase[NET]            = new NetTest();
    _funcBase[HDD]            = new HddTest();
    _funcBase[FAN]            = new FanTest();
    _funcBase[WIFI]           = new WifiTest();
    _funcBase[SSD]            = new SsdTest();
    _funcBase[SOUND]          = new SoundTest();
    _funcBase[BRIGHT]         = new BrightTest();
    _funcBase[CAMERA]         = new CameraTest();
    _funcBase[STRESS]         = new StressTest();
    _funcBase[NEXT_PROCESS]   = new NextProcess();
    _funcBase[UPLOAD_MES_LOG] = new UploadMesLog();
    
    _uiHandle                 = UiHandle::get_uihandle();
    
    _baseInfo                 = new BaseInfo;
    _hwInfo                   = new HwInfo;
    _facArg                   = new FacArg;
    _mesInfo                  = new MesInfo;

    _funcFinishStatus         = new FuncFinishStatus;
    _interfaceTestStatus      = new InterfaceTestStatus;    
    _interfaceSelectStatus    = new InterfaceSelectStatus;
    _interfaceTestResult      = new InterfaceTestResult;

    _testStep = STEP_IDLE;
    _interfaceRunStatus = INF_RUNEND;
    
    init_base_info();
    init_hw_info();
    init_fac_config();
    init_mes_log();
}

Control* Control::_control = NULL;
Control* Control::get_control()
{
    if (!_control) {
        _control = new Control();
    }
    return _control;
}

void Control::init_base_info()
{
    string baseinfo = execute_command("/usr/local/bin/system/getHWCfg", true);

    if (baseinfo != "error") {
        int len = baseinfo.size();
        if(baseinfo[0] != '{' || baseinfo[len - 1] != '}')
        {
            LOG_INFO("base info is not right");
            goto _is_third;
        }

        baseinfo = baseinfo.substr(1, baseinfo.length() - 2);
        get_baseinfo(_baseInfo, baseinfo);
        LOG_INFO("product is %s", (_baseInfo->platform).c_str());
    } else {
        LOG_ERROR("get hwcfg.ini information error");
    }
    
_is_third:
    if (_baseInfo->platform == "") {
        _is_third_product = true;
        LOG_INFO("this is third product\n");
    } else {
        _is_third_product = false;
    }
}

void Control::init_hw_info()
{
    get_hwinfo(_hwInfo);
}

void Control::init_ui()
{    
    _uiHandle->add_main_label("产品型号:", _hwInfo->product_name);
    _uiHandle->add_main_label("硬件版本:", _hwInfo->product_hw_version);
    _uiHandle->add_main_label("SN序列号:", _hwInfo->sn);
    _uiHandle->add_main_label("MAC地址:", _hwInfo->mac);

    if (_is_third_product) {
        _uiHandle->add_main_label("CPU型号:", _hwInfo->cpu_type);
        _uiHandle->add_main_label("内存容量:", execute_command("free -m | awk '/Mem/ {print $2}'", true) + "M");
        _uiHandle->add_main_label("HDD容量:", "--");//TODO
        _uiHandle->add_main_label("SSD容量:", "--");//TODO
        _uiHandle->add_main_label("EDID信息:", to_string(edid_read_i2c_test(-1)));
        
        string real_total_num = execute_command("lsusb -t | grep \"Mass Storage\" | wc -l", true);
        string real_num_3 = execute_command("lsusb -t | grep \"Mass Storage\" | grep \"5000M\" | wc -l", true);
        _baseInfo->usb_total_num = real_total_num;
        _baseInfo->usb_3_num = real_num_3;
        _uiHandle->add_main_label("USB信息:", real_num_3 + "/" + real_total_num);
        _uiHandle->add_main_label("网卡信息:", get_third_net_info());

        string wifi_exist = execute_command("ifconfig -a | grep wlan0", true);
        if (wifi_exist != "error" && wifi_exist != "") {
            _baseInfo->wifi_exist = "1";
            _uiHandle->add_main_label("WIFI信息:", "存在");
        }
    
        string camera_exist = execute_command("xawtv -hwscan 2>&1 | grep OK", true);
        if ( camera_exist != "error" && camera_exist != "") {
            _baseInfo->camera_exist = "1";
            _uiHandle->add_main_label("摄像头信息:", "存在");
        }
    }
    
    _uiHandle->add_main_test_button(INTERFACE_TEST_NAME);
    
    _uiHandle->add_interface_test_button(MEM_TEST_NAME);
    
    if (!_is_third_product || _baseInfo->usb_total_num != "0") {
        _uiHandle->add_interface_test_button(USB_TEST_NAME);
    } else {
        _funcFinishStatus->usb_finish = true;
        _interfaceTestStatus->usb_test_over = true;
    }
    _uiHandle->add_interface_test_button(NET_TEST_NAME);
    
    if (_is_third_product) {
        _funcFinishStatus->edid_finish = true;
        _interfaceTestStatus->edid_test_over = true;
        _funcFinishStatus->cpu_finish = true;
        _interfaceTestStatus->cpu_test_over = true;
    } else {
        _uiHandle->add_interface_test_button(EDID_TEST_NAME);
        _uiHandle->add_interface_test_button(CPU_TEST_NAME);
    }
    
    if (_baseInfo->hdd_cap != "0" && _baseInfo->hdd_cap != "") {
        _uiHandle->add_interface_test_button(HDD_TEST_NAME);
    } else {
        _funcFinishStatus->hdd_finish = true;
        _interfaceTestStatus->hdd_test_over = true;
    }
    
    if (_baseInfo->ssd_cap != "0" && _baseInfo->ssd_cap != "") {
        _uiHandle->add_interface_test_button(SSD_TEST_NAME);
    } else {
        _funcFinishStatus->ssd_finish = true;
        _interfaceTestStatus->ssd_test_over = true;
    }
    
    if (_baseInfo->fan_speed != "0" && _baseInfo->fan_speed != "") {
        _uiHandle->add_interface_test_button(FAN_TEST_NAME);
    } else {
        _funcFinishStatus->fan_finish = true;
        _interfaceTestStatus->fan_test_over = true;
    }
    
    if (!_is_third_product && _baseInfo->wifi_exist != "0" && _baseInfo->wifi_exist != "") {
        _uiHandle->add_interface_test_button(WIFI_TEST_NAME);
    } else {
        _funcFinishStatus->wifi_finish = true;
        _interfaceTestStatus->wifi_test_over = true;
    }
    
    _uiHandle->add_main_test_button(SOUND_TEST_NAME);
    _uiHandle->add_main_test_button(DISPLAY_TEST_NAME);

    if (!_is_third_product && _baseInfo->bright_level != "0" && _baseInfo->bright_level != ""){
        _uiHandle->add_main_test_button(BRIGHT_TEST_NAME);
    } else {
        _funcFinishStatus->bright_finish = true;
    }
    
    if (_baseInfo->camera_exist != "0" && _baseInfo->camera_exist != "") {
        _uiHandle->add_main_test_button(CAMERA_TEST_NAME);
    } else {
        _funcFinishStatus->camera_finish = true;
    }

    _uiHandle->add_main_test_button(STRESS_TEST_NAME);
    if (!_is_third_product) {
        _uiHandle->add_main_test_button(UPLOAD_LOG_NAME);
        if (!check_file_exit(WHOLE_TEST_FILE)) {
            _uiHandle->add_main_test_button(NEXT_PROCESS_NAME);
        }    
    }
    
    
    if (_is_third_product) {
        _uiHandle->add_complete_or_single_test_label("第三方终端");
    } else if (check_file_exit(WHOLE_TEST_FILE)) {
        _uiHandle->add_complete_or_single_test_label("整机测试");
    } else {
        _uiHandle->add_complete_or_single_test_label("单板测试");
    }
    
    if (_is_third_product) {
        _uiHandle->sync_main_test_ui(true);
    } else {
        _uiHandle->sync_main_test_ui(false);
    }
 
    _uiHandle->add_stress_test_label("运行时间");
    _uiHandle->add_stress_test_label("CPU温度");
    _uiHandle->add_stress_test_label("编码状态");
    _uiHandle->add_stress_test_label("解码状态");
    _uiHandle->add_stress_test_label("Mem压力测试");
    _uiHandle->add_stress_test_label("Mem");
    _uiHandle->add_stress_test_label("Cpu");
    _uiHandle->add_stress_test_label("CPU频率");
    _uiHandle->add_stress_test_label("SN序列号");
    _uiHandle->add_stress_test_label("MAC地址");
    _uiHandle->add_stress_test_label("产品型号");
    _uiHandle->add_stress_test_label("硬件版本");

    connect(_uiHandle->get_qobject(INTERFACE_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_interface_test()));
        
    if (!_is_third_product && _baseInfo->bright_level != "0" && _baseInfo->bright_level != ""){
        connect(_uiHandle->get_qobject(BRIGHT_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_bright_test()));
    }
    if (_baseInfo->camera_exist != "0" && _baseInfo->camera_exist != "") {
        connect(_uiHandle->get_qobject(CAMERA_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_camera_test()));
    }
    connect(_uiHandle->get_qobject(SOUND_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_sound_test()));
    connect(_uiHandle->get_qobject(DISPLAY_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_display_test()));

    connect(_uiHandle->get_qobject(STRESS_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_stress_test()));
    if (!_is_third_product) {	
        connect(_uiHandle->get_qobject(UPLOAD_LOG_NAME), SIGNAL(clicked()), this, SLOT(start_upload_log()));
    }

    if (!_is_third_product && !check_file_exit(WHOLE_TEST_FILE)) {
        connect(_uiHandle->get_qobject(NEXT_PROCESS_NAME), SIGNAL(clicked()), this, SLOT(start_next_process()));
    }
    connect(_uiHandle, SIGNAL(to_show_test_confirm_dialog(string)), this, SLOT(show_test_confirm_dialog(string)));
    connect(_uiHandle, SIGNAL(sig_ui_handled_test_result(string, string)), this, SLOT(set_test_result_pass_or_fail(string, string)));
    connect(_uiHandle, SIGNAL(sig_ui_handled_sn_mac_test_result(string, string)), this, SLOT(set_sn_mac_test_result(string, string)));
    connect(_uiHandle, SIGNAL(sig_ui_check_state_changed(string, bool)), this, SLOT(set_interface_select_status(string, bool)));
    connect(_uiHandle, SIGNAL(sig_ui_get_message_from_scangun(string)), this, SLOT(check_sn_mac_compare_result(string)));
    connect(_uiHandle, SIGNAL(sig_ui_confirm_shut_down_or_next_process(string)), this, SLOT(confirm_shut_down_or_next_process(string)));
    connect(_uiHandle, SIGNAL(sig_ui_retry_sn_mac()), this, SLOT(retry_sn_mac_test()));
    connect(_uiHandle, SIGNAL(sig_ui_factory_delete_event()), this, SLOT(slot_factory_delete_event()));
}

string Control::get_third_net_info()
{
    NetTest* net = (NetTest*)_funcBase[NET];
    bool result = net->net_test_all(false);
    if (result) {
        NetInfo *info = net->g_net_info;
        string net_info = to_string(info->eth_speed) + "Mbps, " + net->net_get_duplex_desc(info->eth_duplex);
        return net_info;
    } else {
        return "--";
    }
}

void Control::retry_sn_mac_test()
{
    if (_display_sn_or_mac == "MAC") {
        LOG_INFO("retry test mac");
        _uiHandle->show_sn_mac_message_box("MAC");
    } else if (_display_sn_or_mac == "SN") {
        LOG_INFO("retry test sn");
        _uiHandle->show_sn_mac_message_box("SN");
    }
}

void Control::confirm_shut_down_or_next_process(string process)
{
    if (process == NEXT_PROCESS_NAME) {
        _funcBase[NEXT_PROCESS]->start_test(_baseInfo);
    } else if (process == "关机") {
        if (execute_command("shutdown -h now", true) == "error"){
            LOG_ERROR("shutdown cmd run error\n");            
            _uiHandle->confirm_test_result_warning("终端异常，无法关机！");
        }
    }
}

void Control::check_sn_mac_compare_result(string message)
{
    if (_display_sn_or_mac == "MAC") {
        string mac = _hwInfo->mac;
        mac.erase(remove(mac.begin(), mac.end(), ':'), mac.end());
        if (message.size() != mac.size() || message != mac) {
            LOG_INFO("mac test result:\tFAIL\n");
            _uiHandle->update_sn_mac_test_result("MAC", "FAIL");
            _uiHandle->show_sn_mac_comparison_result("MAC", "FAIL");
            return ;
        } else {
            LOG_INFO("mac test result:\tPASS\n");
            _uiHandle->update_sn_mac_test_result("MAC", "PASS");
            _uiHandle->show_sn_mac_comparison_result("MAC", "PASS");
            return ;
        }
    }

    if (_display_sn_or_mac == "SN") {
        string sn = _hwInfo->sn;
        if (message.size() != sn.size() || message != sn) {
            LOG_INFO("sn test result:\tFAIL\n");
            _uiHandle->update_sn_mac_test_result("SN", "FAIL");
            _uiHandle->show_sn_mac_comparison_result("SN", "FAIL");
            return ;
        } else {
            LOG_INFO("sn test result:\tPASS\n");
            _uiHandle->update_sn_mac_test_result("SN", "PASS");
            _uiHandle->show_sn_mac_comparison_result("SN", "PASS");
            return ;
        }
    }
}

void Control::show_test_confirm_dialog(string item)
{
    if (item.compare(STRESS_TEST_NAME) == 0) {
        _stress_test_window_quit_status = false;
    }
    _uiHandle->confirm_test_result_dialog(item);
}

void Control::init_func_test()
{
    SoundTest* sound = (SoundTest*)_funcBase[SOUND];
    sound->init(_baseInfo);
    
    NetTest* net = (NetTest*)_funcBase[NET];
    net->init();

    WifiTest* wifi = (WifiTest*)_funcBase[WIFI];
    wifi->init();
    
    NextProcess* nextpro = (NextProcess*)_funcBase[NEXT_PROCESS];
    nextpro->init();
}

void Control::init_fac_config()
{
    UsbTest* usb = (UsbTest*)_funcBase[USB];
    if (!usb->usb_test_read_status()) {
        LOG_ERROR("init copy fac config error");
    }
    _fac_config_status = get_fac_config_from_conf(FAC_CONFIG_FILE, _facArg);
}

void Control::start_interface_test()
{
    _testStep = STEP_INTERFACE;
    _funcBase[INTERFACE]->start_test(_baseInfo);
}

void Control::start_sound_test()
{
    LOG_INFO("******************** start sound test ********************");
    _funcBase[SOUND]->start_test(_baseInfo);
}

void Control::start_display_test()
{
    LOG_INFO("******************** start display test ********************");
    update_screen_log("==================== " + DISPLAY_TEST_NAME + " ====================\n");
    _uiHandle->show_display_ui();
}

void Control::start_bright_test()
{
    LOG_INFO("******************** start bright test ********************");
    _funcBase[BRIGHT]->start_test(_baseInfo);
}

void Control::start_camera_test()
{
    LOG_INFO("******************** start camera test ********************");
    _funcBase[CAMERA]->start_test(_baseInfo);
}

void Control::start_stress_test()
{
    LOG_INFO("******************** start stress test ********************");
    _funcBase[STRESS]->start_test(_baseInfo);
}

void Control::start_next_process()
{
    LOG_INFO("******************** start next process ********************");
    _funcBase[NEXT_PROCESS]->start_test(_baseInfo);
}

void Control::start_upload_log()
{
    LOG_INFO("******************** start upload log ********************");
    _funcBase[UPLOAD_MES_LOG]->start_test(_baseInfo);
}


void Control::set_test_result(string func, string result, string ui_log)

{
    _uiHandle->set_test_result(func, result);
    _uiHandle->update_screen_log(ui_log);
}

void Control::confirm_test_result(string func)
{
    LOG_INFO("confirm %s result", func.c_str());
    _uiHandle->confirm_test_result_dialog(func);
}

void Control::set_brightness_dialog_button_state(bool state)
{
    LOG_INFO("set_brightness_dialog_button_state");
    _uiHandle->set_brightness_dialog_button_state(state);
}

void Control::show_main_test_ui()
{
    init_ui();
    _uiHandle->to_show_main_test_ui();
    show_stress_record();
    auto_test_mac_or_stress();
}

void Control::show_stress_record(){
    update_screen_log("---------------------------------------------------------------------------------------------\n");
    update_screen_log("\t\tWelcome to Factory Test Software\n");
    update_screen_log("---------------------------------------------------------------------------------------------\n");

    read_stress_record(&_record);
    print_stress_test_result(_record);
}

void Control::print_stress_test_result(vector<string> record) 
{
    update_screen_log("The last Stress test result is...\n");
    update_screen_log("==================== " + STRESS_TEST_NAME + "结果 ====================\n");

    for (unsigned int i = 0; i < record.size(); i++) {
        update_screen_log(record[i]);
    }
    update_screen_log("==================================================\n");
}

void Control::auto_test_mac_or_stress() {    
    if (check_file_exit(STRESS_LOCK_FILE)) {
        LOG_INFO("******************** auto start stress test ********************");
        _funcBase[STRESS]->start_test(_baseInfo);
    } else if (!get_third_product_state()){
        _display_sn_or_mac = "MAC";
        _uiHandle->show_sn_mac_message_box("MAC");
    }
}

int Control::get_test_step()
{
    return _testStep;
}

void Control::init_mes_log()
{
    int i=0;
    int j=0;
    char date[64] = {0, };
    char new_mac_name[128];
    char tmp[128];
    struct tm *timenow = NULL;
    time_t timep;
 
    while (_hwInfo->mac[i] != '\0') {
        if (_hwInfo->mac[i] != ':') {
            new_mac_name[j] = _hwInfo->mac[i];
            j++;
        }
        i++;
    }

    new_mac_name[j] = '\0';
    string mac_capital = "";
    string sn_capital = "";
    
    mac_capital = lower_to_capital(new_mac_name);
    sn_capital = lower_to_capital(_hwInfo->sn);
    
    if (access(MES_FILE, F_OK) == 0) {
        remove(MES_FILE);
    }

    sprintf(tmp, "%s%s.txt", _facArg->ftp_dest_path.c_str(), mac_capital.c_str());
    _facArg->ftp_dest_path = tmp;
    LOG_INFO("_facArg->ftp_dest_path:%s", (_facArg->ftp_dest_path).c_str());
    
    time(&timep);
    timenow = localtime(&timep);
    strftime(date, 64, "%Y%m%d-%H:%M:%S", timenow);



    LOG_MES("---------------------Product infomation-----------------------\n");
    LOG_MES("Model: \t%s\n", _hwInfo->product_name);
    LOG_MES("SN: \t%s\n", sn_capital);
    LOG_MES("MAC: \t%s\n", mac_capital);
    LOG_MES("DATE: \t%s\n", date);
    LOG_MES("OPERATION: \t%s\n", _facArg->ftp_job_number);
    LOG_MES("---------------------Simple test result-----------------------\n");
    LOG_MES("MEMORY:    NULL\n");
    LOG_MES("USB:       NULL\n");
    LOG_MES("NET:       NULL\n");
    LOG_MES("EDID:      NULL\n");
    LOG_MES("CPU:       NULL\n");
    if (_baseInfo->hdd_cap != "0" && _baseInfo->hdd_cap != "") {
        LOG_MES("HDD:       NULL\n");
    }
    if (_baseInfo->ssd_cap != "0" && _baseInfo->ssd_cap != "") {
        LOG_MES("SSD:       NULL\n");
    }
    if (_baseInfo->fan_speed != "0" && _baseInfo->fan_speed != "") {
        LOG_MES("FAN:       NULL\n");
    }
    if (_baseInfo->wifi_exist != "0" && _baseInfo->wifi_exist != "") {
        LOG_MES("WIFI:      NULL\n");
    }
    LOG_MES("AUDIO:     NULL\n");
    LOG_MES("DISPLAY:   NULL\n");
    if (_baseInfo->bright_level != "0" && _baseInfo->bright_level != "") {
        LOG_MES("BRIGHTNESS:NULL\n");
    }
    if (_baseInfo->camera_exist != "0" && _baseInfo->camera_exist != "") {
        LOG_MES("CAMERA:    NULL\n");
    }
    LOG_MES("---------------------Stress test result-----------------------\n");
}

void Control::update_mes_log(string tag, string value)
{
    FILE* fp;
    char line[200];
    char* sp = NULL;
    int file_size;
    int first = 1;
 
    bzero(line,sizeof(line));
 
    if ((fp = fopen(MES_FILE, "r")) == NULL) {
        LOG_ERROR("open %s failed", MES_FILE);
        return;
    }
 
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* buf = (char*)malloc(file_size + 128);
    if (NULL == buf) {
        fclose(fp);
        return;
    }
    
    bzero(buf, file_size);
 
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (first &&((sp = strstr(line, tag.c_str())) != NULL)) {
            char value_temp[128];
            bzero(value_temp, 128);
            sprintf(value_temp, "%s\n", value.c_str());
            memcpy(sp + 11, value_temp, strlen(value_temp) + 1);//修改标签内容，加TAG_OFFSET是为了对齐值
            first = 0;
        }
        strcat(buf, line);
    }
 
    fclose(fp);
    fp = fopen(MES_FILE, "w");
    if (fp == NULL) {
        return ;
    }

    if (buf != NULL) {
        int len = strlen(buf);
        fwrite(buf, 1, len, fp);
    }
    fclose(fp);

    if (buf != NULL) {
        free(buf);
    }

}


void Control::upload_mes_log() {
    update_screen_log("==================== " + UPLOAD_LOG_NAME + " ====================\n");
    if (_fac_config_status != 0) {
        LOG_INFO("fac config is wrong, do not upload");
        _uiHandle->confirm_test_result_warning("配置文件有误");
        set_test_result(UPLOAD_LOG_NAME, "FAIL", "配置文件有误");
        return;
    } else {
        if (!combine_fac_log_to_mes(MES_FILE, STRESS_RECORD)) {
            update_color_screen_log("拷机记录文件为空\n", "red");
            LOG_MES("no stress test record\n");
            LOG_INFO("NO stress record\n");
        }
        LOG_MES("---------------------Detail test result-----------------------\n");
        if (!combine_fac_log_to_mes(MES_FILE, LOG_FILE)) {
            LOG_ERROR("combine log failed");
            _uiHandle->confirm_test_result_warning("log文件拼接失败");
            return;
        }
        string upload_log = "ftp ip:\t\t" + (string)_facArg->ftp_ip + "\n";
        upload_log += "ftp user:\t\t" + (string)_facArg->ftp_user + "\n";
        upload_log += "ftp passwd:\t\t" + (string)_facArg->ftp_passwd + "\n";
        upload_log += "ftp path:\t\t" + (string)_facArg->ftp_dest_path + "\n";
        update_screen_log(upload_log);

        _uiHandle->confirm_test_result_waiting("正在上传中...");
        sleep(1);
        string response = ftp_send_file(MES_FILE, _facArg);
        response = response_to_chinese(response);
        LOG_INFO("upload log: %s", response.c_str());
        if (response.compare("上传成功") == 0) {
            if (check_file_exit(WHOLE_TEST_FILE)) {
                _uiHandle->confirm_test_result_success("上传成功", "关机");
            } else {
                _uiHandle->confirm_test_result_success("上传成功", "下道工序");
            }
            set_test_result(UPLOAD_LOG_NAME, "PASS", response);
        } else {
            _uiHandle->confirm_test_result_warning("上传失败");
            set_test_result(UPLOAD_LOG_NAME, "FAIL", response);
        }
    }
    _testStep = STEP_IDLE;
}

void Control::update_screen_log(string uiLog)
{
    _uiHandle->update_screen_log(uiLog);
}

void Control::update_color_screen_log(string uiLog, string color)
{
    _uiHandle->update_color_screen_log(uiLog, color);;
}

void Control::set_func_test_result(string func, string result)

{
    _uiHandle->set_test_result(func, result);
}


int Control::get_screen_height()
{
    return _uiHandle->get_screen_height();
}

int Control::get_screen_width()
{
    return _uiHandle->get_screen_width();
}

void Control::set_interface_select_status(string func, bool state) {
    _funcFinishStatus->interface_finish = false;
    if (func == MEM_TEST_NAME) {
        _interfaceSelectStatus->mem_select = state;
        if (_interfaceSelectStatus->mem_select) {
            _funcFinishStatus->mem_finish = false;
        } else {
            _funcFinishStatus->mem_finish = true;
            _interfaceTestStatus->mem_test_over = true;
        }
    }
    if (func == USB_TEST_NAME) {
        _interfaceSelectStatus->usb_select = state;
        if (_interfaceSelectStatus->usb_select) {
            _funcFinishStatus->usb_finish = false;
        } else {
            _funcFinishStatus->usb_finish = true;
            _interfaceTestStatus->usb_test_over = true;
        }
    }
    if (func == NET_TEST_NAME) {
        _interfaceSelectStatus->net_select = state;
        if (_interfaceSelectStatus->net_select) {
            _funcFinishStatus->net_finish = false;
        } else {
            _funcFinishStatus->net_finish = true;
            _interfaceTestStatus->net_test_over = true;
        }
    }
    if (func == EDID_TEST_NAME) {
        _interfaceSelectStatus->edid_select = state;
        if (_interfaceSelectStatus->edid_select) {
            _funcFinishStatus->edid_finish = false;
        } else {
            _funcFinishStatus->edid_finish = true;
            _interfaceTestStatus->edid_test_over = true;
        }
    }
    if (func == CPU_TEST_NAME) {
        _interfaceSelectStatus->cpu_select = state;
        if (_interfaceSelectStatus->cpu_select) {
            _funcFinishStatus->cpu_finish = false;
        } else {
            _funcFinishStatus->cpu_finish = true;
            _interfaceTestStatus->cpu_test_over = true;
        }
    }
    if (func == HDD_TEST_NAME) {
        _interfaceSelectStatus->hdd_select = state;
        if (_interfaceSelectStatus->hdd_select) {
            _funcFinishStatus->hdd_finish = false;
        } else {
            _funcFinishStatus->hdd_finish = true;
            _interfaceTestStatus->hdd_test_over = true;
        }
    }
    if (func == FAN_TEST_NAME) {
        _interfaceSelectStatus->fan_select = state;
        if (_interfaceSelectStatus->fan_select) {
            _funcFinishStatus->fan_finish = false;
        } else {
            _funcFinishStatus->fan_finish = true;
            _interfaceTestStatus->fan_test_over = true;
        }
    }
    if (func == WIFI_TEST_NAME) {
        _interfaceSelectStatus->wifi_select = state;
        if (_interfaceSelectStatus->wifi_select) {
            _funcFinishStatus->wifi_finish = false;
        } else {
            _funcFinishStatus->wifi_finish = true;
            _interfaceTestStatus->wifi_test_over = true;
        }
    }
    if (func == SSD_TEST_NAME) {
        _interfaceSelectStatus->ssd_select = state;
        if (_interfaceSelectStatus->ssd_select) {
            _funcFinishStatus->ssd_finish = false;
        } else {
            _funcFinishStatus->ssd_finish = true;
            _interfaceTestStatus->ssd_test_over = true;
        }
    }
}

void Control::set_interface_test_status(string func, bool status){
    if (func == MEM_TEST_NAME) {
        _interfaceTestStatus->mem_test_over = status;
    }
    if (func == USB_TEST_NAME) {
        _interfaceTestStatus->usb_test_over = status;
    }
    if (func == CPU_TEST_NAME) {
        _interfaceTestStatus->cpu_test_over = status;
    }
    if (func == NET_TEST_NAME) {
        _interfaceTestStatus->net_test_over = status;
    }
    if (func == EDID_TEST_NAME) {
        _interfaceTestStatus->edid_test_over = status;
    }
    if (func == HDD_TEST_NAME) {
        _interfaceTestStatus->hdd_test_over = status;
    }
    if (func == FAN_TEST_NAME) {
        _interfaceTestStatus->fan_test_over = status;
    }
    if (func == WIFI_TEST_NAME) {
        _interfaceTestStatus->wifi_test_over = status;
    }
    if (func == SSD_TEST_NAME) {
        _interfaceTestStatus->ssd_test_over = status;
    }
}

void Control::set_interface_test_finish(string func){
    if (func == MEM_TEST_NAME) {
        _funcFinishStatus->mem_finish = true;
    }
    if (func == USB_TEST_NAME) {
        _funcFinishStatus->usb_finish = true;
    }
    if (func == CPU_TEST_NAME) {
        _funcFinishStatus->cpu_finish = true;
    }
    if (func == NET_TEST_NAME) {
        _funcFinishStatus->net_finish = true;
    }
    if (func == EDID_TEST_NAME) {
        _funcFinishStatus->edid_finish = true;
    }
    if (func == HDD_TEST_NAME) {
        _funcFinishStatus->hdd_finish = true;
    }
    if (func == FAN_TEST_NAME) {
        _funcFinishStatus->fan_finish = true;
    }
    if (func == WIFI_TEST_NAME) {
        _funcFinishStatus->wifi_finish = true;
    }
    if (func == SSD_TEST_NAME) {
        _funcFinishStatus->ssd_finish = true;
    }
}

void Control::set_interface_test_result(string func, bool status) {
    if (func == MEM_TEST_NAME) {
        _interfaceTestResult->mem_test_result = status;
    }
    if (func == USB_TEST_NAME) {
        _interfaceTestResult->usb_test_result = status;
    }
    if (func == CPU_TEST_NAME) {
        _interfaceTestResult->cpu_test_result = status;
    }
    if (func == NET_TEST_NAME) {
        _interfaceTestResult->net_test_result = status;
    }
    if (func == EDID_TEST_NAME) {
        _interfaceTestResult->edid_test_result = status;
    }
    if (func == HDD_TEST_NAME) {
        _interfaceTestResult->hdd_test_result = status;
    }
    if (func == FAN_TEST_NAME) {
        _interfaceTestResult->fan_test_result = status;
    }
    if (func == WIFI_TEST_NAME) {
        _interfaceTestResult->wifi_test_result = status;
    }
    if (func == SSD_TEST_NAME) {
        _interfaceTestResult->ssd_test_result = status;
    }
}

void* Control::update_mes_log_thread(void* arg)
{
    MesInfo* info = (MesInfo*)arg;
    Control::get_control()->update_mes_log(info->func, info->status);
    return NULL;
}

void Control::start_update_mes_log(MesInfo* info)
{
    pthread_t tid;
    pthread_create(&tid, NULL, update_mes_log_thread, info);
}


void Control::set_test_result_pass_or_fail(string func, string result)
{
    if (result == "PASS") {
        if (func == SOUND_TEST_NAME) {
            _mesInfo->func = "AUDIO";
            _mesInfo->status = "PASS";
            start_update_mes_log(_mesInfo);
            update_screen_log(SOUND_TEST_NAME + "结果：\t\t\t成功\n");
            LOG_INFO("sound test result:\tPASS\n");
            _funcFinishStatus->sound_finish = true;
        }
        if (func == DISPLAY_TEST_NAME) {
            _mesInfo->func = "DISPLAY";
            _mesInfo->status = "PASS";
            start_update_mes_log(_mesInfo);
            update_screen_log(DISPLAY_TEST_NAME + "结果：\t\t\t成功\n");
            LOG_INFO("display test result:\tPASS\n");
            _funcFinishStatus->display_finish = true;
        }
        if (func == BRIGHT_TEST_NAME) {
            _mesInfo->func = "BRIGHTNESS";
            _mesInfo->status = "PASS";
            start_update_mes_log(_mesInfo);
            update_screen_log(BRIGHT_TEST_NAME + "结果：\t\t\t成功\n");
            LOG_INFO("bright test result:\tPASS\n");
            _funcFinishStatus->bright_finish = true;
        }
        if (func == CAMERA_TEST_NAME) {
            CameraTest* camera = (CameraTest*)_funcBase[CAMERA];
            camera->close_xawtv_window();
            _mesInfo->func = "CAMERA";
            _mesInfo->status = "PASS";
            start_update_mes_log(_mesInfo);
            update_screen_log(CAMERA_TEST_NAME + "结果：\t\t\t成功\n");
            LOG_INFO("camera test result:\tPASS\n");
            _funcFinishStatus->camera_finish = true;
        }
        if (func == STRESS_TEST_NAME) {
            LOG_INFO("stress test result:\tPASS\n");
            StressTest* stress = (StressTest*)_funcBase[STRESS];
            string result = stress->get_stress_result_record();
            _record.push_back("PASS  " + result);
            if (_record.size() > STRESS_RECORD_NUM) {
                _record.erase(_record.begin());
            }
            write_stress_record(_record);
            print_stress_test_result(_record);
            _funcFinishStatus->stress_finish = true;
        }
    } else {

        if (func == SOUND_TEST_NAME) {
            _mesInfo->func = "AUDIO";
            _mesInfo->status = "FAIL";
            start_update_mes_log(_mesInfo);
            update_color_screen_log(SOUND_TEST_NAME + "结果：\t\t\t失败\n", "red");
            LOG_INFO("sound test result:\tFAIL\n");
            _funcFinishStatus->sound_finish = false;
        }
        if (func == DISPLAY_TEST_NAME) {
            _mesInfo->func = "DISPLAY";
            _mesInfo->status = "FAIL";
            start_update_mes_log(_mesInfo);
            update_color_screen_log(DISPLAY_TEST_NAME + "结果：\t\t\t失败\n", "red");
            LOG_INFO("display test result:\tFAIL\n");
            _funcFinishStatus->display_finish = false;
        }
        if (func == BRIGHT_TEST_NAME) {
            _mesInfo->func = "BRIGHTNESS";
            _mesInfo->status = "FAIL";
            start_update_mes_log(_mesInfo);
            update_color_screen_log(BRIGHT_TEST_NAME + "结果：\t\t\t失败\n", "red");
            LOG_INFO("bright test result:\tFAIL\n");
            _funcFinishStatus->bright_finish = false;
        }
        if (func == CAMERA_TEST_NAME) {
            CameraTest* camera = (CameraTest*)_funcBase[CAMERA];
            camera->close_xawtv_window();
            _mesInfo->func = "CAMERA";
            _mesInfo->status = "FAIL";
            start_update_mes_log(_mesInfo);
            update_color_screen_log(CAMERA_TEST_NAME + "结果：\t\t\t失败\n", "red");
            LOG_INFO("camera test result:\tFAIL\n");
            _funcFinishStatus->camera_finish = false;
        }
        if (func == STRESS_TEST_NAME) {
            LOG_INFO("stress test result:\tFAIL\n");
            StressTest* stress = (StressTest*)_funcBase[STRESS];
            string result = stress->get_stress_result_record();
            _record.push_back("FAIL  " + result);
            if (_record.size() > STRESS_RECORD_NUM) {
                _record.erase(_record.begin());
            }
            write_stress_record(_record);
            print_stress_test_result(_record);
            _funcFinishStatus->stress_finish = false;
        }
    }
    _uiHandle->set_test_result(func, result);
}

void Control::set_sn_mac_test_result(string sn_mac, string result)
{
    if (sn_mac == "MAC" && result == "PASS" && check_file_exit(WHOLE_TEST_FILE)) {
        sleep(1);
        _display_sn_or_mac = "SN";
        _uiHandle->show_sn_mac_message_box("SN");
    }
}

bool Control::is_stress_test_window_quit_safely()
{
    return _stress_test_window_quit_status;
}

void Control::slot_factory_delete_event()
{
    factory_delete_event();
}

bool Control::get_decode_status()
{
    return _uiHandle->get_g_decode_status();
}

void Control::factory_delete_event()
{
    LOG_INFO("factory test delete_event occurred.\n");
    SoundTest* sound = (SoundTest*) _funcBase[SOUND];
    sound->sound_record_restore(_baseInfo);
}


