#include <ftplib.h>

#include "../inc/fac_utils.h"
#include "../inc/fac_log.h"

netbuf* ftp_handle;

#define de_ftp_ip       "172.21.5.48"
#define de_ftp_user     "test"
#define de_ftp_passwd   "test"
#define de_wifi_ssid    "sfc-test"
#define de_wifi_passwd  "12345678"
#define de_wifi_enp     "WPA"


static int read_conf_line(const char* conf_path, const char* tag,char* value)
{
    FILE* conf_fp;
    if((conf_fp = fopen(conf_path, "r")) == NULL){
        LOG_ERROR("ftp_config.conf open failed\n");
        return FAIL;
    }
    else{
        char match[128];
        char line[256];
        sprintf(match, "%s=%%s", tag);
        
        while(fgets(line, sizeof(line), conf_fp) != NULL){
            if(line[0] != '#') {//ignore the comment
                if(strstr(line, tag)!=NULL){
                    sscanf(line, match, value);
                    return SUCCESS;
                } 
            }
        }
    }
    return FAIL;
}

int get_fac_config_from_conf(const char* conf_path, FacArg *fac)
{
    char* IP = (char*)malloc(128);
	memset(IP, 0, 128);
    if(read_conf_line(conf_path, "ftp_ip",IP) == FAIL){
		memcpy(IP, de_ftp_ip, strlen(de_ftp_ip));
		LOG_INFO("use default ftp_ip\n");
    }

    char* ftp_user = (char*)malloc(128); 
	memset(ftp_user, 0, 128);
    if(read_conf_line(conf_path, "ftp_username", ftp_user) == FAIL){
		memcpy(ftp_user, de_ftp_user, strlen(de_ftp_user));
		LOG_INFO("use default ftp_username\n");
    }

    char* ftp_passwd = (char*)malloc(128);
    memset(ftp_passwd, 0, 128);
	if(read_conf_line(conf_path, "ftp_passwd",ftp_passwd) == FAIL){
        memcpy(ftp_passwd, de_ftp_passwd, strlen(de_ftp_passwd));
		LOG_INFO("use default ftp_passwd\n");
    }

    char* dest_path = (char*)malloc(128);
	memset(dest_path, 0, 128);
	if(read_conf_line(conf_path, "ftp_dest_path",dest_path) == FAIL){
        LOG_ERROR("read dest_path failed\n");
		return FAIL;
    }

    char* job_number = (char*)malloc(128);
    memset(job_number, 0, 128);
	if(read_conf_line(conf_path, "job_number", job_number) == FAIL){
        LOG_ERROR("read job_number faild\n");
		return FAIL - 1;
    }

    fac->ftp_ip = IP;
    fac->ftp_user = ftp_user;
    fac->ftp_passwd = ftp_passwd;
    fac->ftp_dest_path = dest_path;
    fac->ftp_job_number = job_number;


	char* ssid = (char*)malloc(128);
	memset(ssid, 0, 128);
	if(read_conf_line(conf_path, "wifi_ssid",ssid) == FAIL){
		memcpy(ssid, de_wifi_ssid, strlen(de_wifi_ssid));
		LOG_INFO("use default wifi_ssid\n");
	}
		  
	char* wifi_passwd = (char*)malloc(128);
	memset(wifi_passwd, 0, 128);
	if(read_conf_line(conf_path, "wifi_passwd",wifi_passwd) == FAIL){
		memcpy(wifi_passwd, de_wifi_passwd, strlen(de_wifi_passwd));
		LOG_INFO("use default wifi_passwd\n");
	}
			
	char* wifi_enp = (char*)malloc(128);			
	memset(wifi_enp, 0, 128);
	if(read_conf_line(conf_path, "wifi_enp", wifi_enp) == FAIL){
		memcpy(wifi_enp, de_wifi_enp, strlen(de_wifi_enp));
		LOG_INFO("use default wifi_enp\n");
	}

	fac->wifi_ssid = ssid;
	fac->wifi_passwd = wifi_passwd;;
	fac->wifi_enp = wifi_enp;

	return SUCCESS;
}

char* ftp_send_file(const char* local_file_path, FacArg fac)
{
    LOG_INFO("send log start.\n");
    FtpInit();
    char* ftp_rsp;
    char* ret_rsp = 0;
    ret_rsp = (char*)malloc(256);
    LOG_INFO("ftp_ip:%s, user:%s, passwd:%s, path:%s\n", fac.ftp_ip, fac.ftp_user, fac.ftp_passwd, fac.ftp_dest_path);
	
    if(FtpConnect(fac.ftp_ip, &ftp_handle) != 1){
        strcpy(ret_rsp, "connect faild");
        return ret_rsp;
    }
    
    if(FtpLogin(fac.ftp_user, fac.ftp_passwd, ftp_handle) != 1){
        ftp_rsp = FtpLastResponse(ftp_handle);
        ret_rsp = (char*)memcpy(ret_rsp, ftp_rsp, strlen(ftp_rsp)+1);
        FtpQuit(ftp_handle);
        return ret_rsp;
    }

    if(FtpPut(local_file_path, fac.ftp_dest_path, FTPLIB_ASCII, ftp_handle) != 1){
        ftp_rsp = FtpLastResponse(ftp_handle);
        ret_rsp = (char*)memcpy(ret_rsp, ftp_rsp, strlen(ftp_rsp)+1);
        FtpQuit(ftp_handle);
        return ret_rsp;
    }

    ftp_rsp = FtpLastResponse(ftp_handle);	
    ret_rsp = (char*)memcpy(ret_rsp, ftp_rsp, strlen(ftp_rsp)+1);
    FtpQuit(ftp_handle);
    LOG_INFO("send log end.\n");
    return ret_rsp;
}

