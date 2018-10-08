
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

#include "../../inc/NetTest.h"
#include "../../inc/fac_log.h"
#include "../../inc/fac_utils.h"

#define TEST_PROTO   (0xaaaa)
#define TEST_MAGIC   (0xffffeeee)

#define ETH_LINK_SPEED  1000 /* Mbps */

NetInfo* g_net_info = NULL;
string net_screen_log = "";

NetTest::NetTest()
{
}

bool NetTest::net_get_eth_name(char* eth_name, int size)
{
    int i = 0;
    int num = 0;
    int fd = -1;
    int ret = 0;
    struct ifconf ifconf;
    char buf[512] = { 0, };
    struct ifreq *ifreq = NULL;    

    if (NULL == eth_name) {
        return false;
    }

	strcpy(eth_name, "br0");
#if 0    // set default value
    if (is_product_idv(global_product_id)
        || (global_product_id == PRODUCT_RAIN100V2_V1_32)
        || (global_product_id == PRODUCT_RAIN200PRO_EXAM)
        || (global_product_id == PRODUCT_RAIN200PRO_EXAM_V1_20) ) {
        strcpy(eth_name, "br0");
    } else {
        strcpy(eth_name, "eth0");
    }
#endif
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_ERROR("create socket failed\n");
        return false;
    }

    ifconf.ifc_len = 512;
    ifconf.ifc_buf = (char *)buf;

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
    	if (strncmp("eth", ifreq->ifr_name, 3) == 0) {
     	   strncpy(eth_name, ifreq->ifr_name, size);
    	} else if (strncmp("br", ifreq->ifr_name, 2) == 0) {
            /**
             * Rain200Pro-Exam has the eth name beginning with 'br'.
             * real ethernet card ethN is bridged to brN.
             */
        	strncpy(eth_name, ifreq->ifr_name, size);
   		}

    ifreq++;
    }

    close(fd);

    return true;
}

bool NetTest::net_get_eth_index(char* eth_name, unsigned int* index) {

    int fd = -1;
    struct ifreq ifr;

    if (NULL == eth_name) {
        return false;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_ERROR("create socket failed\n");
        return false;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, eth_name, ETH_NAME_LEN);

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

bool NetTest::net_sprintf_mac_addr(unsigned char* src, char* dst) {

    int ret = 0;
    ret = sprintf((char *)dst, "%02x:%02x:%02x:%02x:%02x:%02x", src[0], src[1], src[2],
                    src[3], src[4], src[5]);
    if (ret < 0) {
        return false;
    }

    return true;
}

bool NetTest::net_get_mac_addr0(unsigned char* eth_name, unsigned char* hw_buf) {

    int fd = -1;
    struct ifreq ifr;
    unsigned char buf[128] = { 0, };

    if (NULL == eth_name || NULL == hw_buf) {
        return false;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_ERROR("create socket failed\n");
        return false;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, (char *)eth_name, ETH_NAME_LEN);

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
        LOG_ERROR("get mac addr failed\n");
        close(fd);
        return false;
    }

    memcpy(hw_buf, ifr.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
    close(fd);

    net_sprintf_mac_addr(hw_buf, (char *)buf);
    LOG_INFO("get mac addr %s\n", buf);

    return true;
}

void* NetTest::net_recv_loopback_msg(void *arg) {

    int fd = -1;
    int ret = 0;
    int len = 0;
    MacPacket recv_packet;
//	MacPacket send_packet;
    struct sockaddr_ll recv_sll;
//	struct sockaddr_ll send_sll;
    NetInfo* info = NULL;
    char buf[128] = { 0, };
    unsigned char bc_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    pthread_detach(pthread_self());
    info = (NetInfo*) arg;

    start: fd = socket(PF_PACKET, SOCK_RAW, htons(TEST_PROTO));
    if (fd < 0) {
        LOG_ERROR("create raw sock failed\n");
        return NULL;
    }

    memset(&recv_sll, 0, sizeof(struct sockaddr_ll));
    recv_sll.sll_family = PF_PACKET;
    recv_sll.sll_ifindex = info->eth_index;
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
            LOG_ERROR("recv len failed %d\n", len);
            close(fd);
            usleep(10000);
            goto start;
        }

        if (TEST_MAGIC != recv_packet.magic) {
            LOG_ERROR("recv magic=%d is invalid\n", recv_packet.magic);
            close(fd);
            goto start;
        }

		// exchange will transform message
        if (0 != memcmp(recv_packet.dst_mac, bc_mac, MAC_ADDR_LEN)) {
            LOG_INFO("recv roll back msg index=%d\n", recv_packet.index);
            info->recv_num++;
        } else {
			// send back
            net_send_msg((char *)info->mac, (char *)recv_packet.src_mac, info->eth_index,
                                recv_packet.index);

            net_sprintf_mac_addr(recv_packet.src_mac, (char *)buf);

            LOG_ERROR("send back to %s index=%d\n", buf, recv_packet.index);
        }
    }

    return NULL;
}

