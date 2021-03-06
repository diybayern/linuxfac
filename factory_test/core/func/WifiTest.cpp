
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

#include "WifiTest.h"

WebInfo* WifiTest::g_wifi_info = new WebInfo();
string WifiTest::screen_log_black = "";
string WifiTest::screen_log_red = "";

WifiTest::~WifiTest()
{
    LOG_INFO("~WifiTest()");
    if (g_wifi_info != NULL) {
        delete g_wifi_info;
        g_wifi_info = NULL;
    }
}

bool WifiTest::wifi_get_wlan_name(char* wlan_name, int size)
{
    int i = 0;
    int num = 0;
    int fd = -1;
    int ret = 0;
    struct ifconf ifconf;
    unsigned char buf[512] = { 0, };
    struct ifreq *ifreq = NULL;

    if (wlan_name == NULL) {
        LOG_ERROR("wlan name is NULL");
        return false;
    }

    strcpy(wlan_name, "wlan0");
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_ERROR("(wlan name)create socket failed\n");
        return false;
    }

    ifconf.ifc_len = INTERFACE_NUM;
    ifconf.ifc_buf = (char*)buf;

    ret = ioctl(fd, SIOCGIFCONF, &ifconf);
    if (ret < 0) {
        LOG_ERROR("(wlan name) ioctl failed\n");
        close(fd);
        return false;
    }

    ifreq = (struct ifreq*) buf;
    num = ifconf.ifc_len / sizeof(struct ifreq);

    for (i = 0; i < num; i++) {
        LOG_INFO("wlan card name %s\n", ifreq->ifr_name);
        if (strncmp("wlan", ifreq->ifr_name, 4) == 0) {
            strncpy(wlan_name, ifreq->ifr_name, size);
        }
        ifreq++;
    }
    close(fd);
    return true;

}

bool WifiTest::wifi_get_wlan_index(char* wlan_name, unsigned int* index)
{
    if (wlan_name == NULL || index == NULL) {
        LOG_ERROR("wlan name or index is NULL");
        return false;
    }
    int fd = -1;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_ERROR("(wlan index)create socket failed\n");
        return false;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, wlan_name, WEB_NAME_LEN);

    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        LOG_ERROR("(wlan index) ioctl failed\n");
        close(fd);
        return false;
    }

    *index = ifr.ifr_ifindex;
    close(fd);
    LOG_INFO("wlan %s index: %u\n", wlan_name, *index);

    return true;
}

bool WifiTest::wifi_sprintf_mac_addr(unsigned char* src, char* dst)
{
    if (src == NULL || dst == NULL) {
        LOG_ERROR("sprintf src or dst mac is null\n");
        return false;
    }
    
    int ret = 0;
    ret = sprintf((char*)dst, "%02x:%02x:%02x:%02x:%02x:%02x", src[0], src[1], src[2], src[3], src[4], src[5]);
    if (ret < 0) {
        LOG_ERROR("sprintf mac addr failed\n");
        return false;
    }

    return true;
}

bool WifiTest::wifi_get_mac_addr(unsigned char* wlan_name, unsigned char* hw_buf)
{
    if (wlan_name == NULL || hw_buf == NULL) {
        LOG_ERROR("wlan name or hw_buf parameter is wrong");
        return false;
    }

    int fd = -1;
    struct ifreq ifr;
    unsigned char buf[128] = { 0, };

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_ERROR("(wlan mac) create socket failed\n");
        return false;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, (char *)wlan_name, WEB_NAME_LEN);

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
        LOG_ERROR("(wlan mac) ioctl failed\n");
        close(fd);
        return false;
    }

    memcpy(hw_buf, ifr.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
    close(fd);

    wifi_sprintf_mac_addr(hw_buf, (char *)buf);
    LOG_INFO("get wlan0 mac addr %s\n", buf);

    return true;
}

