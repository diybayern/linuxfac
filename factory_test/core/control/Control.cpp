#include "Control.h"
#include "fac_log.h"

#include <algorithm>

extern bool interfaceTestSelectStatus[INTERFACE_TEST_NUM];
extern bool interfaceTestResult[INTERFACE_TEST_NUM];
extern bool interfaceTestOver[INTERFACE_TEST_NUM];
extern bool interfaceTestFinish[INTERFACE_TEST_NUM];
extern bool funcFinishStatus[FUNC_TEST_NUM];

Control::Control():QObject()
{
    _funcBase[MEM]            = new MemTest();
    _funcBase[USB]            = new UsbTest();
    _funcBase[CPU]            = new CpuTest();
    _funcBase[EDID]           = new EdidTest();
    _funcBase[NET]            = new NetTest();
    _funcBase[HDD]            = new HddTest();
    _funcBase[SSD]            = new SsdTest();
    _funcBase[FAN]            = new FanTest();
    _funcBase[WIFI]           = new WifiTest();
    _funcBase[INTERFACE]      = new InterfaceTest();
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

Control::~Control()
{
    LOG_INFO("~Control()");
    for (int i = 0; i < FUNC_TYPE_NUM; i++) {
        if (_funcBase[i] != NULL) {
            if (i == NET) {
                NetTest* net = (NetTest*)_funcBase[NET];
                net->~NetTest();
            } else if (i == WIFI) {
                WifiTest* wifi = (WifiTest*)_funcBase[NET];
                wifi->~WifiTest();
            } else if (i == SOUND) {
                SoundTest* sound = (SoundTest*)_funcBase[NET];
                sound->~SoundTest();
            }
            delete _funcBase[i];
            _funcBase[i] = NULL;
        }
    }

    if (_baseInfo != NULL) {
        LOG_INFO("~baseInfo");
        delete _baseInfo;
        _baseInfo = NULL;
    }
        
    if (_hwInfo != NULL) {
        LOG_INFO("~_hwInfo");
        delete _hwInfo;
        _hwInfo = NULL;
    }

    if (_facArg != NULL) {
        LOG_INFO("~_facArg");
        delete _facArg;
        _facArg = NULL;
    }

    if (_mesInfo != NULL) {
        LOG_INFO("~_mesInfo");
        delete _mesInfo;
        _mesInfo = NULL;
    }
}

void Control::init_base_info()
{
    string baseinfo = execute_command("/usr/local/bin/system/getHWCfg", true);

    if (baseinfo != "error") {
        int len = baseinfo.size();
        if (baseinfo[0] == '{' && baseinfo[len - 1] == '}') {  // hwInfo string must have '{', '}'
            baseinfo = baseinfo.substr(1, baseinfo.length() - 2);  // delete '{', '}'
            get_baseinfo(_baseInfo, baseinfo);
            LOG_INFO("product is %s", (_baseInfo->platform).c_str());
        } else {
            LOG_INFO("base info is not right");
        }
    } else {
        LOG_ERROR("get hwcfg.ini information error");
    }
    
    if (_baseInfo->platform == "") {  // platform is IDV, VDI or null
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
        //input -1 to get actual linked edid num
        _uiHandle->add_main_label("EDID信息:", to_string(edid_read_i2c_test(-1)));
        
        string real_total_num = execute_command("lsusb -t | grep \"Mass Storage\" | wc -l", true);
        string real_num_3 = execute_command("lsusb -t | grep \"Mass Storage\" | grep \"5000M\" | wc -l", true);
        if (real_total_num != "error") {
            _baseInfo->usb_total_num = real_total_num;
        } else if (real_num_3 != "error") {
            _baseInfo->usb_3_num = real_num_3;
        }
        _uiHandle->add_main_label("USB信息:", _baseInfo->usb_3_num + "/" + _baseInfo->usb_total_num);
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
        
        _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_INTERFACE]);
        _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_MEM]);
        
        if (_baseInfo->usb_total_num == "0") { // do not need test usb when it is third product and no usb
            interfaceTestSelectStatus[I_USB] = false;
            interfaceTestFinish[I_USB]       = true;
            interfaceTestOver[I_USB]         = true;
        } else {
            _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_USB]);
        }
        
        _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_NET]);
        
        interfaceTestSelectStatus[I_EDID] = false;
        interfaceTestFinish[I_EDID]       = true;
        interfaceTestOver[I_EDID]         = true;
        
        interfaceTestSelectStatus[I_CPU]  = false;
        interfaceTestFinish[I_CPU]        = true;
        interfaceTestOver[I_CPU]          = true;

        if (_baseInfo->hdd_cap != "0" && _baseInfo->hdd_cap != "") {
            _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_HDD]);
        } else {
            interfaceTestSelectStatus[I_HDD] = false;
            interfaceTestFinish[I_HDD]       = true;
            interfaceTestOver[I_HDD]         = true;
        }

        if (_baseInfo->ssd_cap != "0" && _baseInfo->ssd_cap != "") {
            _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_SSD]);
        } else {
            interfaceTestSelectStatus[I_SSD] = false;
            interfaceTestFinish[I_SSD]       = true;
            interfaceTestOver[I_SSD]         = true;
        }

        interfaceTestSelectStatus[I_FAN] = false;
        interfaceTestFinish[I_FAN]       = true;
        interfaceTestOver[I_FAN]         = true;

        interfaceTestSelectStatus[I_WIFI] = false;
        interfaceTestFinish[I_WIFI]       = true;
        interfaceTestOver[I_WIFI]         = true;

        _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_SOUND]);
        _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_DISPLAY]);

        funcFinishStatus[F_BRIGHT] = true;

        if (_baseInfo->camera_exist != "0" && _baseInfo->camera_exist != "") {
            _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_CAMERA]);
        } else {
            funcFinishStatus[F_CAMERA] = true;
        }

        _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_STRESS]);
        _uiHandle->add_complete_or_single_test_label("第三方终端");
        _uiHandle->sync_main_test_ui(true);
        
    }else {
    
        _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_INTERFACE]);
        _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_MEM]);
        _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_USB]);
        _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_NET]);
        _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_EDID]);
        _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_CPU]);
        
        if (_baseInfo->hdd_cap != "0" && _baseInfo->hdd_cap != "") {
            _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_HDD]);
        } else {
            interfaceTestSelectStatus[I_HDD] = false;
            interfaceTestFinish[I_HDD]       = true;
            interfaceTestOver[I_HDD]         = true;
        }

        if (_baseInfo->ssd_cap != "0" && _baseInfo->ssd_cap != "") {
            _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_SSD]);
        } else {
            interfaceTestSelectStatus[I_SSD] = false;
            interfaceTestFinish[I_SSD]       = true;
            interfaceTestOver[I_SSD]         = true;
        }

        if (_baseInfo->fan_speed != "0" && _baseInfo->fan_speed != "") {
            _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_FAN]);
        } else {
            interfaceTestSelectStatus[I_FAN] = false;
            interfaceTestFinish[I_FAN]       = true;
            interfaceTestOver[I_FAN]         = true;
        }

        if (_baseInfo->wifi_exist != "0" && _baseInfo->wifi_exist != "") {
            _uiHandle->add_interface_test_button(INTERFACE_TEST_NAME[I_WIFI]);
        } else {
            interfaceTestSelectStatus[I_WIFI] = false;
            interfaceTestFinish[I_WIFI]       = true;
            interfaceTestOver[I_WIFI]         = true;
        }

        _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_SOUND]);
        _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_DISPLAY]);

        if (_baseInfo->bright_level != "0" && _baseInfo->bright_level != "") {
            _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_BRIGHT]);
        } else {
            funcFinishStatus[F_BRIGHT] = true;
        }

        if (_baseInfo->camera_exist != "0" && _baseInfo->camera_exist != "") {
            _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_CAMERA]);
        } else {
            funcFinishStatus[F_CAMERA] = true;
        }

        _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_STRESS]);
        _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_UPLOAD_MES_LOG]);
        
        if (!check_file_exit(WHOLE_TEST_FILE)) {
            _uiHandle->add_main_test_button(FUNC_TEST_NAME[F_NEXT_PROCESS]);
        }    

        if (check_file_exit(WHOLE_TEST_FILE)) {
            _uiHandle->add_complete_or_single_test_label("整机测试");
        } else {
            _uiHandle->add_complete_or_single_test_label("单板测试");
        }

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

    connect(_uiHandle->get_qobject(FUNC_TEST_NAME[F_INTERFACE]), SIGNAL(clicked()), this, SLOT(start_interface_test()));           
    connect(_uiHandle->get_qobject(FUNC_TEST_NAME[F_SOUND]), SIGNAL(clicked()), this, SLOT(start_sound_test()));
    connect(_uiHandle->get_qobject(FUNC_TEST_NAME[F_DISPLAY]), SIGNAL(clicked()), this, SLOT(start_display_test()));
    if (_baseInfo->bright_level != "0" && _baseInfo->bright_level != ""){
        connect(_uiHandle->get_qobject(FUNC_TEST_NAME[F_BRIGHT]), SIGNAL(clicked()), this, SLOT(start_bright_test()));
    }
    if (_baseInfo->camera_exist != "0" && _baseInfo->camera_exist != "") {
        connect(_uiHandle->get_qobject(FUNC_TEST_NAME[F_CAMERA]), SIGNAL(clicked()), this, SLOT(start_camera_test()));
    }

    if (!_is_third_product) {
        connect(_uiHandle->get_qobject(FUNC_TEST_NAME[F_UPLOAD_MES_LOG]), SIGNAL(clicked()), this, SLOT(start_upload_log()));

        if (!check_file_exit(WHOLE_TEST_FILE)) {
            connect(_uiHandle->get_qobject(FUNC_TEST_NAME[F_NEXT_PROCESS]), SIGNAL(clicked()), this, SLOT(start_next_process()));
        }
    }

    connect(_uiHandle->get_qobject(FUNC_TEST_NAME[F_STRESS]), SIGNAL(clicked()), this, SLOT(start_stress_test()));
    connect(_uiHandle, SIGNAL(to_show_test_confirm_dialog(string)), this, SLOT(show_test_confirm_dialog(string)));
    connect(_uiHandle, SIGNAL(sig_ui_handled_test_result(string, string)), this, SLOT(set_test_result_pass_or_fail(string, string)));
    connect(_uiHandle, SIGNAL(sig_ui_handled_sn_mac_test_result(string, string)), this, SLOT(set_sn_mac_test_result(string, string)));
    connect(_uiHandle, SIGNAL(sig_ui_check_state_changed(string, bool)), this, SLOT(set_interface_select_status(string, bool)));
    connect(_uiHandle, SIGNAL(sig_ui_get_message_from_scangun(string)), this, SLOT(check_sn_mac_compare_result(string)));
    connect(_uiHandle, SIGNAL(sig_ui_confirm_shut_down_or_next_process(string)), this, SLOT(confirm_shut_down_or_next_process(string)));
    connect(_uiHandle, SIGNAL(sig_ui_retry_sn_mac()), this, SLOT(retry_sn_mac_test()));
    connect(_uiHandle, SIGNAL(sig_ui_factory_delete_event()), this, SLOT(slot_factory_delete_event()));
}

