#ifndef UIHANDLE_H
#define UIHANDLE_H
#include "ui.h"
#include <iostream>
using namespace std;

class UiHandle: public QObject
{
    Q_OBJECT

    public:
        UiHandle();
        ~UiHandle();
        static UiHandle* get_uihandle();

    public:
        void add_interface_test_button(string item);
        void add_main_label(string item, string result);
        void add_main_test_button(string item);
        void add_stress_test_label(string item);
        void confirm_test_result_dialog(string title);
        void confirm_test_result_warning(string title);
        void confirm_test_result_success(string title);
        void confirm_test_result_waiting(string title);
        void set_test_result(string item, string result);
        void set_stress_test_pass_or_fail(string result);
        void show_display_ui();
        void show_stress_test_ui();
        void update_screen_log(string textInfo);
        void update_stress_label_value(string item, string result);
        void sync_main_test_ui();
        void to_show_main_test_ui();
        void add_complete_or_single_test_label(string config);
        void show_sn_mac_message_box(string sn_mac);
        void start_audio_progress_dialog();
        void ui_set_interface_test_state(int state);
        void show_sn_mac_comparison_result(string sn_mac, string result);
        void update_sn_mac_test_result(string sn_mac, string result);
        bool get_is_complete_test() {return _is_complete_test;}
        void set_is_complete_test(bool category) {_is_complete_test = category;}
        int get_screen_width();
        int get_screen_height();
        bool get_auto_upload_check_state();
        string get_test_count();
        QObject *get_qobject(string name);

    private:
        static UiHandle* _ui_handle;
        bool _is_complete_test;

    signals:
        void to_draw_main_test_window();
        void need_to_show_main_test();
        void print_result(QString func, QString result);
        void to_update_stress_label_value(QString item, QString result);
        void to_show_stress_test_window();
        void to_show_display_test_window();
        void to_show_sn_mac_message_box(QString sn_mac);
        void need_to_update_screen_log(QString info);
        void to_confirm_test_result_dialog(QString title);
        void to_start_audio_progress_dialog();
        void to_confirm_test_result_warning(QString title);
        void to_confirm_test_result_success(QString title);
        void to_confirm_test_result_waiting(QString title);
        void to_show_test_confirm_dialog(string item);
        void sig_ui_handled_test_result(string test_item, string result);
        void sig_ui_handled_sn_mac_test_result(string sn_mac, string result);
        void sig_ui_check_state_changed(string item, bool state);
        void sig_set_interface_test_state(int state);
        void sig_show_sn_mac_comparison_result(QString sn_mac, QString result);
        void sig_ui_get_message_from_scangun(string message);
        void sig_ui_confirm_shut_down_or_next_process(string process);
        void to_update_stress_test_pass_or_fail(QString result);
        void sig_ui_retry_sn_mac();
        void to_update_sn_mac_state(QString sn_mac, QString result);

    public slots:
        void quit_test_window(QString item);
        void slot_handled_test_result(QString test_item, QString result);
        void slot_check_state_changed(QString item, bool state);
        void slot_recv_sn_mac_test_result(QString sn_mac, QString result);
        void slot_get_message_from_scangun(QString message);
        void slot_confirm_shut_down_or_next_process(QString process);
        void slot_retry_sn_mac();
};


#endif//UIHANDLE_H
