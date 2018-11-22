#include "fac_utils.h"
#include "fac_log.h"

#include <ftplib.h>
#include <map>

netbuf* ftp_handle;

/*
**execute command and return output result
*/
string execute_command(string cmd, bool norm_print)
{
    if (cmd == "") {
        return "error";
    }
    string cmd_result = "";
    char result[1024];
    int rc = 0;
    FILE *fp;
    fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        LOG_ERROR("popen execute fail.");
        return "error";
    }
    while (fgets(result, sizeof(result), fp) != NULL) {
        string tempResult = result;
        cmd_result = cmd_result + tempResult;
    }
    rc = pclose(fp);
    if (rc == -1) {
        LOG_ERROR("close fp fail.");
        return "error";
    } else {
        if (norm_print) {
            LOG_INFO("command:%s, subprocess end status:%d, command end status:%d", cmd.c_str(), rc, WEXITSTATUS(rc));
        }
        if (WEXITSTATUS(rc) != 0) {
            if (!norm_print) {
                LOG_INFO("command:%s, subprocess end status:%d, command end status:%d", cmd.c_str(), rc, WEXITSTATUS(rc));
            }
            return "error";
        }

        if (cmd_result.length() > 0) {
            string tmp = cmd_result.substr(cmd_result.length() - 1, cmd_result.length());
            if (tmp == "\n" || tmp == "\r") {
                return cmd_result.substr(0, cmd_result.length() - 1) + "\0";
            } else {
                return cmd_result;
            }
        } else {
            return cmd_result;
        }
       }
}

/* get system time */
string get_current_time()
{
    char tmp_buf[TIME_MAX_LEN] = {0};//TODO: char[] tmp_buf
    struct timeval  tv;
    struct timezone tz;
    struct tm nowtime;
    gettimeofday(&tv, &tz);
    localtime_r(&tv.tv_sec, &nowtime);
    strftime(tmp_buf, TIME_MAX_LEN, "%Y-%m-%d %H:%M:%S", &nowtime);
    snprintf(tmp_buf + strlen(tmp_buf), TIME_MAX_LEN, ".%03ld", tv.tv_usec / 1000);
    
    return (string)tmp_buf;
}

/* get boot time */
void get_current_open_time(TimeInfo* date)
{
    if (date == NULL) {
        LOG_ERROR("date pointer is NULL ");
        return;
    }
    struct timespec time_space;
    
    clock_gettime(CLOCK_MONOTONIC, &time_space);
    
    date->day    = time_space.tv_sec / (24 *60 * 60);
    date->hour   = (time_space.tv_sec % (24 *60 * 60))/(60 * 60);
    date->minute = (time_space.tv_sec % (60 * 60))/60;
    date->second = time_space.tv_sec % 60;
}

void diff_running_time(TimeInfo* dst, TimeInfo* src)
{
    if (dst == NULL || src == NULL) {
        LOG_ERROR("TimeInfo is null");
        return;
    }
    
    if (dst->second < src->second) {
        dst->second += 60;
        dst->minute -= 1;
    }
    dst->second -= src->second;
    dst->second %= 60;

    if (dst->minute < src->minute) {
        dst->minute += 60;
        dst->hour -= 1;
    }
    dst->minute -= src->minute;
    dst->minute %= 60;

    if (dst->hour < src->hour) {
        dst->hour += 24;
        dst->day -= 1;
    }
    dst->hour -= src->hour;
    dst->hour %= 24;

    dst->day -= src->day;
}

bool check_file_exit(string filename)
{ 
    if (filename == "") {
        return false;
    }

    if (access(filename.c_str(), F_OK) == 0) {
        return true;
    }
    
    return false;
}

bool get_file_size(string filename, int *size)
{
    if (filename == "" || size == NULL) {
        LOG_ERROR("filename or size pointer is null");
        return false;
    }
    
    FILE* infile = NULL;
    if ((infile = fopen(filename.c_str(), "rb")) == NULL) {
        return false;
    }
    
    fseek(infile, 0L, SEEK_END);
    *size = ftell(infile);

    fclose(infile);

    return true;
}

