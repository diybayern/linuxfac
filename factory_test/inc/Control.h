#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>
#include "fac_utils.h"
#include "MemTest.h"
#include "FuncTest.h"
#include "SoundTest.h"
#include "NetTest.h"
#include "EdidTest.h"
#include "WifiTest.h"
#include "HddTest.h"
#include "BrightTest.h"
#include "CameraTest.h"

#include "FuncBase.h"
#include "UiHandle.h"

#define   FUNC_TYPE_NUM     (15)

enum FuncType
{
    INTERFACE = 0,
    MEM,
    USB,
    NET,
    EDID,
    CPU,
    HDD,
    FAN,
    WIFI,
    SOUND,
    BRIGHT,
    CAMERA,
    STRESS,
    NEXT_PROCESS,
    UPLOAD_MES_LOG,
};

enum InterfaceRunStatus
{
    INF_RUNEND = 0,
    INF_BREAK,
    INF_RUNNING
};

enum TestStep {
    STEP_IDLE = 0,
    STEP_INTERFACE,
    STEP_SOUND,
    STEP_DISPLAY,
    STEP_BRIGHTNESS,
    STEP_CAMERA,
    STEP_STRESS,
    STEP_MEM,
    STEP_FAN
};

struct FuncFinishStatus {
    bool interface_finish;
    bool mem_finish;
    bool usb_finish;
    bool cpu_finish;
    bool net_finish;
    bool edid_finish;
    bool hdd_finish;
    bool fan_finish;
    bool wifi_finish;
    bool sound_finish;
    bool display_finish;
    bool bright_finish;
    bool camera_finish;
    bool stress_finish;
};

struct InterfaceTestStatus {
    bool mem_test_over;
    bool usb_test_over;
    bool cpu_test_over;
    bool net_test_over;
    bool edid_test_over;
    bool hdd_test_over;
    bool fan_test_over;
    bool wifi_test_over;
};

struct InterfaceTestResult {
    bool mem_test_result;
    bool usb_test_result;
    bool cpu_test_result;
    bool net_test_result;
    bool edid_test_result;
    bool hdd_test_result;
    bool fan_test_result;
    bool wifi_test_result;
};

struct InterfaceTestFailNum {
    int mem_test_fail_num;
    int usb_test_fail_num;
    int cpu_test_fail_num;
    int net_test_fail_num;
    int edid_test_fail_num;
    int hdd_test_fail_num;
    int fan_test_fail_num;
    int wifi_test_fail_num;
};




struct InterfaceSelectStatus {
    bool mem_select;
    bool usb_select;
    bool cpu_select;
    bool net_select;
    bool edid_select;
    bool hdd_select;
    bool fan_select;
    bool wifi_select;
};

struct MesInfo {
    MesInfo():func(""),
        status("")
    {
    }

    string func;
    string status;
};

class UsbTest;
class Control : public QObject
{
    Q_OBJECT
public:
    //explicit Control(QObject *parent = 0);
    Control();
    static Control* _control;
    void set_test_result(string func,string result,string ui_log);
    void confirm_test_result(string func);
    void set_brightness_dialog_button_state(bool state);
    static Control* get_control();
    void show_main_test_ui();
    void auto_test_mac_sn();
    void update_screen_log(string uiLog);
    void update_color_screen_log(string uiLog, string color);
    void set_func_test_result(string func,string result);
    void upload_mes_log();
    void init_mes_log();
    void update_mes_log(string tag,string value);
    int get_screen_height();
    int get_screen_width();
    bool is_stress_test_window_quit_safely();
    int get_test_step();
    void factory_delete_event();
    static void* update_mes_log_thread(void* arg);
    void start_update_mes_log(MesInfo* info);
    int get_decode_status();
    void show_stress_record();
    void print_stress_test_result(vector<string> record);

    UiHandle* get_ui_handle() {
        return _uiHandle;
    }
    
    FuncFinishStatus* get_func_finish_status()
    {
        return _funcFinishStatus;
    }

    BaseInfo* get_base_info()
    {
        return _baseInfo;
    }

    HwInfo* get_hw_info()
    {
        return _hwInfo;
    }

    void set_interfacetest_finish()
    {
        _funcFinishStatus->interface_finish = true;
    }


    void set_interface_test_status(string func, bool status);
    void set_interface_test_finish(string func);
    void set_interface_test_result(string func, bool status);


    string get_stress_test_stage() {
        return _stress_test_stage;
    }

    bool get_auto_upload_log_status() {
        return _autoUploadLog;
    }

    int get_interface_test_times() {
        string times = _uiHandle->get_test_count();
        return get_int_value(times);
    }

    bool get_auto_upload_mes_status() {
        return _uiHandle->get_auto_upload_check_state();
    }

    void set_test_step(int step) {
        _testStep = step;
    }

    FuncBase** get_funcbase() {
        return _funcBase;
    }

    InterfaceSelectStatus* get_interface_select_status() {
        return _interfaceSelectStatus;
    }

    InterfaceTestStatus* get_interface_test_status() {
        return _interfaceTestStatus;
    }

    InterfaceTestResult* get_interface_test_result() {
        return _interfaceTestResult;
    }

    int get_interface_run_status() {
        return _interfaceRunStatus;
    }

    void set_interface_run_status(int status) {
        _interfaceRunStatus = status;
    }

    void init_func_test();

    int get_fac_config_status(){
        return _fac_config_status;
    }
    
    bool get_whole_test_state(){
        return _whole_test_state;
    }
    
    void set_stress_test_window_quit_status(bool status) {
        _stress_test_window_quit_status = status;
    }

    void set_pcba_whole_lock_state(bool state) {
        _pcba_whole_lock_state = state;
    }

    bool get_pcba_whole_lock_state() {
        return _pcba_whole_lock_state;
    }

    FacArg* get_fac_arg() {
        return _facArg;
    }
    
private:
    void init_base_info();
    void init_hw_info();
    
    void init_fac_config();
    void auto_start_stress_test();

    void ui_init();

private:

    UiHandle* _uiHandle;
    BaseInfo* _baseInfo;
    HwInfo* _hwInfo;
    MesInfo* _mesInfo;
    FuncFinishStatus* _funcFinishStatus;
    InterfaceTestStatus* _interfaceTestStatus;
    InterfaceSelectStatus* _interfaceSelectStatus;
    InterfaceTestResult* _interfaceTestResult;
    InterfaceTestFailNum* _interfaceTestFailNum;
    
    int _testStep;
    FuncBase* _funcBase[FUNC_TYPE_NUM];
    FacArg* _facArg;
    string _stress_test_stage;
    bool _autoUploadLog;
    string _mes_log_file;
    bool _auto_upload_mes;
    int _interfaceRunStatus;
    string _sn_mac;
    int _fac_config_status;
    bool _whole_test_state;
    bool _is_idv;
    bool _stress_test_window_quit_status;
    bool _pcba_whole_lock_state;
    bool _lock_file_status;
    vector<string> _record;

signals:

public slots:
    void start_interface_test();
    void start_sound_test();
    void start_display_test();
    void start_bright_test();
    void start_camera_test();
    void start_stress_test();
    void start_upload_log();
    void start_next_process();
    void show_test_confirm_dialog(string item);
    void set_interface_select_status(string func, bool state);
    void set_test_result_pass_or_fail(string func, string result);
    void set_sn_mac_test_result(string sn_mac, string result);
    void check_sn_mac_compare_result(string message);
    void confirm_shut_down_or_next_process(string process);
    void retry_sn_mac_test();
    void slot_factory_delete_event();
};

#endif // CONTROL_H
