
#include <ctype.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>

#include "NetTest.h"

WebInfo* NetTest::g_net_info = new WebInfo();
string NetTest::screen_log_black = "";
string NetTest::screen_log_red = "";

NetTest::~NetTest()
{
    LOG_INFO("~NetTest()");
    if (g_net_info != NULL) {
        delete g_net_info;
        g_net_info = NULL;
    }
}

bool NetTest::net_get_eth_name(char* eth_name, int size)
{
    if (eth_name == NULL || size <= 0) {
        LOG_ERROR("eth name or size is NULL");
        return false;
    }

    int i = 0;
    int num = 0;
    int fd = -1;
    int ret = 0;
    struct ifconf ifconf;
    char buf[512] = { 0, };
    struct ifreq *ifreq = NULL;    

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_ERROR("create socket failed\n");
        return false;
    }

    ifconf.ifc_len = 512;
    ifconf.ifc_buf = (char*)buf;

    ret = ioctl(fd, SIOCGIFCONF, &ifconf);
    if (ret < 0) {
        LOG_ERROR("get net config failed\n");
        close(fd);
        return false;
    }

    ifreq = (struct ifreq*) buf;
    num = ifconf.ifc_len / sizeof(struct ifreq);

    for (i = 0; i < num; i++) {
        LOG_INFO("net card name %s\n", ifreq->ifr_name);
        if (strncmp("eth", ifreq->ifr_name, 3) == 0 || strncmp("br", ifreq->ifr_name, 2) == 0) {
            strncpy(eth_name, ifreq->ifr_name, size);
        }

        ifreq++;
    }

    close(fd);

    return true;
}

bool NetTest::net_get_eth_index(char* eth_name, unsigned int* index)
{
    if (eth_name == NULL || index == NULL) {
        LOG_ERROR("eth name or index is null");
        return false;
    }

    int fd = -1;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_ERROR("create socket failed\n");
        return false;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, eth_name, WEB_NAME_LEN);

    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        LOG_ERROR("get eth index failed\n");
        close(fd);
        return false;
    }

    *index = ifr.ifr_ifindex;
    close(fd);
    LOG_INFO("eth %s index: %u\n", eth_name, *index);

    return true;
}

bool NetTest::net_sprintf_mac_addr(unsigned char* src, char* dst)
{
    if (src == NULL || dst == NULL) {
        LOG_ERROR("src or dst is NULL");
        return false;
    }

    int ret = 0;
    ret = sprintf((char*)dst, "%02x:%02x:%02x:%02x:%02x:%02x", src[0], src[1], src[2], src[3], src[4], src[5]);
    if (ret < 0) {
        LOG_ERROR("sprintf mac addr failed");
        return false;
    }

    return true;
}

bool NetTest::net_get_mac_addr0(unsigned char* eth_name, unsigned char* hw_buf)
{
    if (eth_name == NULL || hw_buf == NULL) {
        LOG_ERROR("eth name or hw_buf is NULL");
        return false;
    }

    int fd = -1; 
    struct ifreq ifr;
    unsigned char buf[128] = { 0, };

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_ERROR("create socket failed\n");
        return false;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, (char*)eth_name, WEB_NAME_LEN);

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
        LOG_ERROR("get mac addr failed\n");
        close(fd);
        return false;
    }

    memcpy(hw_buf, ifr.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
    close(fd);

    net_sprintf_mac_addr(hw_buf, (char*)buf);
    LOG_INFO("get mac addr %s\n", buf);

    return true;
}

void* NetTest::net_recv_loopback_msg(void *arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is NULL");
        return NULL;
    }
    
    int fd = -1;
    int ret = 0;
    int len = 0;
    MacPacket recv_packet;
//    MacPacket send_packet;
    struct sockaddr_ll recv_sll;
