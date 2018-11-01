#ifndef _FAC_UTILS_H
#define _FAC_UTILS_H

#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <memory>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <vector>

using std::string;
using namespace std;

const string MEM_TEST_NAME  = "内存测试";
const string USB_TEST_NAME  = "USB测试";
const string NET_TEST_NAME  = "网口测试";
const string EDID_TEST_NAME = "EDID测试";
const string CPU_TEST_NAME  = "CPU测试";
const string HDD_TEST_NAME  = "HDD测试";
const string FAN_TEST_NAME  = "FAN测试";
const string WIFI_TEST_NAME = "WIFI测试";
const string SSD_TEST_NAME  = "SSD测试";

const string INTERFACE_TEST_NAME = "接口测试";
const string SOUND_TEST_NAME     = "音频测试";
const string DISPLAY_TEST_NAME   = "显示测试";
const string BRIGHT_TEST_NAME    = "亮度测试";
const string CAMERA_TEST_NAME    = "摄像头测试";
const string STRESS_TEST_NAME    = "拷机测试";
const string UPLOAD_LOG_NAME     = "上传日志";
const string NEXT_PROCESS_NAME   = "下道工序";


const string FACTORY_PATH        = "/usr/local/bin/factory/";
const string STRESS_LOCK_FILE    = FACTORY_PATH + "lock";
const string FAN_TEST_SCRIPT     = FACTORY_PATH + "fan_test.sh";
const string MEM_TEST_SCRIPT     = FACTORY_PATH + "mem_test.sh";
const string WIFI_TEST_SCRIPT    = FACTORY_PATH + "wifi_test.sh";
const string HDD_TEST_SCRIPT     = FACTORY_PATH + "hdd_test.sh";
const string SSD_TEST_SCRIPT     = FACTORY_PATH + "ssd_test.sh";
const string CAMERA_CHECK_SCRIPT = FACTORY_PATH + "check_camera.sh";
const string CAMERA_START_SCRIPT = FACTORY_PATH + "start_xawtv.sh";
const string GET_CPU_TEMP_SCRIPT = FACTORY_PATH + "get_cpu_temp.sh";

const string GET_BASEINFO_INI    = FACTORY_PATH + "hwcfg.ini";

const string MEM_UI_LOG          = FACTORY_PATH + "mem_ui_log";
const string FAC_CONFIG_FILE     = "/tmp/fac_config.conf";

const int BRIGHTNESS_VALUE[6] = 
{
    7, 17, 27, 37, 47, 57,
};

#define LOG_MAX_SIZE            (5 << 20)
#define LOG_MAX_LEN             (1024)
#define LINE_SZ                 (1024)

#define DEFAULT_FTP_IP          ("172.21.5.48")
#define DEFAULT_FTP_USER        ("test")
#define DEFAULT_FTP_PASSWD      ("test")

#define MEM_TEST_CAP            ("10M")

#define TIME_MAX_LEN            (50)

#define USB_MOU_KEY_NUM         (2)     /* mouse and keyboard */
#define USB_SCANNER_NUM         (1)     /* scanner */
#define USB_MAX_NUM             (10)
#define USB_BLOCK_LEN           (16)
#define USB_VENDOR_LEN          (64)
#define USB_WRITE_LEN           (1024 * 1024)
#define USB_PATH_LEN            (1024)
#define USB_SPEED_LEN           (32)

#define MAC_ADDR_LEN            (6)
#define CMD_BUF_SIZE            (256)
#define TEST_PROTO              (0xaaaa)
#define TEST_MAGIC              (0xffffeeee)

#define ETH_LINK_SPEED          (1000) /* Mbps */
#define ETH_NAME_LEN            (16)
#define ETH_STATUS_UP           (0)
#define ETH_STATUS_DOWN         (1)

#define WLAN_NAME_LEN           (16)
#define TOTAL_SEND_NUM          (100)
#define RECEIVE_NUM             (70)
#define INTERFACE_NUM           (512)

#define MES_FILE                ("/var/log/mes.txt")
#define STRESS_RECORD           ("/var/log/stress.log")
#define LOG_FILE                ("/var/log/factory.log")
#define LOG_FILE_BAK            ("/var/log/factory_bak.log")

#define NEXT_LOCK               ("next")
#define PCBA_LOCK               ("PCBA")
#define WHOLE_LOCK              ("whole")
#define WHOLE_TEST_FILE         ("/tmp/whole_test")

#define XAWTV_MAX_FAIL_COUNT    (5)

#define STRESS_RECORD_NUM       (10)
#define STRESS_MEM_CAP_MAX      (100)   /* M */
#define STRESS_MEM_PERCENT      (0.7)

