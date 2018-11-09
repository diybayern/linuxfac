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
#include "BrightTest.h"
#include "CameraTest.h"

#include "FuncBase.h"
#include "UiHandle.h"


class UsbTest;
class Control : public QObject
{
    Q_OBJECT
public:
    //explicit Control(QObject *parent = 0);
    Control();
    ~Control();
    static Control* _control;
    void set_test_result(string func, string result, string ui_log);
    void confirm_test_result(string func);
    void set_brightness_dialog_button_state(bool state);
    static Control* get_control();
    void show_main_test_ui();
    void auto_test_mac_or_stress();
    void update_screen_log(string uiLog);
    void update_color_screen_log(string uiLog, string color);
    void set_func_test_result(string func, string result);
    void upload_mes_log();
    void init_mes_log();
    void update_mes_log(string tag, string value);
    int get_screen_height();
    int get_screen_width();
    bool is_stress_test_window_quit_safely();
    int get_test_step();
    void factory_delete_event();
    static void* update_mes_log_thread(void* arg);
    void start_update_mes_log(MesInfo* info);
    bool get_decode_status();
    void show_stress_record();
    void print_stress_test_result(vector<string> record);

    UiHandle* get_ui_handle() {
        return _uiHandle;
    }
    

    BaseInfo* get_base_info()
    {
        return _baseInfo;
    }

    HwInfo* get_hw_info()
    {
        return _hwInfo;
    }

    void set_interface_test_status(string func, bool status);
    void set_interface_test_finish(string func);
    void set_interface_test_result(string func, bool status);

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

    int get_interface_run_status() {
        return _interfaceRunStatus;
    }

    void set_interface_run_status(int status) {
        _interfaceRunStatus = status;
    }

    void init_func_test();

    int get_fac_config_status() {
        return _fac_config_status;
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

    bool get_third_product_state() {
        return _is_third_product;
    }

    string get_third_net_info();
    
private:
    void init_base_info();
    void init_hw_info();
    void init_fac_config();
    void init_ui();

private:

    UiHandle* _uiHandle;
    BaseInfo* _baseInfo;
    HwInfo* _hwInfo;
    MesInfo* _mesInfo;
    
    int _testStep;
    FuncBase* _funcBase[FUNC_TYPE_NUM];
    FacArg* _facArg;
    int _interfaceRunStatus;
    string _display_sn_or_mac;
    int _fac_config_status;
    bool _stress_test_window_quit_status;
    bool _pcba_whole_lock_state;
    vector<string> _record;
    bool _is_third_product;

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
