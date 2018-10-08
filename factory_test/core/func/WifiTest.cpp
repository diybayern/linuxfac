
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

#include "../../inc/WifiTest.h"
#include "../../inc/fac_log.h"
#include "../../inc/fac_utils.h"

#define TEST_PROTO   (0xaaaa)
#define TEST_MAGIC   (0xffffeeee)

WifiInfo* g_wifi_info = NULL;
string wifi_screen_log = "";

WifiTest::WifiTest()
{
}

bool WifiTest::wifi_get_wlan_name(char* wlan_name, int size) {
	 int i = 0;
	 int num = 0;
	 int fd = -1;
	 int ret = 0;
	 struct ifconf ifconf;
	 unsigned char buf[512] = { 0, };
	 struct ifreq *ifreq = NULL;	
 
	 if (wlan_name == NULL) {
		 return false;
	 }
 
	 strcpy(wlan_name, "wlan0");
	 fd = socket(AF_INET, SOCK_STREAM, 0);
	 if (fd < 0) {
		 LOG_ERROR("(wlan name)create socket failed\n");
		 return false;
	 }
 
	 ifconf.ifc_len = INTERFACE_NUM;
	 ifconf.ifc_buf = (char *)buf;
 
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

 bool WifiTest::wifi_get_wlan_index(char* wlan_name, unsigned int* index) {
	 int fd = -1;
	 struct ifreq ifr;
 
	 if (wlan_name == NULL) {
		 return false;
	 }
 
	 fd = socket(AF_INET, SOCK_STREAM, 0);
	 if (fd < 0) {
		 LOG_ERROR("(wlan index)create socket failed\n");
		 return false;
	 }
 
	 memset(&ifr, 0, sizeof(struct ifreq));
	 strncpy(ifr.ifr_name, wlan_name, WLAN_NAME_LEN);
 
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

bool WifiTest::wifi_sprintf_mac_addr(unsigned char* src, char* dst) {
	 int ret = 0;
	 ret = sprintf((char *)dst, "%02x:%02x:%02x:%02x:%02x:%02x", src[0], src[1], src[2],
			 src[3], src[4], src[5]);
	 if (ret < 0) {
		 return false;
	 }
 
	 return true;
 }

 bool WifiTest::wifi_get_mac_addr(unsigned char* wlan_name, unsigned char* hw_buf) 
 {
	 int fd = -1;
	 struct ifreq ifr;
	 unsigned char buf[128] = { 0, };
 
	 if (wlan_name == NULL || hw_buf == NULL) {
		 return false;
	 }
 
	 fd = socket(AF_INET, SOCK_STREAM, 0);
	 if (fd < 0) {
		 LOG_ERROR("(wlan mac) create socket failed\n");
		 return false;
	 }
 
	 memset(&ifr, 0, sizeof(struct ifreq));
	 strncpy(ifr.ifr_name, (char *)wlan_name, WLAN_NAME_LEN);
 
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

 void* WifiTest::wifi_recv_loopback_msg(void *arg) {
	 int fd = -1;
	 int ret = 0;
	 int len = 0;
	 MacPacket recv_packet;
	 struct sockaddr_ll recv_sll;
	 WifiInfo* info = NULL;
	 unsigned char buf[128] = { 0, };
	 unsigned char bc_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
 
	 pthread_detach(pthread_self());
	 info = (WifiInfo*) arg;
 
	 while(1) {
		 fd = socket(PF_PACKET, SOCK_RAW, htons(TEST_PROTO));
		 if (fd < 0) {
			 LOG_ERROR("create (recv) raw sock failed\n");
			 return NULL;
		 }
 
		 memset(&recv_sll, 0, sizeof(struct sockaddr_ll));
		 recv_sll.sll_family = PF_PACKET;
		 recv_sll.sll_ifindex = info->wlan_index;
		 recv_sll.sll_protocol = htons(TEST_PROTO);
 
		 ret = bind(fd, (struct sockaddr *) &recv_sll, sizeof(recv_sll));
		 if (ret < 0) {
			 LOG_ERROR("bind recv socket failed ret=%d errno=%d\n", ret, errno);
			 close(fd);
			 return NULL;
		 }
 
		 while(1) {
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
				 LOG_INFO("recv roll back msg index=%d\n", recv_packet.index);
				 info->recv_num++;
			 } else {
				 // if broadcast package, send back orientation package
				 wifi_send_msg((char *)info->mac, (char *)recv_packet.src_mac, info->wlan_index,
						 recv_packet.index);
 
				 wifi_sprintf_mac_addr(recv_packet.src_mac, (char *)buf);
 
				 LOG_ERROR("send back to %s index=%d\n", buf, recv_packet.index);
			 }
		 }
		 
	 }
 
	 return NULL;
 }

bool WifiTest::wifi_send_msg(char* src_mac, char* dst_mac, unsigned int index, unsigned int seq) {
 
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
 
	 ret = sendto(fd, (void*) &packet, sizeof(packet), 0,
			 (struct sockaddr *) &sll, sizeof(sll));
	 if (ret < 0) {
		 LOG_ERROR("send failed ret=%d errno=%d\n", ret, errno);
		 close(fd);
		 return false;
	 }
 
	 close(fd);
	 return true;
 }

 bool WifiTest::init() {
    bool ret = 0;
    pthread_t pid;
    WifiInfo* info;

    info = (WifiInfo *)malloc(sizeof(WifiInfo));
    if (!info) {
        return false;
    }

    memset(info, 0, sizeof(WifiInfo));

    ret = wifi_get_wlan_name((char *)info->wlan_name, WLAN_NAME_LEN);
    if (ret == false) {
        LOG_ERROR("get wlan name failed\n");
        free(info);
        return false;
    }

    ret = wifi_get_wlan_index((char *)info->wlan_name, &info->wlan_index);
    if (ret == false) {
        LOG_ERROR("get wlan index failed\n");
        free(info);
        return false;
    }

    ret = wifi_get_mac_addr(info->wlan_name, info->mac);
    if (ret == false) {
        LOG_ERROR("get mac addr failed\n");
        free(info);
        return false;
    }

    int tmp = pthread_create(&pid, NULL, wifi_recv_loopback_msg, info);
    if (tmp < 0) {
        LOG_ERROR("create wifi recv thread failed\n");
        free(info);
        return false;
    }

    g_wifi_info = info;

    return true;
}

bool WifiTest::wifi_send_broadcast_msg(WifiInfo* info, int num) {

    int i = 0;
    bool ret = true;
    unsigned char dest_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    for (i = 0; i < num; i++) {
        ret &= wifi_send_msg((char *)info->mac, (char *)dest_mac, info->wlan_index, info->seq++);
        usleep(100);
    }

    return ret;
}

bool WifiTest::wifi_test_send_msg() {
    bool ret = true;
    WifiInfo *info = NULL;

    info = g_wifi_info;
    if (info == NULL){
        LOG_ERROR("wifi info is null");
		wifi_screen_log += "wifi info is null\n\n";
        return false;    
    }
    
    LOG_INFO("wifi send package test start: \n");
	wifi_screen_log += "wifi send package test start: \n\n";

    info->recv_num = 0;
    wifi_send_broadcast_msg(info, TOTAL_SEND_NUM);

    //waiting for receiving msg
    usleep(20000 * TOTAL_SEND_NUM);

    LOG_INFO("send package num: \t\t%d\n",  TOTAL_SEND_NUM);
    LOG_INFO("recv package num: \t\t%d\n",  info->recv_num);
	wifi_screen_log += "send package num:\t\t100\nrecv package num:\t\t" + to_string(info->recv_num) + "\n";
    if (info->recv_num < RECEIVE_NUM) {
        ret = false;
        LOG_ERROR("WIFI test failed!\n");
    }
    if (system("ifconfig wlan0 down") < 0){
        LOG_ERROR("wifi down error!\n");
		wifi_screen_log += "wifi down error!\n";
        ret = false;
    }
    
    return ret;
}

bool WifiTest::check_if_wifi_connect_pass(void)
{
    char wifi_info[CMD_BUF_SIZE];
    char wifi_status[CMD_BUF_SIZE];
    char wifi_ssid_mac[CMD_BUF_SIZE];
	int size = 0;
	
	if(!get_file_size("/tmp/wifi_test_info.tmp", &size)) {
		LOG_ERROR("/tmp/wifi_test_info.tmp is null\n");
		return false;
	}
	memset(wifi_info, 0, CMD_BUF_SIZE);
	if(!read_local_data("/tmp/wifi_test_info.tmp",wifi_info,size)) {
		return false;
	}
	LOG_INFO("WIFI INFO:%s\n",wifi_info);
	wifi_screen_log += "WIFI INFO:" + (string)wifi_info + "\n";;


	if(!get_file_size("/tmp/wifi.status", &size)) {
		LOG_ERROR("/tmp/wifi.status is null\n");
		return false;
	}
	memset(wifi_status, 0, CMD_BUF_SIZE);
	if(!read_local_data("/tmp/wifi.status", wifi_status, size)) {
		return false;
	}


    if (!strcmp(delNL(wifi_status), "SUCCESS")) {
        LOG_INFO("WIFI is ready\n");
		wifi_screen_log += "WIFI is ready\n";

		if(!get_file_size("/tmp/ssid.mac", &size)) {
			LOG_ERROR("/tmp/ssid.mac is null\n");
			wifi_screen_log += "/tmp/ssid.mac is null\n";
			return false;
		}
		memset(wifi_ssid_mac, 0, CMD_BUF_SIZE);
		if(!read_local_data("/tmp/ssid.mac",wifi_ssid_mac,size)) {
			LOG_ERROR("/tmp/ssid.mac read error\n");
			wifi_screen_log += "/tmp/ssid.mac read error\n";
			return false;
		}
        LOG_INFO("WIFI SSID mac:\t%s\n",wifi_ssid_mac);
		wifi_screen_log += "WIFI ssid mac:\t\t" + (string)wifi_ssid_mac + "\n";
        
        return true;
    } else {
        if (!strcmp(delNL(wifi_status), "IP not available!")) {
            LOG_ERROR("IP not available\n");
			wifi_screen_log += "IP not available\n";
            return true;
        } else {
            LOG_ERROR("WIFI fail reason: \t%s\n", wifi_status);
			wifi_screen_log += "WIFI fail reason: \t" + (string)wifi_status + "\n";
            return false;
        }
    }
}

void* WifiTest::test_all(void*)
{
	Control *control = Control::get_control();
    while (1) {
		//starting wifi test after net test over
		sleep(1);
        if (control->get_interface_test_status()->net_test_over) {
            break;
		}
	}
	control->set_interface_test_status(WIFI_TEST_NAME, false);
	wifi_screen_log += "==================== wifi test ====================\n";
	bool is_pass = false;
	string str = execute_command("bash " + WIFI_TEST_SCRIPT);
    if (str == "error"){
    	LOG_ERROR("wifi init error!\n");                        
    }else {
		if (check_if_wifi_connect_pass()) {
			is_pass = wifi_test_send_msg();		 
		}
    }

	if (is_pass) {
		wifi_screen_log += "wifi test result:\t\t\tSUCCESS\n\n";
		control->set_interface_test_result(WIFI_TEST_NAME, true); 
	} else {
		wifi_screen_log += "wifi test result:\t\t\tFAIL\n\n";
		control->set_interface_test_result(WIFI_TEST_NAME, false); 
	}
	control->update_screen_log(wifi_screen_log);
	control->set_interface_test_status(WIFI_TEST_NAME, true);
	wifi_screen_log = "";
	return NULL;
}

void WifiTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}


