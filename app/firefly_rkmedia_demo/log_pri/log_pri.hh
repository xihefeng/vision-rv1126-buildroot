#ifndef __LOG_PRI_
#define __LOG_PRI_

#include <iostream>
#include <stdio.h>  
#include <stdarg.h>
#include <cstring> 
#include <sys/time.h>

using namespace std;

#define ESC_START     "\033["
#define ESC_END       "\033[0m"
#define ESC_FLICK     "\033[5m"
#define ESC_RECER     "\033[7m"
#define COLOR_FATAL   "31;40;5m"
#define COLOR_ALERT   "31;40;1m"
#define COLOR_CRIT    "31;40;1m"
#define COLOR_ERROR   "31;40;1m"
#define COLOR_WARN    "33;40;1m"
#define COLOR_NOTICE  "34;40;1m"
#define COLOR_INFO    "32;40;1m"
#define COLOR_DEBUG   "36;40;1m"
#define COLOR_TRACE   "37;40;1m"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define Msg_Info(level,format, args...) Myprintf(level,ESC_START COLOR_INFO "[INFO]-[%s]-[%s]-[%d]:" format ESC_END, __FILENAME__, __FUNCTION__ , __LINE__, ##args)
 
#define Msg_Debug(level,format, args...) Myprintf(level,ESC_START COLOR_DEBUG "[DEBUG]-[%s]-[%s]-[%d]:" format ESC_END, __FILENAME__, __FUNCTION__ , __LINE__, ##args)
 
#define Msg_Warn(level,format, args...) Myprintf(level, ESC_START COLOR_WARN "[WARN]-[%s]-[%s]-[%d]:" format ESC_END, __FILENAME__, __FUNCTION__ , __LINE__, ##args)
 
#define Msg_Error(level,format, args...) Myprintf(level, ESC_START COLOR_ERROR "[ERROR]-[%s]-[%s]-[%d]:" format ESC_END, __FILENAME__, __FUNCTION__ , __LINE__, ##args)


#define Msg_Green(level,format, args...) Myprintf(level,ESC_START COLOR_INFO format ESC_END, ##args)
 
#define Msg_Blue(level,format, args...) Myprintf(level,ESC_START COLOR_DEBUG format ESC_END, ##args)
 
#define Msg_Yellow(level,format, args...) Myprintf(level, ESC_START COLOR_WARN format ESC_END, ##args)
 
#define Msg_Red(level,format, args...) Myprintf(level, ESC_START COLOR_ERROR format ESC_END, ##args)

#define Msg_RECER(level,format, args...) Myprintf(level, ESC_RECER format ESC_END, ##args)

void Myprintf(int level,const char *cmd, ...);
void log_pri_init(void);
unsigned long get_time(void);

#endif


