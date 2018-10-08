
#ifndef _WIFI_TEST_H_
#define _WIFI_TEST_H_

#include "Control.h"
#include "FuncBase.h"

#define WLAN_NAME_LEN  (16)
#define TOTAL_SEND_NUM (100)
#define RECEIVE_NUM    (70)
#define INTERFACE_NUM  (512)

struct WifiInfo {
    unsigned char mac[MAC_ADDR_LEN];
    unsigned char wlan_name[WLAN_NAME_LEN];
    unsigned int wlan_index;
    unsigned int wlan_status;
    unsigned int wlan_link;
    unsigned int wlan_speed;
    unsigned char wlan_duplex;
    unsigned int seq;
    unsigned int recv_num;
};

class Control;

class WifiTest : public FuncBase
{
public:
    WifiTest();
    static bool init();
    static void* test_all(void*);
    void start_test(BaseInfo* baseInfo);

private:
    Control* _control;
    static bool wifi_get_wlan_name(char* wlan_name, int size);
    static bool wifi_get_wlan_index(char* wlan_name, unsigned int* index);
    static bool wifi_get_mac_addr(unsigned char* wlan_name, unsigned char* hw_buf);
    static bool wifi_sprintf_mac_addr(unsigned char* src, char* dst);
    static void* wifi_recv_loopback_msg(void *arg);
    static bool wifi_send_msg(char* src_mac, char* dst_mac, unsigned int index, unsigned int seq);
    static bool wifi_test_send_msg();
    static bool wifi_send_broadcast_msg(WifiInfo* info, int num);
    static bool check_if_wifi_connect_pass(void);

    
};


#endif



