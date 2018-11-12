
#ifndef _NET_TEST_H_
#define _NET_TEST_H_

#include "Control.h"
#include "FuncBase.h"

class NetTest : public FuncBase
{
public:
    ~NetTest();
    static WebInfo* g_net_info;
    bool init();
    static bool net_test_all(bool test_flag);
    static void* test_all(void*);
    void start_test(BaseInfo* baseInfo);
    static string net_get_duplex_desc(char duplex);

private:
    static string screen_log_black;
    static string screen_log_red;
    bool net_get_eth_name(char* eth_name, int size);
    bool net_get_eth_index(char* eth_name, unsigned int* index);
    bool net_get_mac_addr0(unsigned char* eth_name, unsigned char* hw_buf);
    static bool net_sprintf_mac_addr(unsigned char* src, char* dst);
    static void* net_recv_loopback_msg(void *arg);
    static bool net_get_eth_info(WebInfo *info);
    static int net_eth_no(char *eth_name);
    static bool net_get_eth_status(int fd, char *eth_name, unsigned int *status);
    static int net_test_ioctl(int fd, char *eth_name, void *cmd);
    static bool net_send_broadcast_msg(WebInfo* info, int num);
    static bool net_send_msg(char* src_mac, char* dst_mac, unsigned int index, unsigned int seq);
};


#endif



