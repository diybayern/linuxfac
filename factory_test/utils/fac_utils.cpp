#include "../inc/fac_utils.h"
#include "../inc/fac_log.h"

#include <ftplib.h>
#include <map>

using namespace std;

netbuf* ftp_handle;

#define DEFAULT_FTP_IP       "172.21.5.48"
#define DEFAULT_FTP_USER     "test"
#define DEFAULT_FTP_PASSWD   "test"
#define DEFAULT_WIFI_SSID    "sfc-test"
#define DEFAULT_WIFI_PASSWD  "12345678"
#define DEFAULT_WIFI_ENP     "WPA"

/*
**execute command and return output result
*/
string execute_command(string cmd) {
    string cmd_result = "";
    char result[1024];
    int rc = 0;
    FILE *fp;
    fp = popen(cmd.c_str(),"r");
    if(fp == NULL)
    {
        perror("popen execute fail.");
        return "error";
    }
    while(fgets(result,sizeof(result),fp) != NULL)
    {
        string tempResult = result;
        cmd_result = cmd_result + tempResult;
    }
    rc = pclose(fp);
    if(rc == -1)
    {
        perror("close fp fail.");
        return "error";
    }else{
        LOG_INFO("command:%s,subprocess end status:%d,command end status:%d",cmd.c_str(),rc,WEXITSTATUS(rc));

        if(WEXITSTATUS(rc) != 0)
        {
            return "error";
        }

        if(0 < cmd_result.length())
        {
            string tmp = cmd_result.substr(cmd_result.length() - 1, cmd_result.length());
            if(tmp == "\n" || tmp == "\r"){
                return cmd_result.substr(0, cmd_result.length() - 1) + "\0";
            }else{
                return cmd_result;
            }
        }else
        {
            return cmd_result;
        }
   }
}

void get_current_time(char tmp_buf[]) {
    struct timeval  tv;
    struct timezone tz;
    struct tm nowtime;
    gettimeofday(&tv, &tz);
    localtime_r(&tv.tv_sec, &nowtime);
    strftime(tmp_buf, TIME_MAX_LEN, "%Y-%m-%d %H:%M:%S", &nowtime);
    tmp_buf += strlen(tmp_buf);
    snprintf(tmp_buf, TIME_MAX_LEN, ".%03ld", tv.tv_usec/1000);
}

void get_current_open_time(TimeInfo* date) {
	struct timespec time_space;
	
	clock_gettime(CLOCK_MONOTONIC, &time_space);
	
	date->day = time_space.tv_sec / (24 *60 * 60);
	date->hour = (time_space.tv_sec % (24 *60 * 60))/(60 * 60);
	date->minute = (time_space.tv_sec % (60 * 60))/60;
	date->second = time_space.tv_sec % 60;
}