//    struct sockaddr_ll send_sll;
    WebInfo* info = NULL;
    char buf[128] = { 0, };   //TODO: char[] buf
    unsigned char bc_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    pthread_detach(pthread_self());
    info = (WebInfo*)arg;

    start: fd = socket(PF_PACKET, SOCK_RAW, htons(TEST_PROTO));
    if (fd < 0) {
        LOG_ERROR("create raw sock failed\n");
        return NULL;
    }

    memset(&recv_sll, 0, sizeof(struct sockaddr_ll));
    recv_sll.sll_family = PF_PACKET;
    recv_sll.sll_ifindex = info->index;
    recv_sll.sll_protocol = htons(TEST_PROTO);

    ret = bind(fd, (struct sockaddr*)&recv_sll, sizeof(recv_sll));
    if (ret < 0) {
        LOG_ERROR("bind recv socket failed ret=%d errno=%d\n", ret, errno);
        close(fd);
        return NULL;
    }

    while (1) {
        len = recvfrom(fd, &recv_packet, sizeof(recv_packet), 0, 0, 0);
        if (len != sizeof(recv_packet)) {
            LOG_ERROR("recv len failed %d\n", len);
            close(fd);
            usleep(10000);
            goto start;
        }

        if (recv_packet.magic != TEST_MAGIC) {
            LOG_ERROR("recv magic=%d is invalid\n", recv_packet.magic);
            close(fd);
            goto start;
        }

        // exchange will transform message
        if (memcmp(recv_packet.dst_mac, bc_mac, MAC_ADDR_LEN) != 0) {
            net_sprintf_mac_addr(recv_packet.src_mac, (char*)buf);
            LOG_INFO("recv roll back msg index=%d, from mac=%s\n", recv_packet.index, buf);
            info->recv_num++;
        } else {
            // send back
            net_send_msg((char *)info->mac, (char *)recv_packet.src_mac, info->index, recv_packet.index);
            net_sprintf_mac_addr(recv_packet.src_mac, (char*)buf);
            LOG_ERROR("send back to %s index=%d\n", buf, recv_packet.index);
        }
    }

    return NULL;
}

bool NetTest::init()
{
    bool ret = false;
    pthread_t pid;
    WebInfo* info = NULL;   //TODO: WebInfo --> char[]

    info = g_net_info;
    if (info == NULL) {
        LOG_ERROR("new WebInfo failed\n");
        return false;
    }

    memset(info, 0, sizeof(WebInfo));

    ret = net_get_eth_name((char*)info->name, WEB_NAME_LEN);
    if (ret == false) {
        LOG_ERROR("get eth name failed\n");
        delete info;
        info = NULL;
        return false;
    }

    ret = net_get_eth_index((char*)info->name, &info->index);
    if (ret == false) {
        LOG_ERROR("get eth index failed\n");
        delete info;
        info = NULL;
        return false;
    }

    ret = net_get_mac_addr0(info->name, info->mac);
    if (ret == false) {
        LOG_ERROR("get mac addr failed\n");
        delete info;
        info = NULL;
        return false;
    }

    int tmp = pthread_create(&pid, NULL, net_recv_loopback_msg, info);
    if (tmp < 0) {
        LOG_ERROR("create thread failed\n");
        delete info;
        info = NULL;
        return false;
    }

    return true;
}

int NetTest::net_eth_no(char *eth_name)
{
    if (eth_name == NULL) {
        LOG_ERROR("eth name is NULL");
        return -1;
    }
    
    char *p = eth_name;

    while (*p) {
        if (is_digit(p)) {
            break;
        }
        p++;
    }
    return atoi(p);
}

bool NetTest::net_get_eth_status(int fd, char *eth_name, unsigned int *status)
{
    if (fd < 0 || eth_name == NULL || status == NULL) {
        LOG_ERROR("fd=%d, eth name or status is wrong", fd);
        return false;
    }
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, eth_name, WEB_NAME_LEN);
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        LOG_ERROR("get eth status failed: %s\n", strerror(errno));
        return false;
    }

    // ifr.ifr_flags & IFF_RUNNING  --->link
    if ((ifr.ifr_flags & IFF_UP)) {
        *status = ETH_STATUS_UP;
    } else {
        *status = ETH_STATUS_DOWN;
    }

    LOG_ERROR("cur eth up: %d\n", !*status);
    return true;
}

int NetTest::net_test_ioctl(int fd, char *eth_name, void *cmd)
{
    if (fd < 0 || eth_name == NULL || cmd == NULL) {
        LOG_ERROR("fd=%d, eth name or cmd is wrong", fd);
        return false;
    }

    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, eth_name, WEB_NAME_LEN);
    ifr.ifr_data = (char*)cmd;
    return ioctl(fd, SIOCETHTOOL, &ifr);
}