void* WifiTest::wifi_recv_loopback_msg(void *arg)
{
    if (arg == NULL) {
        LOG_ERROR("arg is NULL");
        return NULL;
    }
    
    int fd = -1;
    int ret = 0;
    int len = 0;
    MacPacket recv_packet;
    struct sockaddr_ll recv_sll;
    WebInfo* info = NULL;
    unsigned char buf[128] = { 0, };
    unsigned char bc_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    pthread_detach(pthread_self());
    info = (WebInfo*) arg;

    while (1) {
        fd = socket(PF_PACKET, SOCK_RAW, htons(TEST_PROTO));
        if (fd < 0) {
            LOG_ERROR("create (recv) raw sock failed\n");
            return NULL;
        }

        memset(&recv_sll, 0, sizeof(struct sockaddr_ll));
        recv_sll.sll_family = PF_PACKET;
        recv_sll.sll_ifindex = info->index;
        recv_sll.sll_protocol = htons(TEST_PROTO);

        ret = bind(fd, (struct sockaddr *) &recv_sll, sizeof(recv_sll));
        if (ret < 0) {
            LOG_ERROR("bind recv socket failed ret=%d errno=%d\n", ret, errno);
            close(fd);
            return NULL;
        }

        while (1) {
            len = recvfrom(fd, &recv_packet, sizeof(recv_packet), 0, 0, 0);
        
            if (len != sizeof(recv_packet)) {
                //LOG_ERROR("recv len failed %d\n", len);
                close(fd);
                usleep(10000);
                break;
            }

            if (recv_packet.magic != TEST_MAGIC) {
                LOG_ERROR("recv magic=%d is invalid\n", recv_packet.magic);
                close(fd);
                break;
            }

            // exchange will transform message
            if (memcmp(recv_packet.dst_mac, bc_mac, MAC_ADDR_LEN) != 0) {
                //if orientation package, increase the rece_num
                wifi_sprintf_mac_addr(recv_packet.src_mac, (char *)buf);
                LOG_INFO("recv roll back msg index=%d, from mac=%s\n", recv_packet.index, buf);
                info->recv_num++;
            } else {
                // if broadcast package, send back orientation package
                wifi_send_msg((char *)info->mac, (char *)recv_packet.src_mac, info->index, recv_packet.index);

                wifi_sprintf_mac_addr(recv_packet.src_mac, (char *)buf);
                LOG_INFO("send back to %s index=%d\n", buf, recv_packet.index);
            }
        }
        
    }

    return NULL;
}