bool NetTest::init()
{
    int ret = 0;
    pthread_t pid;
    NetInfo* info;

    info = (NetInfo *)malloc(sizeof(NetInfo));
    if (!info) {
		LOG_ERROR("malloc NetInfo failed \n");
        return false;
    }

    memset(info, 0, sizeof(NetInfo));

    ret = net_get_eth_name((char *)info->eth_name, ETH_NAME_LEN);
    if (false == ret) {
        LOG_ERROR("get eth name failed\n");
        free(info);
        return false;
    }

    ret = net_get_eth_index((char *)info->eth_name, &info->eth_index);
    if (false == ret) {
        LOG_ERROR("get eth index failed\n");
        free(info);
        return false;
    }

    ret = net_get_mac_addr0(info->eth_name, info->mac);
    if (false == ret) {
        LOG_ERROR("get mac addr failed\n");
        free(info);
        return false;
    }

    ret = pthread_create(&pid, NULL, net_recv_loopback_msg, info);
    if (ret < 0) {
        LOG_ERROR("create thread failed\n");
        free(info);
        return false;
    }

    g_net_info = info;

    return true;
}

inline int NetTest::net_eth_no(char *eth_name)
{
    char *p = eth_name;

    while (*p) {
    	if (is_digit(p))
        	break;
        p++;
    }
    return atoi(p);
}

bool NetTest::net_get_eth_status(int fd, char *eth_name, unsigned int *status) {

    struct ifreq ifr;

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, eth_name, ETH_NAME_LEN);
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
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, eth_name, ETH_NAME_LEN);
    ifr.ifr_data = (char*)cmd;
    return ioctl(fd, SIOCETHTOOL, &ifr);
}