bool NetTest::net_get_eth_info(WebInfo *info)
{
    if (info == NULL) {
        LOG_ERROR("info is NULL");
        return false;
    }
    
    int fd, ret;
    struct ethtool_cmd ecmd;
    struct ethtool_value edata;
    char eth_name[WEB_NAME_LEN];

    if (info == NULL || info->name == NULL) {
        LOG_ERROR("pointer NULL error!\n");
        return false;
    }
    if (strncmp("br", (char*)info->name, 2) == 0) {
        snprintf(eth_name, WEB_NAME_LEN, "eth%d", net_eth_no((char *)info->name));
    } else {
        strcpy(eth_name, (char*)info->name);
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_ERROR("create socket failed: %s\n", strerror(errno));
        return false;
    }

    bool flag = net_get_eth_status(fd, (char *)eth_name, &info->status);
    if (flag == false) {
        close(fd);
        return false;
    }

    edata.cmd = ETHTOOL_GLINK;
    ret = net_test_ioctl(fd, (char*)eth_name, &edata);
    if (ret < 0) {
        LOG_ERROR("get eth link failed: %s\n", strerror(errno));
        close(fd);
        return false;
    }
    info->link = edata.data;

    /* Get seed & duplex */
    ecmd.cmd = ETHTOOL_GSET;
    ret = net_test_ioctl(fd, (char*)eth_name, &ecmd);
    if (ret < 0) {
        LOG_ERROR("get eth speed & duplex info failed: %s\n", strerror(errno));
        close(fd);
        return false;
    }
    info->speed = (ecmd.speed_hi << 16) | ecmd.speed;
    info->duplex = ecmd.duplex;
    LOG_INFO("ethernet card speed: %uMbps, duplex: %i\n", info->speed, info->duplex);

    close(fd);
    return true;
}

string NetTest::net_get_duplex_desc(char duplex)
{
    switch (duplex) {
    case DUPLEX_HALF:
        return "Half";
    case DUPLEX_FULL:
        return "Full";
    default:
        return "Unknown!";
    }
}