bool write_local_data(string filename, string mod, const char* buf, int size) //TODO: char* buf
{
    if (filename == "" || buf == NULL || size == 0) {
        LOG_ERROR("parameters is wrong");
        return false;
    }

    int count = 0;
    FILE * outfile = NULL;

    if ((outfile = fopen(filename.c_str(), mod.c_str())) == NULL) {
        LOG_ERROR("Can't open %s\n", filename.c_str());
        return false;
    }

    count = fwrite(buf, size, 1, outfile);
    if (count != 1) {
        LOG_ERROR("Write data failed: file=%s, count=%d, size=%d\n", filename.c_str(), count, size);
        fclose(outfile);
        return false;
    }

    fflush(outfile);
    //fsync(fileno(outfile));    //if fsync sound test has problem!!!
    fclose(outfile);

    return true;
}

bool read_local_data(string filename, char* buf, int size) //TODO: char* buf
{
    if (filename == "" || buf == NULL || size == 0) {
        LOG_ERROR("parameters is wrong");
        return false;
    }
    
    int ret = 0;
    FILE * infile = NULL;

    infile = fopen(filename.c_str(), "rb");
    if (infile == NULL) {
        LOG_ERROR("Can't open %s\n", filename.c_str());
        return false;
    }

    ret = fread(buf, size, 1, infile);
    if (ret != 1) {
        LOG_ERROR("Read file failed: file=%s, size=%d\n", filename.c_str(), size);
        fclose(infile);
        return false;
    }

    fclose(infile);
    return true;
}

bool remove_local_file(string filename)
{
    if (filename == ""){
        return true;
    }

    int ret;
    ret = remove(filename.c_str());
    if (execute_command("sync", true) == "error" ) {
        LOG_ERROR("system sync error\n");
    }
    
    if(ret == 0) {
        return true;
    }
    
    return false;
}

void get_hwinfo(HwInfo* hwInfo)
{
    if (hwInfo == NULL) {
        LOG_ERROR("hwInfo is NULL");
        return;
    }
    string sn = execute_command("dmidecode -s system-serial-number", true);
    sn = lower_to_capital(sn);
    hwInfo->sn = sn;
    string mac = execute_command("ifconfig | grep HWaddr | awk '/eth0/ {print $5}'", true);
    mac = lower_to_capital(mac);
    hwInfo->mac = mac;
    
    hwInfo->product_name       = execute_command("dmidecode -s system-product-name", true);
    hwInfo->product_id         = execute_command("dmidecode -s baseboard-product-name", true);
    hwInfo->product_hw_version = execute_command("dmidecode -s system-version", true);
    hwInfo->cpu_type           = execute_command("dmidecode -s processor-version", true);
    hwInfo->cpu_fre            = execute_command("cat /proc/cpuinfo | grep 'model name' |uniq | awk '/model name/ {print $NF}'", true);
    hwInfo->mem_cap            = execute_command("free -m | awk '/Mem/ {print $2}'", true);
}

int get_int_value(const string str)
{
    if (str.size() == 0) {
        return 0;
    } else {
        return atoi(str.c_str());
    }
}

