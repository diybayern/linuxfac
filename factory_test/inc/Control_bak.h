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
    UPLOAD_LOG,
    NEXT_PROCESS
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

class UsbTest;
class Control : public QObject
{
    Q_OBJECT
public:
    //explicit Control(QObject *parent = 0);
    Control();
    void set_test_result(string func,string result,string ui_log);
    void confirm_test_result(string func);
    static Control* get_control();
    void show_main_test_ui();
    void update_screen_log(string uiLog);
    void set_func_test_result(string func,string result);
    void upload_mes_log();
    void init_mes_log();
    void update_mes_log(char* tag,char* value);
    int get_screen_height();
    int get_screen_width();
    bool is_stress_test_window_quit_safely();
    bool stress_test_window_quit_status;

    int get_test_step();
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

    void set_mem_test_finish()
    {
        _funcFinishStatus->mem_finish = true;
    }

    void set_usb_test_finish()
    {
        _funcFinishStatus->usb_finish = true;
    }

    void set_cpu_test_finish()
    {
        _funcFinishStatus->cpu_finish = true;
    }

    void set_edid_test_finish()
    {
        _funcFinishStatus->edid_finish = true;
    }

    void set_net_test_finish()
    {
        _funcFinishStatus->net_finish = true;
    }

    void set_hdd_test_finish()
    {
        _funcFinishStatus->hdd_finish = true;
    }

    void set_fan_test_finish()
    {
        _funcFinishStatus->fan_finish = true;
    }

    void set_wifi_test_finish()
    {
        _funcFinishStatus->wifi_finish = true;
    }

    void set_mem_test_status(bool status)
    {
        _interfaceTestStatus->mem_test_over= status;
    }
    
    void set_usb_test_status(bool status)
    {
        _interfaceTestStatus->usb_test_over= status;
    }
    
    void set_net_test_status(bool status)
    {
        _interfaceTestStatus->net_test_over= status;
    }
    
    void set_edid_test_status(bool status)
    {
        _interfaceTestStatus->edid_test_over= status;
    }
    
    void set_cpu_test_status(bool status)
    {
        _interfaceTestStatus->cpu_test_over= status;
    }
    
    void set_hdd_test_status(bool status)
    {
        _interfaceTestStatus->hdd_test_over= status;
    }
    void set_fan_test_status(bool status)
    {
        _interfaceTestStatus->fan_test_over= status;
    }
    
    void set_wifi_test_status(bool status)
    {
        _interfaceTestStatus->wifi_test_over= status;
    }

    void set_mem_test_result(bool status)
    {
        _interfaceTestResult->mem_test_result= status;
    }
    
    void set_usb_test_result(bool status)
    {
        _interfaceTestResult->usb_test_result = status;
    }
    
    void set_net_test_result(bool status)
    {
        _interfaceTestResult->net_test_result= status;
    }
    
    void set_edid_test_result(bool status)
    {
        _interfaceTestResult->edid_test_result= status;
    }
    
    void set_cpu_test_result(bool status)
    {
        _interfaceTestResult->cpu_test_result= status;
    }
    
    void set_hdd_test_result(bool status)
    {
        _interfaceTestResult->hdd_test_result= status;
    }
    void set_fan_test_result(bool status)
    {
        _interfaceTestResult->fan_test_result= status;
    }
    
    void set_wifi_test_result(bool status)
    {
        _interfaceTestResult->wifi_test_result= status;
    }
    

    string get_stress_test_stage() {
        return _stress_test_stage;
    }

    bool get_auto_upload_log_status() {
        return _autoUploadLog;
    }

    int get_interface_test_times() {
        return _interface_test_times;
    }

    void set_interface_test_times(string time) {
        _interface_test_times = get_int_value(time);
    }

    bool get_auto_upload_mes_status() {
        return _auto_upload_mes;
    }

    void set_auto_upload_mes_status(bool status) {
        _auto_upload_mes = status;
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
    
private:
    void init_base_info();
    void init_hw_info();
    void init_func_test();
    void init_fac_config();
    void auto_start_stress_test();

    void ui_init();

private:
    static Control* _control;
    UiHandle* _uiHandle;
    BaseInfo* _baseInfo;
    HwInfo* _hwInfo;
    FuncFinishStatus* _funcFinishStatus;
    InterfaceTestStatus* _interfaceTestStatus;
    InterfaceSelectStatus* _interfaceSelectStatus;
    InterfaceTestResult* _interfaceTestResult;
    InterfaceTestFailNum* _interfaceTestFailNum;
	UsbTest* _usb;
    
    int _testStep;
    FuncBase* _funcBase[FUNC_TYPE_NUM];
    FacArg* _facArg;
    string _stress_test_stage;
    bool _autoUploadLog;
    string _mes_log_file;
    int _interface_test_times;
    bool _auto_upload_mes;
    int _interfaceRunStatus;

signals:

public slots:
    void start_interface_test();
    void start_mem_test();
    void start_usb_test();
    void start_net_test();
    void start_edid_test();
    void start_cpu_test();
    void start_hdd_test();
    void start_fan_test();
    void start_wifi_test();
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
};

#endif // CONTROL_H
