#include "../../inc/Control.h"
#include "../../inc/fac_log.h"

#include <algorithm>

Control::Control():QObject()
{
    _funcBase[INTERFACE]    = new InterfaceTest();
    _funcBase[MEM]          = new MemTest();
    _funcBase[USB]          = new UsbTest();
    _funcBase[CPU]          = new CpuTest();
    _funcBase[EDID]         = new EdidTest();
    _funcBase[NET]          = new NetTest();
    _funcBase[HDD]          = new HddTest();
    _funcBase[FAN]          = new FanTest();
    _funcBase[WIFI]         = new WifiTest();
    _funcBase[SOUND]        = new SoundTest();
    _funcBase[BRIGHT]       = new BrightTest();
    _funcBase[CAMERA]       = new CameraTest();
    _funcBase[STRESS]       = new StressTest();
    _funcBase[NEXT_PROCESS] = new NextProcess();
    
    _uiHandle               = UiHandle::get_uihandle();

    
    _baseInfo               = new BaseInfo;
    _hwInfo                 = new HwInfo;
    _facArg                 = new FacArg;

    _funcFinishStatus                   = new FuncFinishStatus;
    _funcFinishStatus->interface_finish = false;
    _funcFinishStatus->mem_finish       = false;
    _funcFinishStatus->usb_finish       = false;
    _funcFinishStatus->cpu_finish       = false;
    _funcFinishStatus->net_finish       = false;
    _funcFinishStatus->edid_finish      = false;
    _funcFinishStatus->hdd_finish       = false;
    _funcFinishStatus->fan_finish       = false;
    _funcFinishStatus->wifi_finish      = false;
    _funcFinishStatus->sound_finish     = false;
    _funcFinishStatus->display_finish   = false;
    _funcFinishStatus->bright_finish    = false;
    _funcFinishStatus->camera_finish    = false;
    _funcFinishStatus->stress_finish    = false;

    _interfaceTestStatus                 = new InterfaceTestStatus;    
	_interfaceTestStatus->cpu_test_over  = false;
	_interfaceTestStatus->mem_test_over  = false;
	_interfaceTestStatus->usb_test_over  = false;
	_interfaceTestStatus->edid_test_over = false;
	_interfaceTestStatus->net_test_over  = false;
	_interfaceTestStatus->hdd_test_over  = false;
	_interfaceTestStatus->fan_test_over  = false;
	_interfaceTestStatus->wifi_test_over = false;

    _interfaceSelectStatus              = new InterfaceSelectStatus;
    _interfaceSelectStatus->mem_select  = true;
    _interfaceSelectStatus->usb_select  = true;
    _interfaceSelectStatus->cpu_select  = true;
    _interfaceSelectStatus->net_select  = true;
    _interfaceSelectStatus->edid_select = true;
    _interfaceSelectStatus->hdd_select  = true;
    _interfaceSelectStatus->fan_select  = true;
    _interfaceSelectStatus->wifi_select = true;

	_interfaceTestResult                = new InterfaceTestResult;

    _testStep = STEP_IDLE;
	_interfaceRunStatus = INF_RUNEND;
    _stress_test_stage = "";
    _autoUploadLog = true;
    _mes_log_file = "";
	_auto_upload_mes = true;
    _is_idv = true;
	
    init_base_info();
    init_hw_info();
    ui_init();
    init_fac_config();
    init_mes_log();

    auto_start_stress_test();
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
    string dmi = execute_command("cat " + GET_BASEINFO_INI);
	if (dmi != "error") {
		get_baseinfo(_baseInfo,dmi);
	} else {
		LOG_ERROR("get hwcfg.ini information error");
	}
}

void Control::init_hw_info()
{
    get_hwinfo(_hwInfo);
}

