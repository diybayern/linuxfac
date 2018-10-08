#include "../../inc/Control.h"


Control::Control():QObject()
{
    //_memTest = new MemTest(this);
    _funcBase[MEM] = new MemTest(this);
    uihandle = new UiHandle();
    uihandle->add_main_label("产品型号:", "Rain410");
    uihandle->add_main_label("硬件版本:", "V1.00");
    uihandle->add_main_label("SN序列号:", "12345678");
    uihandle->add_main_label("MAC地址:", "00:11:22:33:44:55:66");
    uihandle->add_main_label("CPU型号:", "i3-6100U");

    uihandle->add_main_test_button("接口测试");
    uihandle->add_interface_test_button("内存测试");
    uihandle->add_interface_test_button("USB测试");
    uihandle->add_interface_test_button("网口测试");
    uihandle->add_interface_test_button("EDID测试");
    uihandle->add_interface_test_button("CPU测试");
    uihandle->add_interface_test_button("HDD测试");
    uihandle->add_interface_test_button("FAN测试");
    uihandle->add_interface_test_button("WIFI测试");

    uihandle->add_main_test_button("音频测试");
    uihandle->add_main_test_button("显示测试");
    uihandle->add_main_test_button("亮度测试");
    uihandle->add_main_test_button("摄像头测试");
    uihandle->add_main_test_button("拷机测试");
    uihandle->add_main_test_button("上传日志");
    uihandle->add_main_test_button("下道工序");

    uihandle->add_complete_or_single_test_label("整机测试");
    uihandle->sync_main_test_ui();
    connect(uihandle->get_qobject("内存测试"), SIGNAL(clicked()), this, SLOT(startMemTest()));

}


Control* Control::_control = NULL;
Control* Control::get_control()
{
    if (!_control) {
        _control = new Control();
    }
    return _control;
}

void Control::startMemTest()
{
    _funcBase[MEM]->startTest(10);
    cout << "2" << endl;
}

void Control::setTestResult(string func,string result,string ui_log)

{
    cout << "func:" << func << endl;
    cout << "result:" << result << endl;
    cout << "ui_log:" << ui_log << endl;
    uihandle->set_test_result(func, result, ui_log);
}

void Control::showMainTestUi()
{
    uihandle->to_show_main_test_ui();
}