/* Obtain system configuration information from ini file or DMI */
void get_baseinfo(BaseInfo* baseInfo, const string info)
{
    if (baseInfo == NULL || info == "") {
        LOG_ERROR("baseInfo or info string is NULL");
        return;
    }
    
    map<string, string> tmap;
    string str;
    char buf[128] = {0};
    int cnt = 0;
    int idx;
    for (unsigned int i = 0; i < info.size(); i++) {
        if (info[i] != ';') {
            buf[cnt++] = info[i];
        } else {
            buf[cnt] = '\0';
            str = string(buf);

            idx = str.find_first_of(':');
            tmap[str.substr(0, idx)] = str.substr(idx + 1, str.length() - idx - 1); 
            cnt = 0;
        }
    }
    
    if (cnt != 0) { 
        buf[cnt] = '\0';
        str = string(buf); 
        idx = str.find_first_of(':');
        tmap[str.substr(0, idx)] = str.substr(idx + 1, str.length() - idx - 1);      
    }

    string usb   = tmap["USB"];
    idx = usb.find_first_of('/');
    string usb_3 = usb.substr(0, idx);
    string usb_t = usb.substr(idx + 1, usb.length() - idx - 1); 

    baseInfo->platform      = tmap["PLAT"];
    baseInfo->mem_cap       = tmap["MEM"];
    baseInfo->usb_total_num = usb_t;
    baseInfo->usb_3_num     = usb_3;
    baseInfo->cpu_type      = tmap["CPU"];
    baseInfo->ssd_cap       = tmap["SSD"];
    baseInfo->emmc_cap      = tmap["EMMC"];
    baseInfo->hdd_cap       = tmap["HDD"];
    baseInfo->wifi_exist    = tmap["WIFI"];
    baseInfo->fan_speed     = tmap["FAN"];
    baseInfo->bright_level  = tmap["BRT"];
    baseInfo->camera_exist  = tmap["CAM"];
    baseInfo->vga_exist     = tmap["VGA"];
    baseInfo->hdmi_exist    = tmap["HDMI"]; 
    baseInfo->lcd_info      = tmap["LCD"];
}

bool is_digit(string str)
{
    int len = 0;

    len = str.size();
    if (len == 0) {
        return false;
    }

    for (int i = 0; i < len; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }

    return true;
}

/* read ftp and wifi config from fac_config.conf */
string read_conf_line(const string conf_path, const string tag)
{
    if (conf_path == "" || tag == "" || !check_file_exit(conf_path)) {
        LOG_ERROR("path or tag is null");
        return "";
    }
    FILE* conf_fp;
    if ((conf_fp = fopen(conf_path.c_str(), "r")) == NULL) {
        LOG_ERROR("%s open failed\n", conf_path.c_str());
        return "";
    } else {
        char match[128];
        char line[CMD_BUF_SIZE];
        char value[128] = {0};             //TODO: char[] match, line,value
        sprintf(match, "%s=%%s", tag.c_str()); // format is tag=%s
        while (fgets(line, sizeof(line), conf_fp) != NULL) { //TODO: fgets
            string str = delNL(line);
            if (str[0] != '#') { //ignore the comment
                if (str.find(tag) != str.npos) {
                    sscanf(str.c_str(), match, value);
                    return value;
                }
            }
        }
        LOG_INFO("not find %s", tag.c_str());
    }
    fclose(conf_fp);
    return "";
}

int get_fac_config_from_conf(const string conf_path, FacArg *fac)
{
    if (fac == NULL) {
        LOG_ERROR("fac does not have mem malloc");
        return FAIL;
    }
    
    int ret = 0;
    string str = "";
    
    /* get wifi config */
    str = read_conf_line(conf_path, "wifi_ssid");
    if (str == "") {
        LOG_INFO("no wifi_ssid config\n");
    }
    fac->wifi_ssid = str;

    str = read_conf_line(conf_path, "wifi_passwd");
    if (str == "") {
        LOG_INFO("no wifi_passwd config\n");
    }
    fac->wifi_passwd = str;

    str = read_conf_line(conf_path, "wifi_enp");
    if (str == "") {
        LOG_INFO("no wifi_enp config\n");
    }
    fac->wifi_enp = str;

    /* get ftp config */
    /* must have ftp path and job number! */
    str = read_conf_line(conf_path, "ftp_dest_path");
    if (str == "") {
        LOG_ERROR("read dest_path failed\n");
        ret = NO_FTP_PATH;
    } else if (str[0] != '\\'){
        LOG_ERROR("ftp dest_path is empty\n");
        ret = NO_FTP_PATH;
    }
    fac->ftp_dest_path = str;

    str = read_conf_line(conf_path, "job_number");
    if (str == "") {
        LOG_ERROR("read job_number faild\n");
        if (ret == NO_FTP_PATH) {
            ret = NO_PATH_AND_NUM;
        } else {
            ret = NO_JOB_NUMBER;
        }
    }
    fac->ftp_job_number = str;

    if (ret != 0) {
        return ret;  // if job number or path is null, stop read ftp config
    }

    str = read_conf_line(conf_path, "ftp_ip");
    if (str == "") {
        LOG_INFO("ftp_ip is empty\n");
    }
    fac->ftp_ip = str;

    str = read_conf_line(conf_path, "ftp_username");
    if (str == "") {
        LOG_INFO("ftp_username is empty\n");
    }
    fac->ftp_user = str;

    str = read_conf_line(conf_path, "ftp_passwd");
    if (str == "") {
        LOG_INFO("ftp_passwd is empty\n");
    }
    fac->ftp_passwd = str;

    return ret;
}

