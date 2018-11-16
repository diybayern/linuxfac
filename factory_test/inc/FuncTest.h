#ifndef _FUNC_TEST_H
#define _FUNC_TEST_H

#include "Control.h"
#include "FuncBase.h"

#include <dirent.h>
#include <libudev.h>
#include <stdint.h>
#include <stdbool.h>

class MemTest : public FuncBase
{
public:
    static bool compare_men_cap(int mem_cap);
    static bool mem_stability_test();
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);
private:
    static string screen_log_black;
    static string screen_log_red;
};

class UsbTest : public FuncBase
{
public:
    static bool usb_num_test(string total_num, string num_3);
    static bool get_dev_mount_point(struct udev_device* dev, char* dst);
    static struct udev_device* get_child(struct udev* udev, struct udev_device* parent, string subsystem);
    static void get_usb_mass_storage(USB_INFO_T* info);
    static bool usb_test_mount(string block, string dir);
    static bool usb_test_write(string dir, string file_name);
    static bool usb_test_read(string dir, string file_name);
    static bool usb_test_umount(string dir);
    static bool usb_test_write_read(USB_INFO_T* info);
    static bool usb_test_all(int num);
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);

    bool usb_test_read_status();
    bool usb_test_read_cfg(USB_INFO_T* info);
    bool usb_test_read_config(string dir);
private:
    static string screen_log_black;
    static string screen_log_red;

};

class CpuTest : public FuncBase
{
public:
    bool is_cpu_test_pass(BaseInfo* baseInfo);
    void start_test(BaseInfo* baseInfo);
private:
    string screen_log_black;
    string screen_log_red;
};

class HddTest : public FuncBase
{
public:
    static bool hdd_test_all(string hdd_cap);
    static bool check_if_hdd_pass();
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);
private:
    static string screen_log_black;
    static string screen_log_red;
};

class SsdTest : public FuncBase
{
public:
    static bool ssd_test_all(string ssd_cap);
    static bool check_if_ssd_pass();
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);
private:
    static string screen_log_black;
    static string screen_log_red;

};

class FanTest : public FuncBase
{
public:
    static string fan_speed_test(string speed);
    static void *test_all(void *arg);
    void start_test(BaseInfo* baseInfo);
private:
    static string screen_log_black;
    static string screen_log_red;
};

class InterfaceTest : public FuncBase
{
public:
    static void* test_all(void *arg);
    void start_test(BaseInfo* baseInfo);
};

class PowerTest : public FuncBase
{
public:
    static void *test_all(void*);
    void start_test(BaseInfo* baseInfo);
};

class StressTest : public FuncBase
{
public:
    static void *test_all(void* arg);
    void start_test(BaseInfo* baseInfo);
    static void* gpu_stress_test(void*);
    static void* camera_stress_test(void* arg);
    static bool start_cpuburn_stress();
    static void stop_cpuburn_stress();
    static void* mem_stress_test(void*);
    static void stop_mem_stress_test();
    string get_stress_result_record();
private:
    static int  mem_stress_test_num;
    static bool mem_stress_status;
    static bool mem_stress_result;
    static string stress_result;
};

class UploadMesLog : public FuncBase
{
public:
    static void* test_all(void*);
    void start_test(BaseInfo* baseInfo);
};

class NextProcess : public FuncBase
{
public:
    static void* test_all(void* arg);
    void start_test(BaseInfo* baseInfo);
    static void next_process_handle(BaseInfo* baseInfo);
    void init();
};

#endif