/* third products show net speed and duplex, just need test send and recv msg */
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

/* try again after sn or mac test failed */
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

/* after upload success */
void Control::confirm_shut_down_or_next_process(string process)
{
    if (process == FUNC_TEST_NAME[F_NEXT_PROCESS]) {
        _funcBase[NEXT_PROCESS]->start_test(_baseInfo);
    } else if (process == "关机") {
        if (execute_command("shutdown -h now", true) == "error"){
            LOG_ERROR("shutdown cmd run error\n");            
            _uiHandle->confirm_test_result_warning("终端异常，无法关机！");
        }
    }
}

/* compare input string with system mac or sn */
void Control::check_sn_mac_compare_result(string message)
{
    if (_display_sn_or_mac == "MAC") {
        string mac = _hwInfo->mac;
        mac.erase(remove(mac.begin(), mac.end(), ':'), mac.end()); // delete ':' in mac
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

/* show result confirmation box to choose PASS or FAIL ----display, stress */
void Control::show_test_confirm_dialog(string item) //TODO: combine with confirm_test_result(string)
{
    if (item == "") {
        LOG_ERROR("func name is NULL");
        return;
    }
    
    LOG_INFO("confirm %s result", item.c_str());
    if (item.compare(FUNC_TEST_NAME[F_STRESS]) == 0) {
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
    if (!usb->usb_test_read_status()) { // copy fac_config.conf from usb to /tmp/
        LOG_ERROR("init copy fac config error");
    }
    /* Read FAC_CONFIG_FILE regardless of whether or not usb file exists */
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
    update_screen_log("==================== " + FUNC_TEST_NAME[F_DISPLAY] + " ====================\n");
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
    if (func == "" || result == "" || ui_log == "") {
        LOG_ERROR("parameters is wrong");
        return;
    }
    _uiHandle->set_test_result(func, result);
    _uiHandle->update_screen_log(ui_log);
}

/* Set PASS button is not clickable before brightness test is finished*/
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

/* print stress record on screen */
void Control::print_stress_test_result(vector<string> record) 
{
    update_screen_log("The last Stress test result is...\n");
    update_screen_log("==================== " + FUNC_TEST_NAME[F_STRESS] + "结果 ====================\n");

    for (unsigned int i = 0; i < record.size(); i++) {
        update_screen_log(record[i]);
    }
    update_screen_log("==================================================\n");
}

/* auto test mac when it is not third-part product and there is not stress file */
void Control::auto_test_mac_or_stress() {    
    if (check_file_exit(STRESS_LOCK_FILE)) {
        LOG_INFO("******************** auto start stress test ********************");
        _funcBase[STRESS]->start_test(_baseInfo);
    } else if (!_is_third_product){
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
    string date = get_current_time();
    string tmp = "";
    string mac_capital = _hwInfo->mac;
    string sn_capital = _hwInfo->sn;
    
    mac_capital.erase(remove(mac_capital.begin(), mac_capital.end(), ':'), mac_capital.end());
    
    if (access(MES_FILE.c_str(), F_OK) == 0) {
        remove(MES_FILE.c_str());
    }

    tmp = _facArg->ftp_dest_path + mac_capital + ".txt";
    _facArg->ftp_dest_path = tmp;
    LOG_INFO("_facArg->ftp_dest_path:%s", (_facArg->ftp_dest_path).c_str());
    
    LOG_MES("---------------------Product infomation-----------------------\n");
    LOG_MES("Model: \t%s\n", (_hwInfo->product_name).c_str());
    LOG_MES("SN: \t%s\n", sn_capital.c_str());
    LOG_MES("MAC: \t%s\n", mac_capital.c_str());
    LOG_MES("DATE: \t%s\n", date.c_str());
    LOG_MES("OPERATION: \t%s\n", (_facArg->ftp_job_number).c_str());
    LOG_MES("---------------------Simple test result-----------------------\n");
    LOG_MES("%s:       NULL\n", INTERFACE_TEST_MES_TAG[I_MEM].c_str());
    LOG_MES("%s:       NULL\n", INTERFACE_TEST_MES_TAG[I_USB].c_str());
    LOG_MES("%s:       NULL\n", INTERFACE_TEST_MES_TAG[I_NET].c_str());
    LOG_MES("%s:      NULL\n", INTERFACE_TEST_MES_TAG[I_EDID].c_str());
    LOG_MES("%s:       NULL\n", INTERFACE_TEST_MES_TAG[I_CPU].c_str());
    if (_baseInfo->hdd_cap != "0" && _baseInfo->hdd_cap != "") {
        LOG_MES("%s:       NULL\n", INTERFACE_TEST_MES_TAG[I_HDD].c_str());
    }
    if (_baseInfo->ssd_cap != "0" && _baseInfo->ssd_cap != "") {
        LOG_MES("%s:       NULL\n", INTERFACE_TEST_MES_TAG[I_SSD].c_str());
    }
    if (_baseInfo->fan_speed != "0" && _baseInfo->fan_speed != "") {
        LOG_MES("%s:       NULL\n", INTERFACE_TEST_MES_TAG[I_FAN].c_str());
    }
    if (_baseInfo->wifi_exist != "0" && _baseInfo->wifi_exist != "") {
        LOG_MES("%s:      NULL\n", INTERFACE_TEST_MES_TAG[I_WIFI].c_str());
    }
    LOG_MES("%s:     NULL\n", FUNC_TEST_TAG_NAME[F_SOUND].c_str());
    LOG_MES("%s:   NULL\n", FUNC_TEST_TAG_NAME[F_DISPLAY].c_str());
    if (_baseInfo->bright_level != "0" && _baseInfo->bright_level != "") {
        LOG_MES("%s:    NULL\n", FUNC_TEST_TAG_NAME[F_BRIGHT].c_str());
    }
    if (_baseInfo->camera_exist != "0" && _baseInfo->camera_exist != "") {
        LOG_MES("%s:    NULL\n", FUNC_TEST_TAG_NAME[F_CAMERA].c_str());
    }
    LOG_MES("---------------------Stress test result-----------------------\n");
}

void Control::update_mes_log(string tag, string value)
{
    if (tag == "" || value == "") {
        return;
    }
    
    FILE* fp;
    char line[200] = {0, };  //TODO: char[] line
    size_t sp = 0;
    int first = 1;
    
    fp = fopen(MES_FILE.c_str(), "r");
    if (fp == NULL) {
        LOG_ERROR("open %s failed", MES_FILE.c_str());
        return;
    }
 
    fseek(fp, 0, SEEK_SET);
    string buf = "";

    while (fgets(line, sizeof(line), fp) != NULL) {  //TODO: fgets(char* , )
        string str_line = line;
        if (first &&((sp = str_line.find(tag)) != str_line.npos)) {
            string value_temp = "";
            value_temp = value + "\n";
            str_line.replace(sp + 11, value_temp.size(), value_temp); //convert NULL to test result in MES_FILE
            first = 0;
        }
        buf += str_line;
    }
 
    fclose(fp);
    fp = fopen(MES_FILE.c_str(), "w");
    if (fp == NULL) {
        return;
    }

    if (buf != "") {
        int len = buf.size();
        fwrite(buf.c_str(), 1, len, fp);
    }
    fclose(fp);
}


void Control::upload_mes_log()
{
    update_screen_log("==================== " + FUNC_TEST_NAME[F_UPLOAD_MES_LOG] + " ====================\n");
    if (_fac_config_status != 0) {
        LOG_INFO("fac config is wrong, do not upload");
        _uiHandle->confirm_test_result_warning("配置文件有误");
        set_test_result(FUNC_TEST_NAME[F_UPLOAD_MES_LOG], "FAIL", "配置文件有误");
        return;
    } else {
        if (!combine_fac_log_to_mes(MES_FILE, STRESS_RECORD)) { // combine test result file and stress record file
            update_color_screen_log("拷机记录文件为空\n", "red");
            LOG_MES("no stress test record\n");
            LOG_INFO("NO stress record\n");
        }
        LOG_MES("---------------------Detail test result-----------------------\n");
        if (!combine_fac_log_to_mes(MES_FILE, LOG_FILE)) { // combine test result file and detail log file
            LOG_ERROR("combine log failed");
            _uiHandle->confirm_test_result_warning("log文件拼接失败");
            return;
        }
        string upload_log = "";
        upload_log  = "ftp ip:\t\t" + _facArg->ftp_ip + "\n";
        upload_log += "ftp user:\t\t" + _facArg->ftp_user + "\n";
        upload_log += "ftp passwd:\t\t" + _facArg->ftp_passwd + "\n";
        upload_log += "ftp path:\t\t" + _facArg->ftp_dest_path + "\n";
        update_screen_log(upload_log);

        _uiHandle->confirm_test_result_waiting("正在上传中...");
        sleep(1);
        string response = ftp_send_file(MES_FILE, _facArg); // upload log
        response = response_to_chinese(response);  // translate upload result to chinese
        LOG_INFO("upload log: %s", response.c_str());
        if (response.compare("上传成功") == 0) {
            if (check_file_exit(WHOLE_TEST_FILE)) {
                _uiHandle->confirm_test_result_success("上传成功", "关机");
            } else {
                _uiHandle->confirm_test_result_success("上传成功", "下道工序");
            }
            set_test_result(FUNC_TEST_NAME[F_UPLOAD_MES_LOG], "PASS", response);
        } else {
            _uiHandle->confirm_test_result_warning("上传失败");
            set_test_result(FUNC_TEST_NAME[F_UPLOAD_MES_LOG], "FAIL", response);
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

void Control::set_interface_select_status(string func, bool state)
{
    funcFinishStatus[F_INTERFACE] = false;

    for (int i = 0; i < INTERFACE_TEST_NUM; i++) {
        if (func == INTERFACE_TEST_NAME[i]) {
            interfaceTestSelectStatus[i] = state;
            /* when selection status changed, test over and finish status need to be changed */
            interfaceTestFinish[i] = (!state);
            interfaceTestOver[i]   = (!state);
            break;
        }
    }
}

void Control::set_interface_test_status(string func, bool status)
{
    for (int i = 0; i < INTERFACE_TEST_NUM; i++) {
        if (func == INTERFACE_TEST_NAME[i]) {
            interfaceTestOver[i] = status;
            break;
        }
    }
}

void Control::set_interface_test_finish(string func)
{
    for (int i = 0; i < INTERFACE_TEST_NUM; i++) {
        if (func == INTERFACE_TEST_NAME[i]) {
            interfaceTestFinish[i] = true;
            break;
        }
    }
}

void Control::set_interface_test_result(string func, bool status)
{
    for (int i = 0; i < INTERFACE_TEST_NUM; i++) {
        if (func == INTERFACE_TEST_NAME[i]) {
            interfaceTestResult[i] = status;
            break;
        }
    }
}

void* Control::update_mes_log_thread(void* arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is NULL");
        return NULL;
    }
    MesInfo* info = (MesInfo*)arg;
    Control::get_control()->update_mes_log(info->func, info->status);
    return NULL;
}

void Control::start_update_mes_log(MesInfo* info)
{
    if (info == NULL) {
        LOG_ERROR("info is NULL");
        return;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, update_mes_log_thread, info);
}

/* 
** after clicked confirm box: 
** show result on the right side of button, screen and file log;
** stress test show record on the screen;
*/
void Control::set_test_result_pass_or_fail(string func, string result)
{
    if (func == FUNC_TEST_NAME[F_STRESS]) {
        LOG_INFO("stress test result:\t%s\n", result.c_str());
        StressTest* stress = (StressTest*)_funcBase[STRESS];
        string stress_result = stress->get_stress_result_record();
        _record.push_back(result + "  " + stress_result);
        if (_record.size() > STRESS_RECORD_NUM) {
            _record.erase(_record.begin());
        }
        write_stress_record(_record);
        print_stress_test_result(_record);
        funcFinishStatus[F_STRESS] = BOOL_RESULT(result);
    } else {
        int fun_id = 0;
        for (int i = F_SOUND; i < F_STRESS; i++) {
            if (func == FUNC_TEST_NAME[i]) {
                fun_id = i;
                break;
            }
        }
        if (fun_id == F_CAMERA) {
            /* close camera window after test finished */
            CameraTest* camera = (CameraTest*)_funcBase[CAMERA];
            camera->close_xawtv_window();
        }
        _mesInfo->func = FUNC_TEST_TAG_NAME[fun_id];
        _mesInfo->status = result;
        start_update_mes_log(_mesInfo);
        update_screen_log(FUNC_TEST_NAME[fun_id] + "结果：\t\t\t" + CHINESE_RESULT(result) + "\n");
        string log_info = FUNC_TEST_TAG_NAME[fun_id] + " test result:\t" + result + "\n";
        LOG_INFO(log_info.c_str());
        funcFinishStatus[fun_id] = BOOL_RESULT(result);
    }
    _uiHandle->set_test_result(func, result);
}

/* after the mac test is passed, show sn test box (whole test) */
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
    _control->~Control();
}