/* translate ftp upload result to chinese */
string response_to_chinese(string response)
{
    string str_res = "";
    if (response.find("226") != response.npos) {
        str_res = "上传成功";
        return str_res;
    } else if (response.find("530") != response.npos) {
        str_res = "错误！登录失败！";
        return str_res;
    } else if (response.find("553") != response.npos) {
        str_res = "错误！无法创建文件！";
        return str_res;
    } else if (response.find("426") != response.npos) {
        str_res = "错误！连接关闭，传送中止！";
        return str_res;
    } else if (response.find("connect faild") != response.npos) {
        str_res = "错误！连接失败!";
        return str_res;
    } else if (response.find("550") != response.npos) {
        str_res = "错误！路径错误！";
        return str_res;
    } else {
        str_res = "未知错误！";
        return str_res;
    }
}

bool combine_fac_log_to_mes(string sendLogPath, string path)
{
    if (sendLogPath == "" || path == "") {
        LOG_ERROR("log path is null");
        return false;
    }
    FILE* fp_mes;
    FILE* fp_fac;
    int c;
    fp_mes = fopen(sendLogPath.c_str(), "ab");
    fp_fac = fopen(path.c_str(), "rb");
    if (fp_mes == NULL || fp_fac == NULL) {
        if (fp_mes) {
            fclose(fp_mes);
        }
        if (fp_fac) {
            fclose(fp_fac);
        }
        return false;
    }
    while ((c = fgetc(fp_fac)) != EOF) {
        fputc(c, fp_mes);
    }
    fclose(fp_mes);
    fclose(fp_fac);
    return true;
}

/* ftp upload log file*/
string ftp_send_file(string local_file_path, FacArg* fac)
{
    if (local_file_path == "" || fac == NULL) {
        LOG_ERROR("no path or ftp config");
        return "error";
    }
    LOG_INFO("send log start.\n");
    FtpInit();
    string ftp_rsp = "";
    LOG_INFO("ftp_ip:%s, user:%s, passwd:%s, path:%s\n", (fac->ftp_ip).c_str(), (fac->ftp_user).c_str(),
            (fac->ftp_passwd).c_str(), (fac->ftp_dest_path).c_str());
    
    if (FtpConnect(fac->ftp_ip.c_str(), &ftp_handle) != 1) {
        ftp_rsp = "connect faild";
        return ftp_rsp;
    }
    if (FtpLogin(fac->ftp_user.c_str(), fac->ftp_passwd.c_str(), ftp_handle) != 1) {
        ftp_rsp = FtpLastResponse(ftp_handle);
        FtpQuit(ftp_handle);
        return ftp_rsp;
    }
    if (FtpPut(local_file_path.c_str(), fac->ftp_dest_path.c_str(), FTPLIB_ASCII, ftp_handle) != 1) {
        ftp_rsp = FtpLastResponse(ftp_handle);
        FtpQuit(ftp_handle);
        return ftp_rsp;
    }
    ftp_rsp = FtpLastResponse(ftp_handle);    
    FtpQuit(ftp_handle);
    
    return ftp_rsp;
}

