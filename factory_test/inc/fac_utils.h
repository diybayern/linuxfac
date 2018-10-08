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

using std::string;


const string MEM_TEST_NAME  = "内存测试";
const string USB_TEST_NAME  = "USB测试";
const string NET_TEST_NAME  = "网口测试";
const string EDID_TEST_NAME = "EDID测试";
const string CPU_TEST_NAME  = "CPU测试";
const string HDD_TEST_NAME  = "HDD测试";
const string FAN_TEST_NAME  = "FAN测试";
const string WIFI_TEST_NAME = "WIFI测试";

const string INTERFACE_TEST_NAME  = "接口测试";
const string SOUND_TEST_NAME      = "音频测试";
const string DISPLAY_TEST_NAME    = "显示测试";
const string BRIGHT_TEST_NAME     = "亮度测试";
const string CAMERA_TEST_NAME     = "摄像头测试";
const string STRESS_TEST_NAME     = "拷机测试";
const string UPLOAD_LOG_NAME      = "上传日志";
const string NEXT_PROCESS_NAME    = "下道工序";


const string FACTORY_PATH         = "/usr/local/bin/factory/";
const string STRESS_LOCK_FILE     = FACTORY_PATH + "lock";
const string FAN_TEST_SCRIPT      = FACTORY_PATH + "fan_test.sh";
const string MEM_TEST_SCRIPT      = FACTORY_PATH + "mem_test.sh";
const string WIFI_TEST_SCRIPT     = FACTORY_PATH + "wifi_test.sh";
const string HDD_TEST_SCRIPT      = FACTORY_PATH + "hdd_test.sh";
const string CAMERA_CHECK_SCRIPT  = FACTORY_PATH + "check_camera.sh";
const string CAMERA_START_SCRIPT  = FACTORY_PATH + "start_xawtv.sh";
const string GET_CPU_TEMP_SCRIPT  = FACTORY_PATH + "get_cpu_temp.sh";

const string GET_BASEINFO_INI     = FACTORY_PATH + "hwcfg.ini";

const string MEM_UI_LOG           = FACTORY_PATH + "mem_ui_log";
const string FAC_CONFIG_FILE      = "/tmp/fac_config.conf";

#define AGAIN           (3)
#define SUCCESS         (0)
#define FAIL            (1)
#define TIME_MAX_LEN    (50)

#define NO_FTP_PATH          (1)
#define NO_JOB_NUMBER        (2)


#define USB_MAX_NUM       (10)
#define USB_BLOCK_LEN     (16)
#define USB_VENDOR_LEN    (64)
#define USB_WRITE_LEN     (1024 * 1024)

#define USB_PATH_LEN      (1024)
#define USB_SPEED_LEN     (32)

#define MAC_ADDR_LEN      (6)
#define CMD_BUF_SIZE      (256)

#define MES_FILE          "/var/log/mes.txt"

#define NEXT_LOCK         ("next")
#define PCBA_LOCK         ("PCBA")
#define WHOLE_LOCK        ("whole")
#define WHOLE_TEST_FILE   ("/tmp/whole_test")

typedef unsigned long long int uint64;

struct BaseInfo {
    BaseInfo():mem_cap(""),
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
    char* ftp_ip;
    char* ftp_user;
    char* ftp_passwd;
    char* ftp_dest_path;
    char* ftp_job_number;
    char* wifi_ssid;
    char* wifi_passwd;
    char* wifi_enp;
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
int get_random();
int get_int_value(const string str);
void get_current_time(char tmp_buf[]);
void get_current_open_time(TimeInfo* date);
void diff_running_time(TimeInfo* dst, TimeInfo* src);
bool check_file_exit(const char* filename);
bool get_file_size(const char *filename, int *size);
bool write_local_data(const char* filename, const char* mod, char* buf, int size);
bool read_local_data(const char* filename, char* buf, int size);
bool remove_local_file(const char* filename);
void get_hwinfo(HwInfo* hwInfo);
void get_baseinfo(BaseInfo* baseInfo,const string baseinfo);
int get_fac_config_from_conf(const string conf_path, FacArg *fac);
char* ftp_send_file(const char* local_file_path, FacArg* fac);
char* response_to_chinese(const char* response);
bool combine_fac_log_to_mes(string sendLogPath);
bool is_digit(string str);
char* delNL(char *line);
char* lower_to_capital(const char* lower_str, char* capital_str);
string get_current_cpu_freq();
string get_mem_info();
string get_cpu_info(CpuStatus* st_cpu);
string change_float_to_string(float fla);
void stop_gpu_stress_test(void);


#endif