bool NetTest::net_get_eth_info(NetInfo *info)
{
    int fd, ret;
    struct ethtool_cmd ecmd;
    struct ethtool_value edata;
    char eth_name[ETH_NAME_LEN];

    if (info == NULL || info->eth_name == NULL) {
        LOG_ERROR("pointer NULL error!\n");
        return false;
    }
    if (strncmp("br", (char *)info->eth_name, 2) == 0) {
        /* ethN is bridged to brN */
        snprintf(eth_name, ETH_NAME_LEN, "eth%d", net_eth_no((char *)info->eth_name));
    } else {
        strcpy(eth_name, (char *)info->eth_name);
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_ERROR("create socket failed: %s\n", strerror(errno));
        return false;
    }

    /* Get eth status */
    ret = net_get_eth_status(fd, (char *)eth_name, &info->eth_status);
    if (ret == false) {
        close(fd);
        return false;
    }
    /* Get eth link */
    edata.cmd = ETHTOOL_GLINK;
    ret = net_test_ioctl(fd, (char *)eth_name, &edata);
    if (ret < 0) {
        LOG_ERROR("get eth link failed: %s\n", strerror(errno));
        close(fd);
        return false;
    }
    info->eth_link = edata.data;

    /* Get seed & duplex */
    ecmd.cmd = ETHTOOL_GSET;
    ret = net_test_ioctl(fd, (char *)eth_name, &ecmd);
    if (ret < 0) {
        LOG_ERROR("get eth speed & duplex info failed: %s\n", strerror(errno));
        close(fd);
        return false;
    }
    info->eth_speed = (ecmd.speed_hi << 16) | ecmd.speed;
    info->eth_duplex = ecmd.duplex;
    LOG_INFO("ethernet card speed: %uMbps, duplex: %i\n", info->eth_speed, info->eth_duplex);

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

bool NetTest::net_send_msg(char* src_mac, char* dst_mac, unsigned int index, unsigned int seq) {

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

bool NetTest::net_send_broadcast_msg(NetInfo* info, int num) {

    int i = 0;
    bool ret = 0;
    unsigned char dest_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    for (i = 0; i < num; i++) {
        ret |= net_send_msg((char *)info->mac, (char *)dest_mac, info->eth_index, info->seq++);
        usleep(100);
    }

    return ret;
}

bool NetTest::net_test_all() {

    int i = 0;
    bool ret = 0;
    NetInfo *info = NULL;

    info = g_net_info;
    if (NULL == info){
        LOG_ERROR("net info is null");
        return false;    
    }
    
    LOG_INFO("net test start: \n");
    LOG_INFO("Network card name: \t\t\t%s \n", info->eth_name);
	net_screen_log += "Network card name: \t\t";
	net_screen_log += (char*)info->eth_name;
	net_screen_log += "\n";

    ret = net_get_eth_info(info);
    if (ret == false) {
        LOG_ERROR("get eth status failed!\n");
        goto error;
    }

    if (info->eth_status == ETH_STATUS_UP) {
        LOG_INFO("Network card status: \t\tup\n");
		net_screen_log += "Network card status: \t\tup\n";
    } else {
        LOG_INFO("Network card status: \t\tdown\n");
		net_screen_log += "Network card status: \t\tdown\n\tERROR: network is down!\n";
        ret = false;
        goto error;
    }

    if (info->eth_link) {
        LOG_INFO("Network link detected: \t\tyes\n");
		net_screen_log += "Network link detected: \t\tyes\n";
    } else {
        LOG_INFO("Network link detected: \t\tno\n");
        LOG_ERROR("ERROR: network is not linked!\n");
		net_screen_log += "Network link detected: \t\tno\n\tERROR: network is not linked!\n";
        ret = false;
        goto error;
    }

    if (info->eth_speed == 0
        || info->eth_speed == (unsigned short)(-1)
        || info->eth_speed == (unsigned int)(-1)) {
        LOG_ERROR("Network card speed: \t\tUnknown!\n");
		net_screen_log += "Network card speed: \t\tUnknown!\n";
        ret = false;
    } else {
        LOG_INFO("Network card speed: \t\t%uMbps\n", info->eth_speed);
		net_screen_log += "Network card speed: \t\t" + to_string(info->eth_speed) + "Mbps\n";
        if (info->eth_speed != ETH_LINK_SPEED) {
            LOG_ERROR("ERROR: Network speed must be %uMbps but current is %uMbps\n",
                                        ETH_LINK_SPEED, info->eth_speed);
			net_screen_log += "ERROR: Network speed must be " + to_string(ETH_LINK_SPEED)
						+ "Mbps but current is " + to_string(info->eth_speed) + "Mbps\n";
            ret = false;
        }
    }

    net_screen_log += "Network card duplex: \t\t" + net_get_duplex_desc(info->eth_duplex) + "\n";
    if (info->eth_duplex != DUPLEX_FULL) {
        net_screen_log += "\nERROR: Network duplex must be full but current is "
					+ net_get_duplex_desc(info->eth_duplex) + "\n";
        ret = false;
    }

    info->recv_num = 0;
    net_send_broadcast_msg(info, 100);

	// wait for 2 seconds
    while (1) {
        i++;
        if (100 == info->recv_num || 100 == i){
            break;
        }
        usleep(20000);
    }

    if (info->recv_num < 90) {
        ret = false;
    }

    LOG_INFO("send package num: \t\t%d\n",  100);
    LOG_INFO("recv package num: \t\t%d\n",  info->recv_num);
	net_screen_log += "send package num: \t\t100\nrecv package num: \t\t" + to_string(info->recv_num) + "\n\n";

error:

    return ret;
}

void* NetTest::test_all(void*)
{
    Control *control = Control::get_control();
	control->set_interface_test_status(NET_TEST_NAME, false);
	net_screen_log += "==================== net test ====================\n";
	bool is_pass = net_test_all();
	if (is_pass) {
		LOG_INFO("net test result: \tSUCCESS\n");
    	net_screen_log += "net test result: \t\t\tSUCCESS\n\n";
        control->set_interface_test_result(NET_TEST_NAME, true);
	} else {
		LOG_INFO("net test result: \tFAIL\n");
    	net_screen_log += "net test result: \t\t\tFAIL\n\n";
        control->set_interface_test_result(NET_TEST_NAME, false);
	}
	control->update_screen_log(net_screen_log);
	control->set_interface_test_status(NET_TEST_NAME, true);
	net_screen_log = "";
	return NULL;
}

void NetTest::start_test(BaseInfo* baseInfo)
{
    pthread_t tid;
    pthread_create(&tid,NULL,test_all,baseInfo);
}