string delNL(string line)
{
    int len = line.size();

    if (line[len - 1] == '\n') {
        line[len - 1] = '\0';
        return line.substr(0, len - 1);
    }
    return line;
}

string lower_to_capital(string lower_str)
{
    for (unsigned int i = 0; i < lower_str.size(); i++) {
        lower_str[i] = toupper(lower_str[i]);
    }
    
    return lower_str;
}

int get_cpu_freq_by_id(int id)
{
    bool ret = false;
    string cmd = "cat /sys/devices/system/cpu/cpu" + to_string(id) + "/cpufreq/scaling_cur_freq";  //cpuinfo_cur_freq
    string str = execute_command(cmd, false);
    if (str == "error") {
        return 0;
    }

    ret = is_digit(str);
    if (ret == false){
        return 0;
    }

    return get_int_value(str);   
}

string get_current_cpu_freq()
{
    int i = 0;
    int cpu_cur = 0;
    int cpu_max = 0;
    string cpu_freq = "";
    string str = execute_command("cat /proc/cpuinfo| grep processor| wc -l", false);
    if (str == "error") {
        return cpu_freq + "\n";
    }

    for (i = 0; i < get_int_value(str); i++){
        cpu_cur = get_cpu_freq_by_id(i);

        if (cpu_max < cpu_cur){
            cpu_max = cpu_cur;
        }        
    }
    cpu_freq += change_float_to_string(1.0 * cpu_max / 1000 / 1000) + "G";
        
    return cpu_freq;
}

/* get memory used and free cap */
string get_mem_info()
{
    int ret = 0;
    string mem_info = "";
    struct sysinfo si;

    ret = sysinfo(&si);
    if (ret == -1) {
        LOG_ERROR("get mem info failed\n");
    }
    string mem_used = to_string((si.totalram - si.freeram) >> 20);
    string mem_free = to_string(si.freeram >> 20);
    
    mem_info += mem_used + "M used\t" + mem_free + "M free";

    return mem_info;
}

/* Convert float to string, keep two decimals */
string change_float_to_string(float fla)
{
    string str = to_string(fla);
    int i;
    for (i = 0; i < (int)str.size(); i++) {
        if(str[i] == '.') {
            break;
        }
    }
    return str.substr(0, i + 3);
}

/* Get cpu usage status */
string get_cpu_info(CpuStatus* st_cpu)
{
    if (st_cpu == NULL) {
        LOG_ERROR("st_cpu is NULL");
        return "";
    }
    
    FILE* fp = NULL;
    char line[8192] = { 0, }; //TODO��char to string
    CpuStatus cpu_info;
    CpuStatus cpu_diff;
    string cpu_str = "";
    string str = "";
    
    if ((fp = fopen("/proc/stat", "r")) == NULL) {
        LOG_ERROR("open /proc/stat failed\n");
        return cpu_str;
    }

    while (fgets(line, sizeof(line), fp) != NULL) { //TODO: fgets(char*, )
        if (!strncmp(line, "cpu ", 4)) {

            sscanf(line + 5,
                    "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                    &cpu_info.cpu_user, &cpu_info.cpu_nice, &cpu_info.cpu_sys,
                    &cpu_info.cpu_idle, &cpu_info.cpu_iowait,
                    &cpu_info.cpu_hardirq, &cpu_info.cpu_softirq,
                    &cpu_info.cpu_steal, &cpu_info.cpu_guest,
                    &cpu_info.cpu_guest_nice);
            break;
        }
    }
    fclose(fp);
    
    cpu_info.cpu_total  = cpu_info.cpu_user + cpu_info.cpu_nice + cpu_info.cpu_sys
            + cpu_info.cpu_idle + cpu_info.cpu_iowait + cpu_info.cpu_hardirq 
            + cpu_info.cpu_softirq + cpu_info.cpu_steal;

    cpu_diff.cpu_total  = cpu_info.cpu_total - st_cpu->cpu_total;

    cpu_diff.cpu_user   = cpu_info.cpu_user - st_cpu->cpu_user;
    cpu_diff.cpu_sys    = cpu_info.cpu_sys - st_cpu->cpu_sys;
    cpu_diff.cpu_idle   = cpu_info.cpu_idle - st_cpu->cpu_idle;
    cpu_diff.cpu_iowait = cpu_info.cpu_iowait - st_cpu->cpu_iowait;

    str = change_float_to_string(100.0 * cpu_diff.cpu_user / cpu_diff.cpu_total);
    cpu_str += str + "% usr\t";

    str = change_float_to_string(100.0 * cpu_diff.cpu_sys / cpu_diff.cpu_total);
    cpu_str += str + "% sys\n";

    str = change_float_to_string(100.0 * cpu_diff.cpu_idle / cpu_diff.cpu_total);
    cpu_str += str + "% idle\t";

    str = change_float_to_string(100.0 * cpu_diff.cpu_iowait / cpu_diff.cpu_total);
    cpu_str += str + "% iowait";

    memcpy(st_cpu, &cpu_info, sizeof(CpuStatus));

    return cpu_str;
}