void diff_running_time(TimeInfo* dst, TimeInfo* src) {

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

bool check_file_exit(const char* filename) { 
    if (filename == NULL){
        return false;
    }

    if (access(filename, F_OK) == 0){
        return true;
    }
    
    return false;
}

bool get_file_size(const char *filename, int *size) {
    FILE* infile = NULL;

	if ((infile = fopen(filename, "rb")) == NULL) {
		return false;
	}
    
	fseek(infile, 0L, SEEK_END);
	*size = ftell(infile);

	fclose(infile);

	return true;
}

bool write_local_data(const char* filename, const char* mod, char* buf, int size) {
	int count = 0;
	FILE * outfile = NULL;

	if ((outfile = fopen(filename, mod)) == NULL) {
		LOG_ERROR("Can't open %s\n",filename);
        return false;
	}

	count = fwrite(buf, size, 1, outfile);
	if (count != 1) {
        LOG_ERROR("Write data failed: file=%s, count=%d, size=%d\n", filename, count, size);
		fclose(outfile);
		return false;
	}

	fflush(outfile);
	fclose(outfile);

	return true;
}

bool read_local_data(const char* filename, char* buf, int size) {
	int ret = 0;
	FILE * infile = NULL;

	if ((infile = fopen(filename, "rb")) == NULL) {
		LOG_ERROR("Can't open %s\n", filename);
		return false;
	}

	ret = fread(buf, size, 1, infile);
	if (ret != 1) {
		LOG_ERROR("Read file failed: file=%s, size=%d\n", filename, size);
		fclose(infile);
		return false;
	}

	fclose(infile);
	return true;
}

bool remove_local_file(const char* filename) {
	int ret;
    if (filename == NULL){
        return true;
    }
    ret = remove(filename);
 	if (execute_command("sync") == "error" ) {
		LOG_ERROR("system sync error\n");
 	}
	
	if(ret == 0)
		return true;
    return false;
}
    

void get_hwinfo(HwInfo* hwInfo) {

	hwInfo->sn = execute_command("dmidecode -s system-serial-number");
	
	string mac = execute_command("ifconfig | grep HWaddr | awk '/eth0/ {print $5}'");
	char* new_mac = (char*)malloc(128);
	new_mac = lower_to_capital(mac.c_str(),new_mac);
	hwInfo->mac = new_mac;
	
	hwInfo->product_name = execute_command("dmidecode -s system-product-name");
    hwInfo->product_id = execute_command("dmidecode -s baseboard-product-name");
	hwInfo->product_hw_version = execute_command("dmidecode -s system-version");
	hwInfo->cpu_type = execute_command("dmidecode -s processor-version");
	hwInfo->cpu_fre = execute_command("cat /proc/cpuinfo | grep 'model name' |uniq | awk '/model name/ {print $NF}'");
	hwInfo->mem_cap = execute_command("free -m | awk '/Mem/ {print $2}'");
}



int get_int_value(const string str)
{
    if (str.size() == 0) {
        return -1;
    } else {
        return atoi(str.c_str());
    }
}

void get_baseinfo(BaseInfo* baseInfo, const string baseinfo) {
	map<string, string> tmap;
	string str;
	char buf[128] = {0};
	int cnt = 0;
	int idx;
	for (unsigned int i = 0; i < baseinfo.size(); i++) {
        if (baseinfo[i] != ';') {
            buf[cnt++]  = baseinfo[i];
        } else {
            buf[cnt] = '\0';
            str = string(buf);

            idx = str.find_first_of(':');
            tmap[str.substr(0, idx)] = str.substr(idx+1, str.length()-idx-1); 
            cnt = 0;
        }
    }
	
	if (cnt != 0) { 
		buf[cnt] = '\0';
		str = string(buf); 
		idx = str.find_first_of(':');
		tmap[str.substr(0, idx)] = str.substr(idx+1, str.length()-idx-1); 	 
	}

	string usb         = tmap["USB"];
	idx = usb.find_first_of('/');
    string usb_3 = usb.substr(0, idx);
	string usb_t = usb.substr(idx+1, usb.length()-idx-1); 

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

bool is_digit(string str) {
	int len = 0;

	len = str.size();
	if (0 == len) {
		return false;
	}

	for (int i=0; i < len; i++) {
		if (str[i] < '0' || str[i] > '9') {
			return false;
		}
	}

	return true;
}

bool read_conf_line(const string conf_path, const char* tag,char* value)
{
    FILE* conf_fp;
    if((conf_fp = fopen(conf_path.c_str(), "r")) == NULL){
        LOG_ERROR("ftp_config.conf open failed\n");
        return false;
    }
    else{
        char match[128];
        char line[256];
        sprintf(match, "%s=%%s", tag);
        
        while(fgets(line, sizeof(line), conf_fp) != NULL){
			delNL(line);
            if(line[0] != '#') {//ignore the comment
                if(strstr(line, tag)!=NULL){
                    sscanf(line, match, value);
                    return true;
                } 
            }
        }
		LOG_INFO("not find %s", tag);
    }
    return false;
}

int get_fac_config_from_conf(const string conf_path, FacArg *fac)
{
	int ret = 0;

	char* dest_path = (char*)malloc(128);
	memset(dest_path, 0, 128);
	if(read_conf_line(conf_path, "ftp_dest_path",dest_path) == false){
		LOG_ERROR("read dest_path failed\n");
		ret += NO_FTP_PATH;
	} else if (*dest_path != '\\'){
		LOG_ERROR("ftp dest_path is empty\n");
		ret += NO_FTP_PATH;
	}
	
	char* job_number = (char*)malloc(128);
	memset(job_number, 0, 128);
	if(read_conf_line(conf_path, "job_number", job_number) == false){
		LOG_ERROR("read job_number faild\n");
		ret += NO_JOB_NUMBER;
	} else if (*job_number == 0){
		LOG_ERROR("job_number is empty\n");
		ret += NO_JOB_NUMBER;
	}

    char* IP = (char*)malloc(128);
	memset(IP, 0, 128);
    if(read_conf_line(conf_path, "ftp_ip",IP) == false){
		memcpy(IP, DEFAULT_FTP_IP, strlen(DEFAULT_FTP_IP));
		LOG_INFO("use default ftp_ip\n");
    }

    char* ftp_user = (char*)malloc(128); 
	memset(ftp_user, 0, 128);
    if(read_conf_line(conf_path, "ftp_username", ftp_user) == false){
		memcpy(ftp_user, DEFAULT_FTP_USER, strlen(DEFAULT_FTP_USER));
		LOG_INFO("use default ftp_username\n");
    }

    char* ftp_passwd = (char*)malloc(128);
    memset(ftp_passwd, 0, 128);
	if(read_conf_line(conf_path, "ftp_passwd",ftp_passwd) == false){
        memcpy(ftp_passwd, DEFAULT_FTP_PASSWD, strlen(DEFAULT_FTP_PASSWD));
		LOG_INFO("use default ftp_passwd\n");
    }

    fac->ftp_ip = IP;
    fac->ftp_user = ftp_user;
    fac->ftp_passwd = ftp_passwd;
    fac->ftp_dest_path = dest_path;
    fac->ftp_job_number = job_number;


	char* ssid = (char*)malloc(128);
	memset(ssid, 0, 128);
	if(read_conf_line(conf_path, "wifi_ssid",ssid) == false){
		memcpy(ssid, DEFAULT_WIFI_SSID, strlen(DEFAULT_WIFI_SSID));
		LOG_INFO("use default wifi_ssid\n");
	}
		  
	char* wifi_passwd = (char*)malloc(128);
	memset(wifi_passwd, 0, 128);
	if(read_conf_line(conf_path, "wifi_passwd",wifi_passwd) == false){
		memcpy(wifi_passwd, DEFAULT_WIFI_PASSWD, strlen(DEFAULT_WIFI_PASSWD));
		LOG_INFO("use default wifi_passwd\n");
	}
			
	char* wifi_enp = (char*)malloc(128);			
	memset(wifi_enp, 0, 128);
	if(read_conf_line(conf_path, "wifi_enp", wifi_enp) == false){
		memcpy(wifi_enp, DEFAULT_WIFI_ENP, strlen(DEFAULT_WIFI_ENP));
		LOG_INFO("use default wifi_enp\n");
	}

	fac->wifi_ssid = ssid;
	fac->wifi_passwd = wifi_passwd;;
	fac->wifi_enp = wifi_enp;

	return ret;
}

char* response_to_chinese(const char* response)
{
    char* ch_res;
    ch_res = (char*)malloc(128);
    if (strstr(response, "226") != NULL) {
        strcpy(ch_res, "上传成功");
        return ch_res;
    } else if (strstr(response, "530") != NULL) {
        strcpy(ch_res, "错误！登录失败！");
        return ch_res;
    } else if (strstr(response, "553") != NULL) {
        strcpy(ch_res, "错误！无法创建文件！");
        return ch_res;
    } else if(strstr(response, "426") != NULL) {
        strcpy(ch_res, "错误！连接关闭，传送中止！");
        return ch_res;
    } else if(strstr(response, "connect faild") != NULL) {
        strcpy(ch_res, "错误！连接失败!");
        return ch_res;
    } else if(strstr(response, "550") != NULL) {
        strcpy(ch_res, "错误！路径错误！");
        return ch_res;
    } else {
        strcpy(ch_res, "未知错误！");
        return ch_res;
    }
}

bool combine_fac_log_to_mes(string sendLogPath) {
    FILE* fp_mes;
    FILE* fp_fac;
    int c;
    fp_mes = fopen(sendLogPath.c_str(),"ab");
    fp_fac = fopen("/var/log/qt.log","rb");
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
        fputc(c,fp_mes);
    }
    fclose(fp_mes);
    fclose(fp_fac);
    return true;
}



char* ftp_send_file(const char* local_file_path, FacArg* fac)
{
    LOG_INFO("send log start.\n");
    FtpInit();
    char* ftp_rsp;
    char* ret_rsp = 0;
    ret_rsp = (char*)malloc(256);
    LOG_INFO("ftp_ip:%s, user:%s, passwd:%s, path:%s\n", fac->ftp_ip, fac->ftp_user, fac->ftp_passwd, fac->ftp_dest_path);
	
    if(FtpConnect(fac->ftp_ip, &ftp_handle) != 1){
        strcpy(ret_rsp, "connect faild");
        return ret_rsp;
    }
    if(FtpLogin(fac->ftp_user, fac->ftp_passwd, ftp_handle) != 1){
        ftp_rsp = FtpLastResponse(ftp_handle);
        ret_rsp = (char*)memcpy(ret_rsp, ftp_rsp, strlen(ftp_rsp)+1);
        FtpQuit(ftp_handle);
        return ret_rsp;
    }
    if(FtpPut(local_file_path, fac->ftp_dest_path, FTPLIB_ASCII, ftp_handle) != 1){
        ftp_rsp = FtpLastResponse(ftp_handle);
        ret_rsp = (char*)memcpy(ret_rsp, ftp_rsp, strlen(ftp_rsp)+1);
        FtpQuit(ftp_handle);
        return ret_rsp;
    }
    ftp_rsp = FtpLastResponse(ftp_handle);	
    ret_rsp = (char*)memcpy(ret_rsp, ftp_rsp, strlen(ftp_rsp)+1);
    FtpQuit(ftp_handle);
    return ret_rsp;
}

char* delNL(char *line)
{
    int len = strlen(line);

    if (line[len-1] == '\n')
        line[len-1] = '\0';
    return line;
}

char* lower_to_capital(const char* lower_str, char* capital_str)
{
    int i=0;
    while (lower_str[i] != '\0') {
        if (lower_str[i]>='a' && lower_str[i]<='z') {
            capital_str[i] = lower_str[i]-32;
        } else {
            capital_str[i] = lower_str[i];
        }
        i++;
    }
    capital_str[i] = '\0';
    return capital_str;
}

int get_cpu_freq_by_id(int id){

    int ret = 0;
	char cmd[128] = {0,};

	sprintf(cmd, "cat /sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_cur_freq", id);
	string str = execute_command(cmd);
	if (str == "error") {
		return 0;
	}

	ret = is_digit((char*)str.c_str());
	if (false == ret){
		return 0;
	}

	return get_int_value(str);   
}

string get_current_cpu_freq(){

    int i = 0;
    int cpu_cur = 0;
    int cpu_max = 0;
	string cpu_freq = "";
	string str = execute_command("cat /proc/cpuinfo| grep processor| wc -l");
	if(str == "error") {
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

string get_mem_info() {

	int ret = 0;
	string mem_info = "";
	struct sysinfo si;

	ret = sysinfo(&si);
	if (-1 == ret) {
		LOG_ERROR("get mem info failed\n");
	}
	string mem_used = to_string((si.totalram - si.freeram) >> 10);
	string mem_free = to_string(si.freeram >> 10);
	
    mem_info += mem_used + "K used  " + mem_free + "K free";

	return mem_info;
}

string change_float_to_string(float fla)
{
	string str = to_string(fla);
	int i;
	for(i = 0; i < (int)str.size(); i++) {
		if(str[i] == '.') {
			break;
		}
	}
	return str.substr(0, i + 3);
}

string get_cpu_info(CpuStatus* st_cpu) {

	FILE* fp = NULL;
	char line[8192] = { 0, };
	CpuStatus cpu_info;
	CpuStatus cpu_diff;
	string cpu_str = "";
	string str = "";
	
	if ((fp = fopen("/proc/stat", "r")) == NULL) {
		LOG_ERROR("open %s failed\n", "/proc/stat");
		return cpu_str;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
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
	
	cpu_info.cpu_total = cpu_info.cpu_user + cpu_info.cpu_nice + cpu_info.cpu_sys
			+ cpu_info.cpu_idle + cpu_info.cpu_iowait + cpu_info.cpu_hardirq 
			+ cpu_info.cpu_softirq + cpu_info.cpu_steal;

	cpu_diff.cpu_total = cpu_info.cpu_total - st_cpu->cpu_total;

	cpu_diff.cpu_user = cpu_info.cpu_user - st_cpu->cpu_user;
	cpu_diff.cpu_sys = cpu_info.cpu_sys - st_cpu->cpu_sys;
	cpu_diff.cpu_idle = cpu_info.cpu_idle - st_cpu->cpu_idle;
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

void stop_gpu_stress_test(void) {
    if (system("killall -s 9 heaven_x64") < 0) {
        LOG_ERROR("system cmd run error\n");
    }
    if (system("killall -s 9 browser_x64") < 0) {
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
