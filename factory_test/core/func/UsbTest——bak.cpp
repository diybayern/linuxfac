#include "../../inc/FuncTest.h"
#include "../../inc/fac_log.h"

UsbTest::UsbTest(Control* control)
       :_control(control)
{
    
}

bool UsbTest::usb_num_test(string total_num, string num_3)
{
    string real_total_num = execute_command("lsusb -t | grep \"Mass Storage\" | wc -l");
    string real_num_3 = execute_command("lsusb -t | grep \"Mass Storage\" | grep \"5000M\" | wc -l");
    if (real_total_num == total_num) {
        if (real_num_3 == num_3) {
            return true;
        } else {
            LOG_INFO("usb3.0 num is %s,which need %s!",real_num_3.c_str(),num_3.c_str());
            return false;
        }
    } else {
        LOG_INFO("usb num is %s,which need %s!",real_total_num.c_str(),total_num.c_str());
        return false;
    }
}

void UsbTest::set_usb_test_result(string func,string result,string ui_log)
{
    Control *control = Control::get_control();
    control->set_test_result(func,result,ui_log);
}

void* UsbTest::test_all(void *arg)
{
	BaseInfo* baseInfo = (BaseInfo *)arg;
	bool result = usb_num_test(baseInfo->usb_total_num,baseInfo->usb_3_num);

	if (result) {
        set_usb_test_result("USB≤‚ ‘","PASS","aa");
	} else {
        set_usb_test_result("USB≤‚ ‘","FAIL","ff");
	}
	return NULL;
}

void UsbTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}