bool NetTest::net_send_msg(char* src_mac, char* dst_mac, unsigned int index, unsigned int seq)
{
    if (src_mac == NULL || dst_mac == NULL) {
        LOG_ERROR("src or dst mac is NULL");
        return false;
    }
    int fd = -1;
    int ret = 0;
    MacPacket packet;
    struct sockaddr_ll sll;

    fd = socket(PF_PACKET, SOCK_RAW, htons(TEST_PROTO));
    if (fd < 0) {
        LOG_ERROR("create raw sock failed\n");
        return false;
    }

    memset(&sll, 0, sizeof(struct sockaddr_ll));
    sll.sll_ifindex = index;
    memset(&packet, 0, sizeof(MacPacket));

    memcpy(&packet.dst_mac, dst_mac, MAC_ADDR_LEN);
    memcpy(&packet.src_mac, src_mac, MAC_ADDR_LEN);
    packet.type = TEST_PROTO;
    packet.magic = TEST_MAGIC;
    packet.index = seq;

    ret = sendto(fd, (void*)&packet, sizeof(packet), 0, (struct sockaddr*)&sll, sizeof(sll));
    if (ret < 0) {
        LOG_ERROR("send failed ret=%d errno=%d\n", ret, errno);
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

/* loop send msg */
bool NetTest::net_send_broadcast_msg(WebInfo* info, int num)
{
    if (info == NULL || num <=0) {
        LOG_ERROR("info is NULL or num is wrong");
        return false;
    }
    int i = 0;
    bool ret = true;
    unsigned char dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    for (i = 0; i < num; i++) {
        ret &= net_send_msg((char *)info->mac, (char *)dest_mac, info->index, info->seq++);
        usleep(100);
    }

    return ret;
}

bool NetTest::net_test_all(bool test_flag) //test_flag = false, is third product to get net info without send packet msg
{
    Control *control = Control::get_control();
    int i = 0;
    bool ret = 0;
    WebInfo *info = NULL;

    info = g_net_info;
    if (info == NULL) {
        LOG_ERROR("net info is null");
        screen_log_red += "\t错误：网口初始化错误，获取网口信息失败\n";
        screen_log_black += "ERROR:net init error! get net info failed\n";
        return false;    
    }
    if (test_flag) {
        LOG_INFO("net test start:\n");
        LOG_INFO("Network card name: \t\t\t%s \n", info->name);
        screen_log_black += "Network card name: \t\t";
        screen_log_black += (char*)info->name;
        screen_log_black += "\n";
    } else {
        LOG_INFO("third product get net info\n");
    }

    ret = net_get_eth_info(info);
    if (ret == false) {
        LOG_ERROR("get eth status failed!\n");
        screen_log_black += "ERROR: get eth status failed!\n";
        screen_log_red += "\t错误：网口状态获取失败\n";
        goto error;
    }

    if (info->status == ETH_STATUS_UP) {
        LOG_INFO("Network card status: \tup\n");
        screen_log_black += "Network card status: \t\tup\n";
    } else {
        LOG_ERROR("Network card status: \tdown\n");
        screen_log_black += "Network card status: \t\tdown\n\tERROR: network is down!\n";
        screen_log_red += "\t错误：网卡关闭\n";
        ret = false;
        goto error;
    }
    
    if (test_flag) {  //Third-part product do not care whether the network is connected.
        if (info->link) {
            LOG_INFO("Network link detected: \tyes\n");
            screen_log_black += "Network link detected: \t\tyes\n";
        } else {
            LOG_ERROR("ERROR: network is not linked!\n");
            screen_log_black += "Network link detected: \t\tno\n\tERROR: network is not linked!\n";
            screen_log_red += "\t错误：网口未连接\n";
            ret = false;
            goto error;
        }
    }
    
    if (info->speed == 0
            || info->speed == (unsigned short)(-1)
            || info->speed == (unsigned int)(-1)) {
        LOG_ERROR("Network card speed: \tUnknown!\n");
        screen_log_black += "Network card speed: \t\tUnknown!\n";
        screen_log_red += "\t错误：网卡速率未知\n";
        ret = false;
    } else {
        LOG_INFO("Network card speed: \t%uMbps\n", info->speed);
        screen_log_black += "Network card speed: \t\t" + to_string(info->speed) + "Mbps\n";
        /* third product do not need test net speed */
        if (!control->get_third_product_state() && info->speed != ETH_LINK_SPEED) {
            LOG_ERROR("ERROR: Network speed must be %uMbps, but current is %uMbps\n",
                                        ETH_LINK_SPEED, info->speed);
            screen_log_black += "\tERROR: Network speed must be " + to_string(ETH_LINK_SPEED)
                        + "Mbps, but current is " + to_string(info->speed) + "Mbps\n";
            screen_log_red += "\t错误：网卡速率必须为" + to_string(ETH_LINK_SPEED)
                        + "Mbps,但检测到速率为" + to_string(info->speed) + "Mbps\n";
            ret = false;
        }
    }

    screen_log_black += "Network card duplex: \t\t" + net_get_duplex_desc(info->duplex) + "\n";
    LOG_INFO("Network card duplex: \t\t%s\n", (net_get_duplex_desc(info->duplex)).c_str());
    if (!control->get_third_product_state() && info->duplex != DUPLEX_FULL) {
        screen_log_black += "\tERROR: Network duplex must be Full, but current is "
                    + net_get_duplex_desc(info->duplex) + "\n";
        screen_log_red += "\t错误：网卡必须为Full全双工，但检测到网卡为"
                    + net_get_duplex_desc(info->duplex) + "\n";
        LOG_ERROR("ERROR: Network duplex must be Full, but current is %s\n",
                    (net_get_duplex_desc(info->duplex)).c_str());
        ret = false;
    }
    
    if (test_flag) {
        info->recv_num = 0;
        net_send_broadcast_msg(info, TOTAL_SEND_NUM); //send 100 packet msg

        LOG_INFO("before wait 2 seconds, info->recv_num=%d", info->recv_num);
        // wait for 2 seconds to recv packet msg
        while (1) {
            i++;
            if (info->recv_num == TOTAL_SEND_NUM || i == 100){
                break;
            }
            usleep(20000);
        }

        if (info->recv_num < ETH_RECV_MIN_NUM) {
            screen_log_red += "\t错误：网口收报个数未达标\n";
            ret = false;
        }

        LOG_INFO("send package num: \t\t%d\n", TOTAL_SEND_NUM);
        LOG_INFO("recv package num: \t\t%d\n", info->recv_num);
        screen_log_black += "send package num: \t\t" + to_string(TOTAL_SEND_NUM) 
                        + "\nrecv package num: \t\t" + to_string(info->recv_num) + "\n\n";
    }
error:

    return ret;
}

void* NetTest::test_all(void*)
{
    Control *control = Control::get_control();
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_NET], false);
    
    screen_log_black = "";
    screen_log_red = "";
    screen_log_black += "==================== " + INTERFACE_TEST_NAME[I_NET] + " ====================\n";
    bool is_pass = net_test_all(true);
    if (is_pass) {
        LOG_INFO("net test result:\tPASS\n");
        screen_log_black += INTERFACE_TEST_NAME[I_NET] + "结果：\t\t\t成功\n\n";
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_NET], true);
    } else {
        LOG_INFO("net test result:\tFAIL\n");
        screen_log_red = INTERFACE_TEST_NAME[I_NET] + "结果：\t\t\t失败\n\n" + screen_log_red;
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_NET], false);
    }
    control->update_color_screen_log(screen_log_black, "black");
    if (screen_log_red != "") {
        control->update_color_screen_log(screen_log_red, "red");
    }
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_NET], true);
    return NULL;
}

void NetTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, test_all, baseInfo);
}


