
#ifndef _NET_TEST_H_
#define _NET_TEST_H_

#include "Control.h"
#include "FuncBase.h"

#define ETH_NAME_LEN     (16)
#define ETH_STATUS_UP    (0)
#define ETH_STATUS_DOWN  (1)

struct NetInfo {
	unsigned char mac[MAC_ADDR_LEN];
	unsigned char eth_name[ETH_NAME_LEN];
	unsigned int eth_index;
	unsigned int eth_status;
	unsigned int eth_link;
    unsigned int eth_speed;
    unsigned char eth_duplex;
	unsigned int seq;
	unsigned int recv_num;
};

class Control;

class NetTest : public FuncBase
{
public:
    NetTest();
    static bool init();
    static bool net_test_all();
    static void* test_all(void*);
    void start_test(BaseInfo* baseInfo);

private:
    Control* _control;

    static bool net_get_eth_name(char* eth_name, int size);
    static bool net_get_eth_index(char* eth_name, unsigned int* index);
    static bool net_get_mac_addr0(unsigned char* eth_name, unsigned char* hw_buf);
    static bool net_sprintf_mac_addr(unsigned char* src, char* dst);
    static void* net_recv_loopback_msg(void *arg);
    static bool net_get_eth_info(NetInfo *info);
    static int net_eth_no(char *eth_name);
    static bool net_get_eth_status(int fd, char *eth_name, unsigned int *status);
    static int net_test_ioctl(int fd, char *eth_name, void *cmd);
    static string net_get_duplex_desc(char duplex);
    static bool net_send_broadcast_msg(NetInfo* info, int num);
    static bool net_send_msg(char* src_mac, char* dst_mac, unsigned int index, unsigned int seq);
};


#endif