void Control::ui_init()
{
	if (check_file_exit(WHOLE_TEST_FILE)) {
		_whole_test_state = true;
	} else {
		_whole_test_state = false;
	}
	
    _uiHandle->add_main_label("产品型号:", _hwInfo->product_name);
    _uiHandle->add_main_label("硬件版本:", _hwInfo->product_hw_version);
    _uiHandle->add_main_label("SN序列号:", _hwInfo->sn);
    _uiHandle->add_main_label("MAC地址:", _hwInfo->mac);
   // _uiHandle->add_main_label("CPU型号:", _hwInfo->cpu_type);
    
    _uiHandle->add_main_test_button(INTERFACE_TEST_NAME);
    
    _uiHandle->add_interface_test_button(MEM_TEST_NAME);
    _uiHandle->add_interface_test_button(USB_TEST_NAME);
    _uiHandle->add_interface_test_button(NET_TEST_NAME);
    _uiHandle->add_interface_test_button(EDID_TEST_NAME);
    _uiHandle->add_interface_test_button(CPU_TEST_NAME);
    
    if (_baseInfo->hdd_cap != "0" && _baseInfo->hdd_cap != "") {
        _uiHandle->add_interface_test_button(HDD_TEST_NAME);
    } else {
        _funcFinishStatus->hdd_finish = true;
		_interfaceTestStatus->hdd_test_over = true;
    }
    
    if (_baseInfo->fan_speed != "0" && _baseInfo->fan_speed != "") {
        _uiHandle->add_interface_test_button(FAN_TEST_NAME);
    } else {
        _funcFinishStatus->fan_finish = true;
		_interfaceTestStatus->fan_test_over = true;
    }

    if (_baseInfo->wifi_exist != "0" && _baseInfo->wifi_exist != "") {
        _uiHandle->add_interface_test_button(WIFI_TEST_NAME);
    } else {
        _funcFinishStatus->wifi_finish = true;
		_interfaceTestStatus->wifi_test_over = true;
    }

    _uiHandle->add_main_test_button(SOUND_TEST_NAME);
    _uiHandle->add_main_test_button(DISPLAY_TEST_NAME);
    
    if (_baseInfo->bright_level != "0" && _baseInfo->bright_level != ""){
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
    _uiHandle->add_main_test_button(UPLOAD_LOG_NAME);

    if (_baseInfo->hdd_cap != "0" && _baseInfo->hdd_cap != "") {
        if (!_whole_test_state) {
             _uiHandle->add_main_test_button(NEXT_PROCESS_NAME);
        }
    }    
    
    if (_whole_test_state) {
        _uiHandle->add_complete_or_single_test_label("整机测试");
    } else {
        _uiHandle->add_complete_or_single_test_label("单板测试");
    }
	_uiHandle->set_is_complete_test(_whole_test_state);
	
    _uiHandle->sync_main_test_ui();
    
    _uiHandle->add_stress_test_label("运行时间");
    _uiHandle->add_stress_test_label("CPU温度");
    _uiHandle->add_stress_test_label("编码状态");
    _uiHandle->add_stress_test_label("解码状态");
    _uiHandle->add_stress_test_label("Mem");
    _uiHandle->add_stress_test_label("Cpu");
    _uiHandle->add_stress_test_label("CPU频率");
    _uiHandle->add_stress_test_label("SN序列号");
    _uiHandle->add_stress_test_label("MAC地址");
    _uiHandle->add_stress_test_label("产品型号");
    _uiHandle->add_stress_test_label("硬件版本");
    
    connect(_uiHandle->get_qobject(INTERFACE_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_interface_test()));
    connect(_uiHandle->get_qobject(SOUND_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_sound_test()));
    connect(_uiHandle->get_qobject(DISPLAY_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_display_test()));

    if (_baseInfo->bright_level != "0" && _baseInfo->bright_level != ""){
        connect(_uiHandle->get_qobject(BRIGHT_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_bright_test()));
    }
    if (_baseInfo->camera_exist != "0" && _baseInfo->camera_exist != "") {
        connect(_uiHandle->get_qobject(CAMERA_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_camera_test()));
    }
    connect(_uiHandle->get_qobject(STRESS_TEST_NAME), SIGNAL(clicked()), this, SLOT(start_stress_test()));
    connect(_uiHandle->get_qobject(UPLOAD_LOG_NAME), SIGNAL(clicked()), this, SLOT(start_upload_log()));

	if (_baseInfo->hdd_cap != "0" && _baseInfo->hdd_cap != "") {
        if (!_whole_test_state) {
    		connect(_uiHandle->get_qobject(NEXT_PROCESS_NAME), SIGNAL(clicked()), this, SLOT(start_next_process()));
        }
	}
	connect(_uiHandle, SIGNAL(to_show_test_confirm_dialog(string)), this, SLOT(show_test_confirm_dialog(string)));
    connect(_uiHandle, SIGNAL(sig_ui_handled_test_result(string, string)), this, SLOT(set_test_result_pass_or_fail(string, string)));
    connect(_uiHandle, SIGNAL(sig_ui_handled_sn_mac_test_result(string, string)), this, SLOT(set_sn_mac_test_result(string, string)));
    connect(_uiHandle, SIGNAL(sig_ui_check_state_changed(string, bool)), this, SLOT(set_interface_select_status(string, bool)));
    connect(_uiHandle, SIGNAL(sig_ui_get_message_from_scangun(string)), this, SLOT(check_sn_mac_compare_result(string)));
    connect(_uiHandle, SIGNAL(sig_ui_confirm_shut_down_or_next_process(string)), this, SLOT(confirm_shut_down_or_next_process(string)));
    connect(_uiHandle, SIGNAL(sig_ui_retry_sn_mac()), this, SLOT(retry_sn_mac_test()));
}

void Control::retry_sn_mac_test()
{
	if (_sn_mac == "MAC") {
		_uiHandle->show_sn_mac_message_box("MAC");
	} else if (_sn_mac == "SN") {
		_uiHandle->show_sn_mac_message_box("SN");
	}
}

void Control::confirm_shut_down_or_next_process(string process)
{
	if (process == NEXT_PROCESS_NAME) {
		_funcBase[NEXT_PROCESS]->start_test(_baseInfo);
	} else if (process == "关机") {
		if (execute_command("shutdown -h now") == "error"){
            LOG_ERROR("shutdown cmd run error\n");			
            _uiHandle->confirm_test_result_warning("终端异常，无法关机！");
		}
	}
}

void Control::check_sn_mac_compare_result(string message)
{
    if (_sn_mac == "MAC") {
		string mac = _hwInfo->mac;
		mac.erase(remove(mac.begin(), mac.end(), ':'), mac.end());
		if (message.size() != mac.size() || message != mac) {
			_uiHandle->update_sn_mac_test_result("MAC", "FAIL");
			_uiHandle->show_sn_mac_comparison_result("MAC", "FAIL");
			return ;
		} else {
			_uiHandle->update_sn_mac_test_result("MAC", "PASS");
			_uiHandle->show_sn_mac_comparison_result("MAC", "PASS");
			return ;
		}
    }

	if (_sn_mac == "SN") {
		string sn = _hwInfo->sn;
		if (message.size() != sn.size() || message != sn) {
			_uiHandle->update_sn_mac_test_result("SN", "FAIL");
			_uiHandle->show_sn_mac_comparison_result("SN", "FAIL");
			return ;
		} else {
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
	SoundTest* sound = (SoundTest*) _funcBase[SOUND];
    sound->init();
	
	NetTest* net = (NetTest*) _funcBase[NET];
    net->init();

	WifiTest* wifi = (WifiTest*) _funcBase[WIFI];
    wifi->init();
	
	NextProcess* nextpro = (NextProcess*) _funcBase[NEXT_PROCESS];
    nextpro->init();
}

void Control::init_fac_config()
{
	UsbTest* usb = (UsbTest*)_funcBase[USB];
	if (!usb->usb_test_read_status()) {
		LOG_ERROR("init copy fac config error");
	}
    _fac_config_status = get_fac_config_from_conf(FAC_CONFIG_FILE, _facArg);
    if (_fac_config_status == NO_FTP_PATH) {
        LOG_INFO("NO_FTP_PATH");
    } else if (_fac_config_status == NO_JOB_NUMBER) {
        LOG_INFO("NO_JOB_NUMBER");
    } else if (_fac_config_status == (NO_FTP_PATH + NO_JOB_NUMBER)) {
        LOG_INFO("NO_FTP_PATH and NO_JOB_NUMBER");
    } 
}

void Control::start_interface_test()
{
    LOG_INFO("start interface test");
    _testStep = STEP_INTERFACE;
	_funcBase[INTERFACE]->start_test(_baseInfo);
}

void Control::start_sound_test()
{
    _funcBase[SOUND]->start_test(_baseInfo);
    LOG_INFO("start sound test");
}

void Control::start_display_test()
{
    _uiHandle->show_display_ui();
    LOG_INFO("start display test");
}

void Control::start_bright_test()
{
    _funcBase[BRIGHT]->start_test(_baseInfo);
    LOG_INFO("start bright test");
}

void Control::start_camera_test()
{
    _funcBase[CAMERA]->start_test(_baseInfo);
    LOG_INFO("start camera test");
}

void Control::start_stress_test()
{
    _funcBase[STRESS]->start_test(_baseInfo);
    LOG_INFO("start_stress_test");
}

void Control::start_next_process()
{
    _funcBase[NEXT_PROCESS]->start_test(_baseInfo);
    LOG_INFO("start next process");
}

void Control::start_upload_log()
{
    LOG_INFO("start upload log");
    upload_mes_log();
}


void Control::set_test_result(string func,string result,string ui_log)

{
    _uiHandle->set_test_result(func, result);
    _uiHandle->update_screen_log(ui_log);
}

void Control::confirm_test_result(string func)
{
    LOG_INFO("confirm_test_result");
    _uiHandle->confirm_test_result_dialog(func);
}

void Control::show_main_test_ui()
{
    _uiHandle->to_show_main_test_ui();
	auto_test_mac_sn();
}

void Control::auto_test_mac_sn() {
	if (!_lock_file_status) {
		_sn_mac = "MAC";
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
    char date[64] = { 0, };
    char new_mac_name[128];
    struct tm *timenow = NULL;
    time_t timep;
 
    while(_hwInfo->mac[i] != '\0'){
        if(_hwInfo->mac[i] != ':'){
            new_mac_name[j] = _hwInfo->mac[i];
            j++;
        }
        i++;
    }

    new_mac_name[j] = '\0';
    char* mac_capital = (char*)malloc(128);
    char* sn_capital = (char*)malloc(128);
    
    mac_capital = lower_to_capital(new_mac_name, mac_capital);
    sn_capital = lower_to_capital(_hwInfo->sn.c_str(), sn_capital);
    
    if (access(MES_FILE, F_OK) == 0) {
        remove(MES_FILE);
    }

    sprintf(_facArg->ftp_dest_path,"%s%s.txt",_facArg->ftp_dest_path,mac_capital);
    LOG_INFO("_facArg->ftp_dest_path:%s",_facArg->ftp_dest_path);
    
    time(&timep);
    timenow = localtime(&timep);
    strftime(date, 64, "%Y%m%d-%H:%M:%S", timenow);



    LOG_MES("---------------------Product infomation-----------------------\n");
    LOG_MES("Model: \t%s\n", _hwInfo->product_name.c_str());
    LOG_MES("SN: \t%s\n", sn_capital);
    LOG_MES("MAC: \t%s\n", mac_capital);
    LOG_MES("DATE: \t%s\n", date);
    LOG_MES("OPERATION: \t%s\n", _facArg->ftp_job_number);
    LOG_MES("---------------------Simple test result-----------------------\n");
    LOG_MES("MEMORY:    NULL\n");
    LOG_MES("USB:       NULL\n");
    LOG_MES("NET:       NULL\n");
    LOG_MES("EDID:      NULL\n");
    if (_baseInfo->hdd_cap != "0" && _baseInfo->hdd_cap != "") {
        LOG_MES("HDD:       NULL\n");
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
    LOG_MES("STRESS:    NULL\n");
    LOG_MES("---------------------Detail test result-----------------------\n");

    free(mac_capital);
    free(sn_capital);
}

void Control::update_mes_log(string tag,string value)
{
    FILE* fp;
    char line[200];
    char* sp = NULL;
    int file_size;
    int first=1;
 
    bzero(line,sizeof(line));
 
    if ((fp = fopen(MES_FILE,"r"))==NULL) {
        LOG_ERROR("open %s failed",MES_FILE);
    }
 
    fseek(fp,0,SEEK_END);
    file_size = ftell(fp);
    fseek(fp,0,SEEK_SET);
    char* buf = (char*)malloc(file_size+128);
    bzero(buf,file_size);
 
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (first &&((sp = strstr(line, tag.c_str())) != NULL)) {
            char value_temp[128];
            bzero(value_temp,128);
            sprintf(value_temp,"%s\n",value.c_str());
            memcpy(sp+11,value_temp,strlen(value_temp)+1);//修改标签内容，加TAG_OFFSET是为了对齐值
            first = 0;
        }
        strcat(buf, line);
    }
 
    fclose(fp);
    fp = fopen(MES_FILE, "w");
    int len = strlen(buf);
    fwrite(buf, 1, len, fp);
    fclose(fp);

    if (buf != NULL) {
        free(buf);
    }
}


void Control::upload_mes_log() {
	update_screen_log("==================== upload log ====================\n");
	if (_fac_config_status != 0) {
		LOG_INFO("fac config is wrong, do not upload");
		_uiHandle->confirm_test_result_warning("配置文件有误");
		set_test_result(UPLOAD_LOG_NAME,"FAIL","配置文件有误");
		return;
	} else if (combine_fac_log_to_mes(MES_FILE)) {
		LOG_INFO("ftp ip:%s\tftp user:%s\tftp passwd:%s\tftp path:%s\t",
					_facArg->ftp_ip,_facArg->ftp_user,_facArg->ftp_passwd,_facArg->ftp_dest_path);
		string upload_log = "ftp ip:\t\t" + (string)_facArg->ftp_ip + "\n";
		upload_log += "ftp user:\t\t" + (string)_facArg->ftp_user + "\n";
		upload_log += "ftp passwd:\t\t" + (string)_facArg->ftp_passwd + "\n";
		upload_log += "ftp path:\t\t" + (string)_facArg->ftp_dest_path + "\n";
		update_screen_log(upload_log);

		//_uiHandle->confirm_test_result_waiting("upload log ...");
        char* response = ftp_send_file(MES_FILE,_facArg);
        response = response_to_chinese(response);
        LOG_INFO("upload %s",response);
		if (!strcmp(response,"上传成功")) {
			_uiHandle->confirm_test_result_success("upload success");
			set_test_result(UPLOAD_LOG_NAME,"PASS",response);
		} else {
			_uiHandle->confirm_test_result_warning("upload fail");
			set_test_result(UPLOAD_LOG_NAME,"FAIL",response);
		}
    } else {
        LOG_INFO("combine mes fail");
		
    }
    _testStep = STEP_IDLE;
}

void Control::update_screen_log(string uiLog)
{
    _uiHandle->update_screen_log(uiLog);
}

void Control::set_func_test_result(string func,string result)

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

void Control::auto_start_stress_test()
{
    if (check_file_exit(STRESS_LOCK_FILE.c_str())) {
		_lock_file_status = true;
        LOG_INFO("auto start stress test");
        _stress_test_stage = execute_command("cat " + STRESS_LOCK_FILE);
        LOG_INFO("stress stage:%s",_stress_test_stage.c_str());
        _funcBase[STRESS]->start_test(_baseInfo);
    } else {
    	_lock_file_status = false;
	}
}

void Control::set_interface_select_status(string func, bool state) {
	_funcFinishStatus->interface_finish = false;
    if (func == MEM_TEST_NAME) {
        _interfaceSelectStatus->mem_select  = state;
        if (_interfaceSelectStatus->mem_select) {
            _funcFinishStatus->mem_finish = false;
        } else {
            _funcFinishStatus->mem_finish = true;
			_interfaceTestStatus->mem_test_over = true;
        }
    }
    if (func ==  USB_TEST_NAME) {
        _interfaceSelectStatus->usb_select  = state;
        if (_interfaceSelectStatus->usb_select) {
            _funcFinishStatus->usb_finish = false;
        } else {
            _funcFinishStatus->usb_finish = true;
			_interfaceTestStatus->usb_test_over = true;
        }
    }
    if (func == NET_TEST_NAME) {
        _interfaceSelectStatus->net_select  = state;
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
        _interfaceSelectStatus->cpu_select  = state;
        if (_interfaceSelectStatus->cpu_select) {
            _funcFinishStatus->cpu_finish = false;
        } else {
            _funcFinishStatus->cpu_finish = true;
			_interfaceTestStatus->cpu_test_over = true;
        }
    }
    if (func == HDD_TEST_NAME) {
        _interfaceSelectStatus->hdd_select  = state;
        if (_interfaceSelectStatus->hdd_select) {
            _funcFinishStatus->hdd_finish = false;
        } else {
            _funcFinishStatus->hdd_finish = true;
			_interfaceTestStatus->hdd_test_over = true;
        }
    }
    if (func == FAN_TEST_NAME) {
        _interfaceSelectStatus->fan_select  = state;
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
}

void Control::set_interface_test_status(string func, bool status){
	if (func == MEM_TEST_NAME) {
		_interfaceTestStatus->mem_test_over= status;
	}
	if (func == USB_TEST_NAME) {
		_interfaceTestStatus->usb_test_over= status;
	}
	if (func == CPU_TEST_NAME) {
		_interfaceTestStatus->cpu_test_over= status;
	}
	if (func == NET_TEST_NAME) {
		_interfaceTestStatus->net_test_over= status;
	}
	if (func == EDID_TEST_NAME) {
		_interfaceTestStatus->edid_test_over= status;
	}
	if (func == HDD_TEST_NAME) {
		_interfaceTestStatus->hdd_test_over= status;
	}
	if (func == FAN_TEST_NAME) {
		_interfaceTestStatus->fan_test_over= status;
	}
	if (func == WIFI_TEST_NAME) {
		_interfaceTestStatus->wifi_test_over= status;
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
}

void Control::set_interface_test_result(string func, bool status) {
	if (func == MEM_TEST_NAME) {
        _interfaceTestResult->mem_test_result= status;
	}
	if (func == USB_TEST_NAME) {
        _interfaceTestResult->usb_test_result= status;
	}
	if (func == CPU_TEST_NAME) {
        _interfaceTestResult->cpu_test_result= status;
	}
	if (func == NET_TEST_NAME) {
        _interfaceTestResult->net_test_result= status;
	}
	if (func == EDID_TEST_NAME) {
        _interfaceTestResult->edid_test_result= status;
	}
	if (func == HDD_TEST_NAME) {
        _interfaceTestResult->hdd_test_result= status;
	}
	if (func == FAN_TEST_NAME) {
        _interfaceTestResult->fan_test_result= status;
	}
	if (func == WIFI_TEST_NAME) {
        _interfaceTestResult->wifi_test_result= status;
	}
}


void Control::set_test_result_pass_or_fail(string func, string result)
{
    if (result == "PASS") {
        if (func == SOUND_TEST_NAME) {
            _funcFinishStatus->sound_finish= true;
			update_mes_log("AUDIO", "PASS");
        }
        if (func == DISPLAY_TEST_NAME) {
            _funcFinishStatus->display_finish= true;
			update_mes_log("DISPLAY", "PASS");
        }
        if (func == BRIGHT_TEST_NAME) {
            _funcFinishStatus->bright_finish= true;
			update_mes_log("BRIGHTNESS", "PASS");
        }
        if (func == CAMERA_TEST_NAME) {
            _funcFinishStatus->camera_finish= true;
			CameraTest* camera = (CameraTest*)_funcBase[CAMERA];
			camera->close_xawtv_window();
			update_mes_log("CAMERA", "PASS");
        }
        if (func == STRESS_TEST_NAME) {
            _funcFinishStatus->stress_finish= true;
			update_mes_log("STRESS", "PASS");
        }
    } else {

        if (func == SOUND_TEST_NAME) {
            _funcFinishStatus->sound_finish= false;
			update_mes_log("AUDIO", "FAIL");
        }
        if (func == DISPLAY_TEST_NAME) {
            _funcFinishStatus->display_finish= false;
			update_mes_log("DISPLAY", "FAIL");
        }
        if (func == BRIGHT_TEST_NAME) {
            _funcFinishStatus->bright_finish= false;
			update_mes_log("BRIGHTNESS", "FAIL");
        }
        if (func == CAMERA_TEST_NAME) {
            _funcFinishStatus->camera_finish= false;
			CameraTest* camera = (CameraTest*)_funcBase[CAMERA];
			camera->close_xawtv_window();
			update_mes_log("CAMERA", "FAIL");
        }
        if (func == STRESS_TEST_NAME) {
            _funcFinishStatus->stress_finish= false;
			update_mes_log("STRESS", "FAIL");
        }
    }
    _uiHandle->set_test_result(func, result);
}

void Control::set_sn_mac_test_result(string sn_mac, string result)
{
	if (sn_mac == "MAC" && result == "PASS" && _whole_test_state) {
		sleep(1);
		_sn_mac = "SN";
		_uiHandle->show_sn_mac_message_box("SN");
	}
}

bool Control::is_stress_test_window_quit_safely()
{
    return _stress_test_window_quit_status;
}