void stop_gpu_stress_test()
{
    if (system("killall -s 9 heaven_x64") < 0) {
        LOG_ERROR("system cmd run error\n");
    }
    if (system("killall -s 9 browser_x64") < 0) { //TODO: confirm browser
        LOG_ERROR("system cmd run error\n");
    }
    if (system("killall -s 9 heaven_x86") < 0) {
        LOG_ERROR("system cmd run error\n");
    }
    if (system("killall -s 9 browser_x86") < 0) {
        LOG_ERROR("system cmd run error\n");
    }
    if (system("killall -s 9 heaven") < 0) {
        LOG_ERROR("system cmd run error\n");
    }
}

/* write stress record to stress.log */
void write_stress_record(vector<string> record)
{
    if (record.size() == 0) {
        LOG_INFO("stress record is null");
        return;
    }
    
    if (access(STRESS_RECORD.c_str(), F_OK) == 0) {
        remove(STRESS_RECORD.c_str());
    }

    for (unsigned int i = 0; i < record.size(); i++) {
        LOG_STRESS(record[i].c_str());
    }

}

/* get stress result, no more than 10 records */
void read_stress_record(vector<string> *record)
{
    if (record == NULL) {
        LOG_ERROR("record pointer is NULL");
        return;
    }
    
    FILE* fp = NULL;
    char line[8192] = { 0, };   //TODO: char[] line

    if ((fp = fopen(STRESS_RECORD.c_str(), "r")) == NULL) {
        LOG_ERROR("open %s failed\n", STRESS_RECORD.c_str());
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) { //TODO: fgets()
        record->push_back(string(line));
    }
    while (record->size() > STRESS_RECORD_NUM) {
        LOG_INFO("stress record is too much\n");
        record->erase(record->begin());
    }

    fclose(fp);
}

bool create_stress_test_lock(bool is_next_pro)
{
    bool ret = true;
    LOG_INFO("start creating stress lock\n");
    if (is_next_pro) {
        ret = write_local_data(STRESS_LOCK_FILE, "w+", (char*)NEXT_LOCK, sizeof(NEXT_LOCK));  // next process
    } else if (check_file_exit(WHOLE_TEST_FILE)) {
        ret = write_local_data(STRESS_LOCK_FILE, "w+", (char*)WHOLE_LOCK, sizeof(WHOLE_LOCK)); // whole test
    } else {
        ret = write_local_data(STRESS_LOCK_FILE, "w+", (char*)PCBA_LOCK, sizeof(PCBA_LOCK)); // pcba test
    }
    if (ret == false) {
        LOG_ERROR("write lock file failed");
        return false;
    }
    
    if (system("sync") < 0) {
        LOG_ERROR("cmd sync error\n");
        return false;
    }

    if (check_file_exit(STRESS_LOCK_FILE)) {
        LOG_INFO("create stress test lock success\n");
        return true;
    } else {
        LOG_ERROR("create stress test lock failed\n");
        return false;
    }
}


