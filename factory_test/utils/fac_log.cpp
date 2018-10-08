#include "../inc/fac_log.h"
#include "../inc/fac_utils.h"

#define WRITE_LOG_PATH                        "/var/log/qt.log"
#define WRITE_LOG_BAK                         "/var/log/qt_bak.log"
#define UPLOAD_MES_LOG                         "/var/log/mes.txt"

#define LOG_MAX_SIZE    (5<<20)
#define LOG_MAX_LEN     (1024)
#define LINE_SZ         (1024)

typedef void (*write_log_callback)(const char*, va_list);

static const char *log_level[LEVEL_MAX] = { (char*)"DEBUG", (char*)"INFO ", \
                                            (char*)"WARN ", (char*)"ERROR" };

static const char *msg_log_warning = "log message is long\n";
static const char *msg_log_nothing = "\n";
static pthread_mutex_t mutex;


void os_log(const char* msg, va_list list) {
    char line[LINE_SZ] = {0};
    FILE* log_file_fp = NULL;
    int file_size = 0;
    
    log_file_fp = fopen(WRITE_LOG_PATH, "a+");

    if (log_file_fp)
        pthread_mutex_lock(&mutex);

    snprintf(line, sizeof(line), "%s", msg);
    vfprintf(log_file_fp, line, list);
    fflush(log_file_fp);
    fclose(log_file_fp);
    
    if (log_file_fp)
        pthread_mutex_unlock(&mutex);
    
    get_file_size(WRITE_LOG_PATH, &file_size);
    if (file_size >= LOG_MAX_SIZE) {
		rename(WRITE_LOG_PATH, WRITE_LOG_BAK);
	}
}

void __write_log(write_log_callback func, const char *fname, const char *function, \
                        int line, int level, const char *fmt, va_list args) {
    char msg[LOG_MAX_LEN + 1];
    char *tmp = msg;
    const char *buf = fmt;
    char tmp_buf[TIME_MAX_LEN] = {0,};
    
    size_t len_fmt  = strnlen(fmt, LOG_MAX_LEN);
    size_t file_len = (fname) ? (strnlen(fname, LOG_MAX_LEN)) : (0);
    size_t func_len = (function) ? (strnlen(function, LOG_MAX_LEN)) : (0);
    size_t line_len = (line >= 0) ? (sizeof(int)) : (0);
    size_t buf_left = LOG_MAX_LEN;
    size_t len_all  = len_fmt;
    size_t len = 0;

    {
        get_current_time(tmp_buf);
        len = snprintf(tmp, buf_left, "%s", tmp_buf);
        buf_left -= len;
        tmp += len;
        len_all += len;
    }

    if ((level >= 0) && (level < LEVEL_MAX)) {
        len = snprintf(tmp, buf_left, " <%s> ", log_level[level]);
        buf_left -= len;
        tmp += len;
        len_all += len;
    }

    if ((file_len) && (func_len) && (line_len)) {
        len = snprintf(tmp, buf_left, "[ %s:%s(%d) ] ", fname, function, line);
        buf_left -= len;
        tmp += len;
        len_all += len;
    }

    if (len_all == 0) {
        buf = msg_log_nothing;
    } else if (len_all >= LOG_MAX_LEN) {
        buf_left -= snprintf(tmp, buf_left, "%s", msg_log_warning);
        buf = msg;
    } else {
        snprintf(tmp, buf_left, "%s", fmt);
        if (fmt[len_fmt - 1] != '\n') {
            tmp[len_fmt]    = '\n';
            tmp[len_fmt + 1]  = '\0';
        }
        buf = msg;
    }

    func(buf, args);
}

void _write_log(const char *fname, const char *function, int line, int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __write_log(os_log, fname, function, line, level, fmt, args);
    va_end(args);
}

void write_mes_log(const char *fmt, ...)
{
    va_list args;
    char *buf;
    FILE *fp;

    buf = (char *)malloc(512);
    if (buf == NULL) {
        LOG_ERROR("Failed to alloc memory(size %d) for mes log.\n", 512);
        return;
    }

    va_start(args, fmt);
    vsnprintf(buf, 512, fmt, args);
    va_end(args);

    fp = fopen(UPLOAD_MES_LOG, "a+");
    if (fp == NULL) {
        LOG_ERROR("Failed to open send log file %s\n", UPLOAD_MES_LOG);
        free(buf);
        return;
    }
    fprintf(fp, "%s", buf);
    fflush(fp);
    fclose(fp);

    free(buf);
}

