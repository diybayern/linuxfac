
#ifndef _WIFI_TEST_H_
#define _WIFI_TEST_H_

#include "Control.h"
#include "FuncBase.h"

class WifiTest : public FuncBase
{
public:
    ~WifiTest();
    bool init();
    static void* test_all(void*);
    void start_test(BaseInfo* baseInfo);

private:
    static WebInfo* g_wifi_info;
    static string screen_log_black;
    static string screen_log_red;
    bool wifi_get_wlan_name(char* wlan_name, int size);
    bool wifi_get_wlan_index(char* wlan_name, unsigned int* index);
    bool wifi_get_mac_addr(unsigned char* wlan_name, unsigned char* hw_buf);
    static bool wifi_sprintf_mac_addr(unsigned char* src, char* dst);
    static void* wifi_recv_loopback_msg(void *arg);
    static bool wifi_send_msg(char* src_mac, char* dst_mac, unsigned int index, unsigned int seq);
    static bool wifi_test_send_msg();
    static bool wifi_send_broadcast_msg(WebInfo* info, int num);
    static bool check_if_wifi_connect_pass(void);

    
};


#endif



