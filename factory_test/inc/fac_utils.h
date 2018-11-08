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

#define   FUNC_TYPE_NUM       (16)
#define   INTERFACE_TEST_NUM  (9)
#define   FUNC_TEST_NUM       (8)


//注意顺序
const string INTERFACE_TEST_NAME[INTERFACE_TEST_NUM]    = {"内存测试", "USB测试", "网口测试", "EDID测试", "CPU测试", "HDD测试", "SSD测试", "FAN测试", "WIFI测试"};
const string INTERFACE_TEST_MES_TAG[INTERFACE_TEST_NUM] = {"MEM", "USB", "NET", "EDID", "CPU", "HDD", "SSD", "FAN", "WIFI"};
const string FUNC_TEST_NAME[FUNC_TEST_NUM]              = {"接口测试", "音频测试", "显示测试", "亮度测试", "摄像头测试", "拷机测试", "上传日志", "下道工序"};
const string FUNC_TEST_TAG_NAME[FUNC_TEST_NUM]          = {"INTERFACE", "AUDIO", "DISPLAY", "BRIGHT", "CAMERA", "STRESS", "UPLOAD_MES_LOG", "NEXT_PROCESS"};

enum FuncType {
    MEM = 0,
    USB,
    NET,
    EDID,
    CPU,
    HDD,
    SSD,
    FAN,
    WIFI,
    INTERFACE,//must behind interface test func
    SOUND,
    BRIGHT,
    CAMERA,
    STRESS,
    UPLOAD_MES_LOG,
    NEXT_PROCESS,
};

//顺序需要与前面定义的一致
enum InterfaceTestType {
    I_MEM = 0,
    I_USB,
    I_NET,
    I_EDID,
    I_CPU,
    I_HDD,
    I_SSD,
    I_FAN,
    I_WIFI,
};

enum FuncTestType {
    F_INTERFACE = 0,
    F_SOUND,
    F_DISPLAY,
    F_BRIGHT,
    F_CAMERA,
    F_STRESS,
    F_UPLOAD_MES_LOG,
    F_NEXT_PROCESS,
};

struct MesInfo {
    MesInfo():func(""),
        status("")
    {
    }
    string func;
    string status;
};

enum InterfaceRunStatus {
    INF_RUNEND = 0,
    INF_BREAK,
    INF_RUNNING
};

enum TestStep {
    STEP_IDLE = 0,
    STEP_INTERFACE,
    STEP_SOUND,
    STEP_DISPLAY,
    STEP_BRIGHTNESS,
    STEP_CAMERA,
    STEP_STRESS,
};

const string FACTORY_PATH        = "/usr/local/bin/factory/";
const string STRESS_LOCK_FILE    = FACTORY_PATH + "lock";
const string FAN_TEST_SCRIPT     = FACTORY_PATH + "fan_test.sh";
const string MEM_TEST_SCRIPT     = FACTORY_PATH + "mem_test.sh";
const string WIFI_TEST_SCRIPT    = FACTORY_PATH + "wifi_test.sh";
const string HDD_TEST_SCRIPT     = FACTORY_PATH + "hdd_test.sh";
const string SSD_TEST_SCRIPT     = FACTORY_PATH + "ssd_test.sh";
const string CAMERA_CHECK_SCRIPT = FACTORY_PATH + "check_camera.sh";
const string CAMERA_START_SCRIPT = FACTORY_PATH + "start_xawtv.sh";
const string CAMERA_CLOSE_SCRIPT = FACTORY_PATH + "close_xawtv.sh";
const string GET_CPU_TEMP_SCRIPT = FACTORY_PATH + "get_cpu_temp.sh";

const string GET_BASEINFO_INI    = FACTORY_PATH + "hwcfg.ini";

const string MEM_UI_LOG          = FACTORY_PATH + "mem_ui_log";
const string FAC_CONFIG_NAME     = "fac_config.conf";
const string FAC_CONFIG_FILE     = "/tmp/" + FAC_CONFIG_NAME;


const string WIFI_INFO_FILE      = "/tmp/wifi_test_info.tmp";
const string WIFI_SSID_FILE      = "/tmp/ssid.mac";
const string WIFI_STATUS_FILE    = "/tmp/wifi.status";
const string HDD_STATUS_FILE     = "/tmp/hdd.status";
const string SSD_STATUS_FILE     = "/tmp/ssd.status";
const string CAMERA_WINID_FILE   = "/tmp/xawtv.winid";

const string USB_MNT_FAC_CONF    = "/mnt/usb_factory_test";
const string USB_TEST_FILE       = "usbbbbbb_test";

const string DEFAULT_FTP_IP      = "172.21.5.48";
const string DEFAULT_FTP_USER    = "test";
const string DEFAULT_FTP_PASSWD  = "test";

const int BRIGHTNESS_VALUE[6] = 
{
    7, 17, 27, 37, 47, 57,
};

#define LOG_MAX_SIZE            (5 << 20)
#define LOG_MAX_LEN             (1024)
#define LINE_SZ                 (1024)

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
#define ETH_RECV_MIN_NUM        (90)

#define TOTAL_SEND_NUM          (100)
#define WLAN_NAME_LEN           (16)
#define WLAN_RECV_MIN_NUM       (70)
#define INTERFACE_NUM           (512)

#define MES_FILE                ("/var/log/factory_test/mes.txt")
#define STRESS_RECORD           ("/var/log/factory_test/stress.log")
#define LOG_FILE                ("/var/log/factory_test/factory.log")
#define LOG_FILE_BAK            ("/var/log/factory_test/factory_bak.log")

#define NEXT_LOCK               ("next")
#define PCBA_LOCK               ("PCBA")
#define WHOLE_LOCK              ("whole")
#define WHOLE_TEST_FILE         ("/tmp/whole_test")

#define XAWTV_MAX_FAIL_COUNT    (5)

#define MEM_TEST_CAP            ("10M")
#define MEM_CAP_MIN_PERCENT     (0.9)
#define STRESS_RECORD_NUM       (10)
#define STRESS_MEM_CAP_MAX      (100)   /* M */
#define STRESS_MEM_PERCENT      (0.7)

#define STRESS_TIME_ENOUGH(x)   ((x).day == 0 && (x).hour == 4 && (x).minute == 0 && (x).second >= 0 && (x).second <= 1)
#define STRESS_ERROR_TIME(x)    ((x).day == 0 && (x).hour == 0 && (x).minute == 0 && (x).second >= 3 && (x).second <= 4)
#define STRESS_MEMTEST_START(x) ((x).day == 0 && (x).hour == 0 && (x).minute == 30 && (x).second >= 0 && (x).second <= 1)
#define STRESS_MEMTEST_ITV(x)   ((x).day == 0 && (x).hour == 0 && (x).minute == 10 && (x).second >= 0 && (x).second <= 1)

#define STRING_RESULT(x)        ((x) ? "PASS" : "FAIL")
#define BOOL_RESULT(x)          ((x) == "PASS" ? true : false)
#define CHINESE_RESULT(x)       ((x) == "PASS" ? "成功" : "失败")

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

string execute_command(string cmd, bool norm_print);
int get_random();
int get_int_value(const string str);
string get_current_time();
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