#define STRESS_TIME_ENOUGH(x)   ((x).day == 0 && (x).hour == 4 && (x).minute == 0 && (x).second >= 0 && (x).second <= 1)
#define STRESS_ERROR_TIME(x)    ((x).day == 0 && (x).hour == 0 && (x).minute == 0 && (x).second >= 3 && (x).second <= 4)
#define STRESS_MEMTEST_START(x) ((x).day == 0 && (x).hour == 0 && (x).minute == 30 && (x).second >= 0 && (x).second <= 1)
#define STRESS_MEMTEST_ITV(x)   ((x).day == 0 && (x).hour == 0 && (x).minute == 10 && (x).second >= 0 && (x).second <= 1)

#define PRINT_RESULT_STR(x)     ((x) ? "PASS" : "FAIL")

enum {
    SUCCESS = 0,
    FAIL,
    AGAIN
};

enum {
    FTP_NORM = 0,
    NO_FTP_PATH,
    NO_JOB_NUMBER,
    NO_PATH_AND_NUM
};

typedef unsigned long long int uint64;

struct BaseInfo {
    BaseInfo():platform(""),
        mem_cap(""),
        usb_total_num(""),
        usb_3_num(""),
        cpu_type(""),
        ssd_cap(""),
        emmc_cap(""),
        hdd_cap(""),
        wifi_exist(""),
        fan_speed(""),
        bright_level(""),
        camera_exist(""),
        vga_exist(""),
        hdmi_exist(""),
        lcd_info("")
        {
        }

    string platform;
    string mem_cap;
    string usb_total_num;
    string usb_3_num;
    string cpu_type;
    string ssd_cap;
    string emmc_cap;
    string hdd_cap;
    string wifi_exist;
    string fan_speed;
    string bright_level;
    string camera_exist;
    string vga_exist;
    string hdmi_exist;
    string lcd_info;
};

struct HwInfo {
    HwInfo():sn(""),
        mac(""),
        product_name(""),
        product_hw_version(""),
        product_id(""),
        cpu_type(""),
        cpu_fre(""),
        mem_cap("")
        {
        }
    string sn;
    string mac;
    string product_name;
    string product_hw_version;
    string product_id;
    string cpu_type;
    string cpu_fre;
    string mem_cap;
};

struct TimeInfo {
    int day;
    int hour;
    int minute;
    int second;
};

struct FacArg{
    FacArg():ftp_ip(""),
        ftp_user(""),
        ftp_passwd(""),
        ftp_dest_path(""),
        ftp_job_number(""),
        wifi_ssid(""),
        wifi_passwd(""),
        wifi_enp("")
        {
        }
    string ftp_ip;
    string ftp_user;
    string ftp_passwd;
    string ftp_dest_path;
    string ftp_job_number;
    string wifi_ssid;
    string wifi_passwd;
    string wifi_enp;
};

struct MacPacket {
    unsigned char dst_mac[MAC_ADDR_LEN];
    unsigned char src_mac[MAC_ADDR_LEN];
    unsigned short type;
    unsigned int magic;
    unsigned int index;
    unsigned char data[100];
};

struct CpuStatus {
    uint64 cpu_total;
    uint64 cpu_user;
    uint64 cpu_nice;
    uint64 cpu_sys;
    uint64 cpu_idle;
    uint64 cpu_iowait;
    uint64 cpu_steal;
    uint64 cpu_hardirq;
    uint64 cpu_softirq;
    uint64 cpu_guest;
    uint64 cpu_guest_nice;
};

typedef struct tagUdevInfo {
    char block[USB_BLOCK_LEN];
    char vendor[USB_VENDOR_LEN];
    int speed; /* Mbps */
} UDEV_INFO_T;

typedef struct tagUsbInfo {

    int dev_num;
    struct udev* udev;
    UDEV_INFO_T dev[USB_MAX_NUM];

} USB_INFO_T;

string execute_command(string cmd);
string execute_command_err_log(string cmd);
int get_random();
int get_int_value(const string str);
void get_current_time(char tmp_buf[]);
void get_current_open_time(TimeInfo* date);
void diff_running_time(TimeInfo* dst, TimeInfo* src);
bool check_file_exit(string filename);
bool get_file_size(string filename, int *size);
bool write_local_data(string filename, string mod, char* buf, int size);
bool read_local_data(string filename, char* buf, int size);
bool remove_local_file(string filename);
void get_hwinfo(HwInfo* hwInfo);
void get_baseinfo(BaseInfo* baseInfo,const string baseinfo);
int get_fac_config_from_conf(const string conf_path, FacArg *fac);
string ftp_send_file(string local_file_path, FacArg* fac);
string response_to_chinese(string response);
bool combine_fac_log_to_mes(string sendLogPath, string path);
bool is_digit(string str);
string delNL(string line);
string lower_to_capital(string lower_str);
string get_current_cpu_freq();
string get_mem_info();
string get_cpu_info(CpuStatus* st_cpu);
string change_float_to_string(float fla);
void stop_gpu_stress_test();
void write_stress_record(vector<string> record);
void read_stress_record(vector<string> *record);


#endif
