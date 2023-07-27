#include "log_pri.hh"
#include <signal.h>
#include <mutex>

#define SHOW_DEBUG
 
/*******************************
  描述  ：获取系统时间戳
  参数  ：无
  返回值：时间戳 
 *******************************/
unsigned long get_time(void)                                                                                                                                                    
{
        struct timeval ts;  
        gettimeofday(&ts, NULL);
        return (ts.tv_sec * 1000 + ts.tv_usec / 1000);
}

mutex log_mtx;
int glevel = -1;
void Myprintf(int level,const char *cmd, ...) {
#ifdef SHOW_DEBUG
	if ( glevel != -1){

		if(glevel < level)
			return;
		
		va_list args;
		log_mtx.lock();
		va_start(args,cmd);
		vprintf(cmd,args);  //函数名
		va_end(args);
		log_mtx.unlock();
	}
#endif
	return;
}

void Handenv(int signo){
	char * slevel = getenv("BLG_DEBUG_LEVEL");
	if(slevel == NULL){
		glevel = -1;
	} else {
		glevel = atoi(slevel);
		glevel++;
	}
	if(glevel > 5 || glevel < -1) {
		unsetenv("BLG_DEBUG_LEVEL");
		Msg_Info(-1,"Now: Level = %s\r\n",NULL);
		return;
	}
	setenv("BLG_DEBUG_LEVEL",to_string(glevel).c_str(),1);
	Msg_Info(-1,"Now: Level = %d\r\n",glevel);
	return;
}

void log_pri_init(void){
	char * slevel = getenv("BLG_DEBUG_LEVEL");
	if(slevel != NULL) {
		glevel = atoi(slevel);
	} 
	signal(SIGUSR1,Handenv); //kill -s SIGUSR1 <PID>
	printf("Set Debug level:\r\n");
	printf("	kill -s SIGUSR1 <PID> or killall -s SIGUSR1 <app_name>\r\n");
}