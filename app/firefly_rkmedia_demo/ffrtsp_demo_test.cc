#include <pthread.h>
#include <getopt.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ffrtsp/ffrtsp.hh"

#define RTSP_PORT 8554

//#define SAVE_FILE

struct RTSP_PUSH_INFO ffrtsp_push[MAXFFRTSPChn];

int FFRTSP_Send(u_int8_t* framebuff,unsigned framesize,bool * quit,int cur_chn)
{
    if (ffrtsp_push[cur_chn].fp == NULL)
	    return -1;
#ifdef SAVE_FILE
    char save_file_path[30] = "test_rtsp_";
    char str[10] = {0};
    sprintf(str, "%d", cur_chn);
    strcat(save_file_path,str);
    FILE *fp = fopen(save_file_path, "a+b");
    fwrite(framebuff, framesize, 1, fp);
    fclose(fp);
    fp = NULL;
#else
    fwrite(framebuff,framesize, 1, ffrtsp_push[cur_chn].fp);
#endif
}

static void *rtspgetbuff(void *data) {
    ffrtspGet(*(struct FFRTSPGet *)data);
}

static void *rtsppushbuff(void *data) {
    ffrtsph264Push((struct RTSP_PUSH_INFO *)data); //h265 需要用 ffrtsph265Push 
}

int main(int argc, char **argv){
  
  struct FFRTSPGet ffrtsp_get;
  ffrtsp_get.count = argc - 1;
  ffrtsp_get.callback = FFRTSP_Send;
  pthread_t rtsppush_thread[MAXFFRTSPChn];
  int i = 0;
  for(i = 0; i < argc - 1; i++) {
  	ffrtsp_get.ffrtsp_get_info[i].url = argv[i + 1];
	ffrtsp_push[i].idex = i;
	ffrtsp_push[i].port = RTSP_PORT + i;
        pthread_create(&rtsppush_thread[i], NULL, rtsppushbuff, (void *)&ffrtsp_push[i]); 
  }


  pthread_t rtspget_thread;
  pthread_create(&rtspget_thread, NULL, rtspgetbuff, (void *)&ffrtsp_get); 
 
    while (1) {
  	usleep(3000 * 1000);
  }
 
  return 0;
}