bool WifiTest::wifi_send_msg(char* src_mac, char* dst_mac, unsigned int index, unsigned int seq)
{
    int fd = -1;
    int ret = 0;
    MacPacket packet;
    struct sockaddr_ll sll;

    fd = socket(PF_PACKET, SOCK_RAW, htons(TEST_PROTO));
    if (fd < 0) {
        LOG_ERROR("create (send) raw sock failed\n");
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

    ret = sendto(fd, (void*) &packet, sizeof(packet), 0, (struct sockaddr *)&sll, sizeof(sll));
    if (ret < 0) {
        LOG_ERROR("send failed ret=%d errno=%d\n", ret, errno);
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

bool WifiTest::init()
{
    bool ret = 0;
    pthread_t pid;
    WebInfo* info = NULL;

    info = g_wifi_info;
    if (info == NULL) {
        LOG_ERROR("wifi info is NULL");
        return false;
    }

    memset(info, 0, sizeof(WebInfo));

    ret = wifi_get_wlan_name((char *)info->name, WEB_NAME_LEN);
    if (ret == false) {
        LOG_ERROR("get wlan name failed\n");
        delete info;
        info = NULL;
        return false;
    }

    ret = wifi_get_wlan_index((char *)info->name, &info->index);
    if (ret == false) {
        LOG_ERROR("get wlan index failed\n");
        delete info;
        info = NULL;
        return false;
    }

    ret = wifi_get_mac_addr(info->name, info->mac);
    if (ret == false) {
        LOG_ERROR("get mac addr failed\n");
        delete info;
        info = NULL;
        return false;
    }

    int tmp = pthread_create(&pid, NULL, wifi_recv_loopback_msg, info);
    if (tmp < 0) {
        LOG_ERROR("create wifi recv thread failed\n");
        delete info;
        info = NULL;
        return false;
    }

    return true;
}

bool WifiTest::wifi_send_broadcast_msg(WebInfo* info, int num)
{
    if (info == NULL || num <= 0) {
        LOG_ERROR("wifi info or num is empty");
        return false;
    }
    int i = 0;
    bool ret = true;
    unsigned char dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    for (i = 0; i < num; i++) {
        ret &= wifi_send_msg((char *)info->mac, (char *)dest_mac, info->index, info->seq++);
        usleep(100);
    }

    return ret;
}

bool WifiTest::wifi_test_send_msg()
{
    bool ret = true;
    WebInfo *info = NULL;

    info = g_wifi_info;
    if (info == NULL) {
        LOG_ERROR("wifi info is null");
        screen_log_black += "wifi info is null\n\n";
        screen_log_red += "\t错误：WiFi初始化错误，获取WiFi信息失败\n";
        return false;    
    }
    
    LOG_INFO("wifi send package test start: \n");
    screen_log_black += "wifi send package test start: \n\n";

    info->recv_num = 0;
    wifi_send_broadcast_msg(info, TOTAL_SEND_NUM);

    //waiting for receiving msg
    usleep(20000 * TOTAL_SEND_NUM);

    LOG_INFO("send package num: \t\t%d\n", TOTAL_SEND_NUM);
    LOG_INFO("recv package num: \t\t%d\n", info->recv_num);
    screen_log_black += "send package num:\t\t" + to_string(TOTAL_SEND_NUM)
                    + "\nrecv package num:\t\t" + to_string(info->recv_num) + "\n";
    if (info->recv_num < WLAN_RECV_MIN_NUM) {
        ret = false;
        LOG_ERROR("WIFI test failed!\n");
        screen_log_red += "\t错误：WiFi收包个数不达标\n";
    }
    if (system("ifconfig wlan0 down") < 0) {
        LOG_ERROR("wifi down error!\n");
        screen_log_black += "wifi down error!\n";
        screen_log_red += "\t错误：WiFi关闭失败\n";
        ret = false;
    }
    
    return ret;
}

bool WifiTest::check_if_wifi_connect_pass()
{
    char wifi_info[CMD_BUF_SIZE] = {0,};
    char wifi_status[CMD_BUF_SIZE] = {0,};
    char wifi_ssid_mac[CMD_BUF_SIZE] = {0,};
    int size = 0;
    
    if (!get_file_size(WIFI_INFO_FILE, &size)) {
        LOG_ERROR("%s is null", WIFI_INFO_FILE.c_str());
        screen_log_black += "\tERROR:get wifi info error\n";
        screen_log_red += "\t错误：WiFi信息获取失败\n";
        return false;
    }

    if (!read_local_data(WIFI_INFO_FILE, wifi_info, size)) {
        LOG_ERROR("read %s error", WIFI_INFO_FILE.c_str());
        screen_log_black += "\tERROR:get wifi info error\n";
        screen_log_red += "\t错误：WiFi信息获取失败\n";
        return false;
    }
    LOG_INFO("WIFI INFO:%s", wifi_info);
    screen_log_black += "WIFI INFO:" + (string)wifi_info + "\n";


    if (!get_file_size(WIFI_STATUS_FILE, &size)) {
        LOG_ERROR("%s is null\n", WIFI_STATUS_FILE.c_str());
        screen_log_black += "\tERROR:get wifi status error\n";
        screen_log_red += "\t错误：WiFi状态获取失败\n";
        return false;
    }

    if (!read_local_data(WIFI_STATUS_FILE, wifi_status, size)) {
        LOG_ERROR("read %s error", WIFI_STATUS_FILE.c_str());
        screen_log_black += "\tERROR:get wifi status error\n";
        screen_log_red += "\t错误：WiFi状态获取失败\n";
        return false;
    }


    if (!(delNL(wifi_status).compare("SUCCESS"))) {
        LOG_INFO("WIFI is ready\n");
        screen_log_black += "WIFI is ready\n";

        if(!get_file_size(WIFI_SSID_FILE, &size)) {
            LOG_ERROR("%s is null\n", WIFI_SSID_FILE.c_str());
            screen_log_black += "\tERROR:get ssid mac error\n";
            screen_log_red += "\t错误：所连ap路由mac地址获取失败\n";
            return false;
        }

        if(!read_local_data(WIFI_SSID_FILE, wifi_ssid_mac, size)) {
            LOG_ERROR("%s read error\n", WIFI_SSID_FILE.c_str());
            screen_log_black += "\tERROR:get ssid mac error\n";
            screen_log_red += "\t错误：所连ap路由mac地址获取失败\n";
            return false;
        }
        LOG_INFO("WIFI SSID mac:\t%s\n", wifi_ssid_mac);
        screen_log_black += "WIFI ssid mac:\t\t" + (string)wifi_ssid_mac + "\n";
        
        return true;
    } else {
        if (!(delNL(wifi_status).compare("IP not available!"))) {
            LOG_ERROR("IP not available\n");
            screen_log_black += "IP not available\n";
            screen_log_red += "\t错误：IP不可用\n";
            return false;
        } else {
            LOG_ERROR("WIFI fail reason: \t%s", wifi_status);
            screen_log_black += "WIFI fail reason: \t" + (string)wifi_status + "\n";
            screen_log_red += "\t错误：" + (string)wifi_status + "\n";
            return false;
        }
    }
}

void* WifiTest::test_all(void*)
{
    pthread_detach(pthread_self());
    Control *control = Control::get_control();
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_WIFI], false);
    bool* interfaceTestOver = control->get_interface_test_over();
    while (1) {
        //starting wifi test after net test over
        if (interfaceTestOver[I_NET]) {
            break;
        }
        sleep(1);
    }

    LOG_INFO("---------- start wifi test ----------\n");
    screen_log_black = "";
    screen_log_red = "";
    screen_log_black += "==================== " + INTERFACE_TEST_NAME[I_WIFI] + " ====================\n";
    bool is_pass = false;
    FacArg* _facArg = control->get_fac_arg();
    string cmd = "bash " + WIFI_TEST_SCRIPT + " " + _facArg->wifi_ssid + " " + _facArg->wifi_passwd + " " + _facArg->wifi_enp;
    if (system(cmd.c_str()) < 0){
        LOG_ERROR("ERROR:wifi_test.sh run error!\n");
        screen_log_black += "wifi_test.sh run error!\n";
        screen_log_red += "\t错误：wifi脚本执行失败!\n";
    } else {
        if (check_if_wifi_connect_pass()) {
            is_pass = wifi_test_send_msg();        
        }
    }

    if (is_pass) {
        LOG_INFO("wifi test result:\tPASS\n");
        screen_log_black += "\n" + INTERFACE_TEST_NAME[I_WIFI] + "结果:\t\t\t成功\n\n";
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_WIFI], true); 
    } else {
        LOG_INFO("wifi test result:\tFAIL\n");
        screen_log_red = INTERFACE_TEST_NAME[I_WIFI] + "结果:\t\t\t失败\n\n" + screen_log_red;
        control->set_interface_test_result(INTERFACE_TEST_NAME[I_WIFI], false); 
    }
    control->update_color_screen_log(screen_log_black, "black");
    if (screen_log_red != "") {
        control->update_color_screen_log(screen_log_red, "red");
    }
    control->set_interface_test_status(INTERFACE_TEST_NAME[I_WIFI], true);
    return NULL;
}

void WifiTest::start_test(BaseInfo* baseInfo)
{
    if (baseInfo == NULL) {
        LOG_ERROR("baseInfo is null");
        return;
    }
    pthread_t tid;
    int err = pthread_create(&tid, NULL, test_all, baseInfo);
    if (err != 0) {
        LOG_ERROR("wifi test create thread error: %s", strerror(err));
    }
}


