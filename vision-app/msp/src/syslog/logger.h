#ifndef _LOGGER_H
#define _LOGGER_H
#include <syslog.h>


#ifndef DEBUG_STDOUT
#define DEBUG_STDOUT 0
#endif

#define SYS_LOG_DEBUG(fmt, args...)    \
do { \
    if (DEBUG_STDOUT) { \
        printf("[ DEBUG ] %s(): Line %d: " fmt, __func__, __LINE__, ##args);  \
    } \
} while (0)

#define SYS_LOG_INFO(fmt, args...)    \
do { \
    if (DEBUG_STDOUT)                        \
        printf("[ INFO ] %s() Line %d:" fmt, __func__, __LINE__, ##args); \
    syslog(LOG_INFO, "%s Line %d: " fmt, __func__, __LINE__, ##args); \
} while (0)

#define SYS_LOG_ERROR(fmt, args...)    \
do { \
    if (DEBUG_STDOUT)                         \
       printf("[ ERROR ] %s() Line %d:" fmt, __func__, __LINE__, ##args); \
    syslog(LOG_ERR, "%s Line %d: " fmt, __func__, __LINE__, ##args); \
} while (0)


void init_logger(const char *name);

#endif
