#ifndef _FAC_LOG_H
#define _FAC_LOG_H

enum LOG_LEVEL {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR,
    LEVEL_MAX,
};

#define LOG_DEBUG(fmt, ...)     _write_log(__FILE__ , __FUNCTION__, __LINE__, DEBUG, fmt, ## __VA_ARGS__)
#define LOG_INFO(fmt, ...)      _write_log(__FILE__ , __FUNCTION__, __LINE__, INFO, fmt, ## __VA_ARGS__)
#define LOG_WARN(fmt, ...)      _write_log(__FILE__ , __FUNCTION__, __LINE__, WARN, fmt, ## __VA_ARGS__)
#define LOG_ERROR(fmt, ...)     _write_log(__FILE__ , __FUNCTION__, __LINE__, ERROR, fmt, ## __VA_ARGS__)

void _write_log(const char *file, const char *func, int line, int level, const char *fmt, ...);


#define LOG_MES(format, args...) write_mes_log(format, ##args)
void write_mes_log(const char *fmt, ...);



#endif /*__FAC_LOG_H__*/













