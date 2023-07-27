#ifndef __RTSP_
#define __RTSP_

#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include "log_pri.hh"
#ifdef __cplusplus 
extern "C"
{
#endif
/*Include ffmpeg header file*/
#include <unistd.h>
#include <libavformat/avformat.h> 
#include <libavcodec/avcodec.h> 
#include <libswscale/swscale.h> 
#include <libavutil/imgutils.h>  
#include <libavutil/opt.h>     
#include <libavutil/mathematics.h>   
#include <libavutil/samplefmt.h>
#include <libavutil/time.h>
#ifdef __cplusplus
}
#endif
#define msleep(x)               usleep((x)*1000)

using namespace std;

#define H264M_1920_1080_25FPS 27000
#define H264M_640_360_25FPS 4000
#define CACHE_BUFF_NUM 25
#define MAX_RTSP_FRAME 4

/*********** ffmpeg rtsp *********/
class rtsppush {
public:
	rtsppush(string name,unsigned int buff_size);
	~rtsppush();
	void exit(void);
	void run(void);
	void write(unsigned char* buff, uint32_t buffLen);
    char* m_cacheBuff;
    int m_readPos;
    int m_writePos;
    int m_cacheLen;
	mutex mtx;
	bool        exit_flag;
	unsigned int _cache_size;
private:
	unsigned int _buff_size;
	AVFormatContext *ifmt_ctx;
	AVFormatContext *ofmt_ctx;
	AVOutputFormat *ofmt;
	void  push(void);
	thread *   	thd_push;
	string          urlpath;
};

class rtspget {
public:
	rtspget(string name,int (*callback)(u_int8_t*,int,void *),void *);
	~rtspget();
	int init(void);
	void run(void);
	void exit(void);
	string codec_type;
	int h;
	int w;
	int num;
	int den;
private:
	int (*_callback)(u_int8_t*,int,void *);
	bool        exit_flag;
	void * 		_data;
	void get(void);
	void do_for_user(void);
	int rtspget_fqueue();
	queue<AVPacket *>   rtsp_queue;
	queue<AVPacket *>   rtsp_free_queue;
	thread *   	thd_get;
	thread *   	thd_do;
	string            filepath;
	AVPacket * av_packet = NULL;
	AVFormatContext *pFormatCtx;
	int videoindex;
};

/********************************/


#endif
