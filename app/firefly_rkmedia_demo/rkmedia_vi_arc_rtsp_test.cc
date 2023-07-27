#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stropts.h>

#include "common/sample_common.h"
#include "rkmedia_api.h"
#include "rkmedia_vdec.h"
#include "ffrtsp/rtsp.hh"
#include <rga/rga.h>
#include <rga/RockchipRga.h>

#include <iostream>
#include <sys/time.h>
#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"
#include "dsqlite.hh"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/freetype.hpp"

#include "log_pri.hh"
#include <curl/curl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "base64/base64.h"

using namespace std;
/*******************************
 全局定义
*******************************/

rtsppush * rtspp[2];

#define CFG_PATH "/etc/ffarc.cfg"
#define DB_PATH "/userdata/ffarc/firefly.db"
#define IMG_SAVE_PATH "/userdata/ffarc/image_save/"

#define ETH_NAME "eth0"

bool Quit = false;
//测试数据，联系商务领取
#define CELING_2_POWER(x,a)  (((x) + ((a)-1)) & (~((a) - 1))) 
#define FLOOR_2_POWER(x,a)   ((x) & (~((a) - 1 )))
#define NSCALE 32
#define FACENUM	10  //支持检测的人脸数不超过10个
#define SMALL_W 640
#define SMALL_H 360
#define FACE_RECO_SEC 2
ASVLOFFSCREEN offscreen1 = {0};
ASF_SingleFaceInfo SingleDetectedFaces = {0};
ASF_FaceFeature feature1 = {0};
ASF_FaceFeature copyfeature1 = {0};
ASF_MultiFaceInfo detectedFaces;
MHandle handle = NULL;
//初始化引擎
MInt32 initMask = ASF_FACE_DETECT | ASF_FACERECOGNITION ;
dsqlite * sq = NULL;

char * httpUrl = NULL;
int http_data_update = false;
string http_data;

string LocalIpAddress;
string LocalMacAddress;
struct Http_server_info {
	int Faceid;
	string name;
};

struct Http_server_save {
	int count;
	string img;
	string MacAddress;
	string IpAddress;
	struct Http_server_info data[FACENUM];
};

Http_server_save Http_face_save = {0};
int Faceid_save[FACENUM] = {0};

/* opencv 色彩新配比 */
cv::Scalar colorArray[10] = {
        cv::Scalar(255, 255, 0, 255),//青色
        cv::Scalar(127, 255, 0, 255),//嫩绿色
        cv::Scalar(255, 0, 0, 255),//蓝色
        cv::Scalar(255, 0, 255, 255),//深粉红色
        cv::Scalar(0, 255, 255, 255),//黄色
        cv::Scalar(0, 0, 255, 255),//红色
        cv::Scalar(240, 32, 160, 255),//紫色
        cv::Scalar(0, 97, 255, 255),//橙色
        cv::Scalar(212, 255, 127, 255),//碧绿色
        cv::Scalar(0, 255, 0, 255),//绿色
};

/* 记录识别到的人脸 */
struct Face_info {
	MInt32  Faceid;
	string name;
	string path;
	MInt32 left;
	MInt32 top;
	MInt32 right;
	MInt32 bottom;
	MInt32 faceOrient;
	cv::Mat img;
	int use;
	int reco;
	int compare;
};

struct Face_save {
	int count;
	struct Face_info data[FACENUM];
};

Face_save facesave = {0};

/* 回调函数输入用户参数 */
struct callback_data {
	MHandle * handle;
	ASF_FaceFeature * data;
	string name;
	MFloat confidenceLevel;
	int Faceid;
};

callback_data fun_data;

/* 参数结构体 */
struct Session {
  CODEC_TYPE_E video_type;
  RK_U32 u32Width;
  RK_U32 u32Height;
  RK_U32 video_fps;
  RK_U32 video_num;
  RK_U32 video_den;
  char appid[120];
  char sdkkey[120];
  char activekey[120];
  char iqfilesPath[120];
  char httpUrl[120];
};

struct demo_cfg {
  int session_count;
  struct Session session_cfg[1];
};

struct demo_cfg cfg;

/*******************************
  描述  ：解析配置文件
  参数  ：配置文件路径
  返回值：成功返回 0 
 *******************************/
static int load_cfg(const char *cfg_file) {
	
	FILE *fp = fopen(cfg_file, "r");
	char line[2048];
	int count = 0;

	if (!fp) {
		Msg_Error(0, "open %s failed\n", cfg_file);
		return -1;
	}

	memset(&cfg, 0, sizeof(cfg));
	while (fgets(line, sizeof(line) - 1, fp)) {
		const char *p;
		memset(&cfg.session_cfg[count], 0, sizeof(cfg.session_cfg[count]));

		if (line[0] == '#')
			continue;

		if ((p = strstr(line, "appid="))) {
			if (sscanf(p,
						"appid=%s sdkkey=%s activekey=%s iqfilesPath=%s httpUrl=%s",
						cfg.session_cfg[count].appid,
						cfg.session_cfg[count].sdkkey,
						cfg.session_cfg[count].activekey,
						cfg.session_cfg[count].iqfilesPath,
						cfg.session_cfg[count].httpUrl) == 0 ) {
				Msg_Error(0,"parse video file failed %s.\n", p);
			}
		} else {
			continue;
		}
		count++;
	}
	cfg.session_count = count;
	fclose(fp);

	return count;
}


/*******************************
描述  ：数据库查询函数回调
参数  ：列表各项参数，用户传参数
返回值：成功返回 MOK
*******************************/
int compare(int id , const unsigned char * name, const unsigned char * path, const void * feature,int len,void * data) {
	MFloat confidenceLevel;
	int ret = 0;
	callback_data * fun_data = (callback_data *)data;
	ASF_FaceFeature feature_db;
	feature_db.feature = (MByte *)feature;
	feature_db.featureSize = (MInt32)len;
	ret = ASFFaceFeatureCompare(*(fun_data->handle), fun_data->data, &feature_db, &confidenceLevel,
			0, ASF_LIFE_PHOTO);
	if (ret != MOK) {
		Msg_Debug(2,"ASFFaceFeatureCompare failed: %x\n", ret);
	} 

	if ( confidenceLevel > 0.9 ) {
		fun_data->name = (char *)name;
		fun_data->confidenceLevel = confidenceLevel;
		fun_data->Faceid = id;
	}
	
	return ret;

}


/*******************************
描述  ：时间戳转换为日期格式
参数  ：时间戳时间，正常日期时间，时间数据大小
返回值：无
*******************************/
void timestampToTime(char* timeStamp, char* dateTime, int dateTimeSize)
{
    time_t tTimeStamp = atoll(timeStamp);
    struct tm* pTm = gmtime(&tTimeStamp);
    strftime(dateTime, dateTimeSize, "%Y-%m-%d %H:%M:%S", pTm);
}

/*******************************
描述  ：NV12 画框函数，避免使用 Opencv
参数  ：图片虚拟地址，宽，高，框x，框y，框宽，框高，R，G，B。后免 RGB 指 RGB 色域下的颜色分量。
返回值：返回 0 代表执行结束
*******************************/
int nv12_border(char *pic, int pic_w, int pic_h, int rect_x, int rect_y,
		int rect_w, int rect_h, int R, int G, int B) {
	/* Set up the rectangle border size */
	const int border = 5;

	/* RGB convert YUV */
	int Y, U, V;
	Y = 0.299 * R + 0.587 * G + 0.114 * B;
	U = -0.1687 * R + 0.3313 * G + 0.5 * B + 128;
	V = 0.5 * R - 0.4187 * G - 0.0813 * B + 128;
	/* Locking the scope of rectangle border range */
	int j, k;
	for (j = rect_y; j < rect_y + rect_h; j++) {
		for (k = rect_x; k < rect_x + rect_w; k++) {
			if (k < (rect_x + border) || k > (rect_x + rect_w - border) ||
					j < (rect_y + border) || j > (rect_y + rect_h - border)) {
				/* Components of YUV's storage address index */
				int y_index = j * pic_w + k;
				int u_index =
					(y_index / 2 - pic_w / 2 * ((j + 1) / 2)) * 2 + pic_w * pic_h;
				int v_index = u_index + 1;
				/* set up YUV's conponents value of rectangle border */
				pic[y_index] = Y;
				pic[u_index] = U;
				pic[v_index] = V;
			}
		}
	}

	return 0;
}


/*******************************
描述  ：记录色彩空间信息
参数  ：宽，高，图片格式，图片虚拟地址，offscreen
返回值：返回 1 无意义，代表结束
*******************************/
int ColorSpaceConversion(MInt32 width, MInt32 height, MInt32 format, MUInt8* imgData, ASVLOFFSCREEN& offscreen)
{
    offscreen.u32PixelArrayFormat = (unsigned int)format;
    offscreen.i32Width = width;
    offscreen.i32Height = height;

    switch (offscreen.u32PixelArrayFormat)
    {
        case ASVL_PAF_RGB24_B8G8R8:
            offscreen.pi32Pitch[0] = offscreen.i32Width * 3;
            offscreen.ppu8Plane[0] = imgData;
            break;
        case ASVL_PAF_I420:
            offscreen.pi32Pitch[0] = width;
            offscreen.pi32Pitch[1] = width >> 1;
            offscreen.pi32Pitch[2] = width >> 1;
            offscreen.ppu8Plane[0] = imgData;
            offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width;
            offscreen.ppu8Plane[2] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width * 5 / 4;
            break;
        case ASVL_PAF_NV12:
        case ASVL_PAF_NV21:
            offscreen.pi32Pitch[0] = offscreen.i32Width;
            offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
            offscreen.ppu8Plane[0] = imgData;
            offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
            break;
        case ASVL_PAF_YUYV:
        case ASVL_PAF_DEPTH_U16:
            offscreen.pi32Pitch[0] = offscreen.i32Width * 2;
            offscreen.ppu8Plane[0] = imgData;
            break;
        case ASVL_PAF_GRAY:
            offscreen.pi32Pitch[0] = offscreen.i32Width;
            offscreen.ppu8Plane[0] = imgData;
            break;
        default:
            return 0;
    }
    return 1;
}

/*******************************
描述  ：ARC 软件激活授权
参数  ：无
返回值：成功返回 0
*******************************/
int printSDKInfo()
{
	Msg_RECER(0,"\n************* ArcFace SDK Info *****************\n");
	MRESULT res = MOK;
	res = ASFOnlineActivation((char *)cfg.session_cfg[0].appid, (char *)cfg.session_cfg[0].sdkkey, (char *)cfg.session_cfg[0].activekey);
	if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res)
		Msg_Error(0,"ASFOnlineActivation failed: %x\n", res);
	else {
		Msg_Info(0,"ASFOnlineActivation sucess: %x\n", res);
		return 0;
	}

	res = ASFOfflineActivation((char*)"ArcFacePro32.dat");
	if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res)
		Msg_Error(0,"ASFOfflineActivation failed: %x\n", res);
	else {
		Msg_Info(0,"ASFOfflineActivation sucess: %x\n", res);
		return 0;
	}

	return -1;

}

/*******************************
描述  ：ffrtsp 取流回调函数，每当获取到源码流 buff 后会调用此函数
参数  ：buff 地址，buff 大小，是否退出，当前通道
返回值：成功返回 0
*******************************/
int FFRTSP_Get(u_int8_t* framebuff,int framesize,void * data)
{	
	int ret = 0;

#if 0
	rtspp[0]->write(framebuff, framesize);
	rtspp[1]->write(framebuff, framesize);
#else	
	Msg_Debug(4,"RTSP Get Pack size: %d\r\n",framesize);
	MEDIA_BUFFER mb = RK_MPI_MB_CreateBuffer(framesize, RK_FALSE, 0);
	RK_MPI_MB_SetSize(mb, framesize);
	memcpy(RK_MPI_MB_GetPtr(mb) ,framebuff , framesize);
	ret = RK_MPI_SYS_SendMediaBuffer(RK_ID_VDEC, 0, mb);
	if(ret) {
		Msg_Error(0,"RK_MPI_SYS_SendMediaBuffer VDEC err : %d\r\n", ret);
	}
	RK_MPI_MB_ReleaseBuffer(mb);
#endif
	return ret;
}

/*******************************
描述  ：初始化 Arc 人脸识别引擎
参数  ：无
返回值：无
*******************************/
int ArcBuffer(void) {
	int ret;
	//初始化引擎
	initMask = ASF_FACE_DETECT | ASF_FACERECOGNITION ;
	ret = ASFInitEngine(ASF_DETECT_MODE_VIDEO, ASF_OP_0_ONLY,
			NSCALE, FACENUM, initMask, &handle);
	if (ret != MOK) {
		Msg_Error(0,"ASFInitEngine failed: %x\n", ret);
		return -1;
	} else {
		Msg_Info(0,"ASFInitEngine sucess: %x\n", ret);
		return 0;
	}
}


/*******************************
描述  ：Rga0 通道回调
参数  ：获取到的 MEDIA_BUFFER
返回值：无
*******************************/
MEDIA_BUFFER rga0_mb;
MEDIA_BUFFER display_mb;
bool rga0_ready = true;
bool display_stat = true;
unsigned long vdec_out = 0;
void rga0_packet_cb(MEDIA_BUFFER mb) {
        if(rga0_ready == true) {
                rga0_mb = RK_MPI_MB_Copy(mb, RK_TRUE);
        	rga0_ready = false;
	}
	Msg_Debug(5,"Vdec Out Time: %dms\r\n",get_time() - vdec_out);
	vdec_out = get_time();
	RK_MPI_MB_ReleaseBuffer(mb);
}

/*******************************
描述  ：Rga0 通道回调处理线程
参数  ：用户传参数，默认为 NULL
返回值：无
*******************************/
cv::Ptr<cv::freetype::FreeType2> ft2;
static void *_rga0_packet_cb(void *data) {
	int ret = 0;
	int show = 0;
	float x_rate = (float)cfg.session_cfg[0].u32Width / SMALL_W;
	float y_rate = (float)CELING_2_POWER(cfg.session_cfg[0].u32Height,16) / SMALL_H;
	cv::Mat * dst = NULL;
	while(Quit == false) {
		if (rga0_ready == false) {
			RK_MPI_MB_BeginCPUAccess(rga0_mb, RK_FALSE);
			dst = new cv::Mat(cv::Size(cfg.session_cfg[0].u32Width ,CELING_2_POWER(cfg.session_cfg[0].u32Height,16)), CV_8UC3,(char *)RK_MPI_MB_GetPtr(rga0_mb));
			show = 0;
			Msg_Debug(3,"Arc detect face count: %d\r\n",facesave.count);
			for (int i = 0; i < facesave.count; i++){
			//for (int i = 0; i < -1; i++){
				int x = facesave.data[i].left * x_rate;
				int y = facesave.data[i].top * y_rate;
				int w = (facesave.data[i].right - facesave.data[i].left) * x_rate;
				int h = (facesave.data[i].bottom - facesave.data[i].top) * y_rate;
				if (x < 0)
					x = 0;
				if (y < 0)
					y = 0;
				while ((uint32_t)(x + w) >= cfg.session_cfg[0].u32Width) {
					w -= 16;
				}
				while ((uint32_t)(y + h) >= cfg.session_cfg[0].u32Height) {
					h -= 16;
				}
				Msg_Debug(4,"Arc detect index [%d], Point: x = %d , y = %d , w = %d , h = %d\r\n",i,x,y,w,h);
				cv::Point p1(x, y);
				cv::Point p2(x + w, y + h);
				cv::rectangle(*dst, p1, p2,
						colorArray[i], 5);
				if (facesave.data[i].compare == 1) {
					cv::Point p_name(x, y - 4);
#if 1
					ft2->putText(*dst, facesave.data[i].name, p_name, 40, colorArray[i], -1, 8, true);
#else 
					cv::putText(*dst, facesave.data[i].name, p_name, 1, 2,
						colorArray[i], 2);
#endif
					int px = CELING_2_POWER(cfg.session_cfg[0].u32Width - ( ( show++ + 1 ) * 86), 4);
					int py = CELING_2_POWER(6,4);
					int pw = CELING_2_POWER(80,4);
					int ph = CELING_2_POWER(80,4);
					cv::Rect Roi = cv::Rect(px, py, pw, ph);
					facesave.data[i].img.copyTo((*dst)(Roi));
					Msg_Debug(4,"Arc detect index [%d], name = %s\r\n", i,facesave.data[i].name);
				}
			}
			if(display_stat == true) {
                		display_mb = RK_MPI_MB_Copy(rga0_mb, RK_TRUE);
				display_stat = false;
			}
			delete dst;
			RK_MPI_MB_EndCPUAccess(rga0_mb, RK_FALSE);
			RK_MPI_MB_ReleaseBuffer(rga0_mb);
			rga0_ready = true;
		} else { 
			usleep(1000);
		}
	}
}

static void *display_fun(void *data) {
	while(Quit == false) {
		if(display_stat == false) {
			RK_MPI_SYS_SendMediaBuffer(RK_ID_RGA, 3, display_mb);
			RK_MPI_SYS_SendMediaBuffer(RK_ID_RGA, 4, display_mb);
			RK_MPI_MB_ReleaseBuffer(display_mb);
			display_stat = true;
		} else {
			usleep(1000);
		}
	}
}

/*******************************
描述  ：Rga1 通道回调
参数  ：获取到的 MEDIA_BUFFER
返回值：无
*******************************/
MEDIA_BUFFER rga1_mb;
bool rga1_ready = true;
void rga1_packet_cb(MEDIA_BUFFER mb) {
      	
	
	if(rga1_ready == true) {
                rga1_mb = RK_MPI_MB_Copy(mb, RK_TRUE);
        	rga1_ready = false;
	}
	
	RK_MPI_MB_ReleaseBuffer(mb);
}

/*******************************
描述  ：Rga1 通道回调处理线程
参数  ：用户传参数，默认为 NULL
返回值：无
*******************************/
static void *_rga1_packet_cb(void *data) {
	int ret = 0;
	int send = 0;
	while(Quit == false) {
		if (rga1_ready == false) {
			ColorSpaceConversion(SMALL_W, SMALL_H, ASVL_PAF_NV12, (MUInt8*)RK_MPI_MB_GetPtr(rga1_mb), offscreen1);
			detectedFaces.faceNum = 0;
			ret = ASFDetectFacesEx(handle, &offscreen1, &detectedFaces);
			if (ret != MOK && ret != MERR_FSDK_FACEFEATURE_MISSFACE) {
				Msg_Debug(0,"ASFDetectFaces  failed : %d\n",ret);
			}
			send = 0;
			if ( detectedFaces.faceNum > 0 ) {
				for (int i = 0; i < detectedFaces.faceNum; i++){
					if ( facesave.data[i].Faceid != detectedFaces.faceID[i] ) {
						facesave.data[i].name = "";
						facesave.data[i].use = 0;
						facesave.data[i].compare = 0;
						facesave.data[i].reco = 1;
						send = 1;
					/* 连续识别 10 次，如果都不成功就不再继续识别了 */
					} else if ( facesave.data[i].use % cfg.session_cfg[0].video_fps < 5 && 
						facesave.data[i].use / cfg.session_cfg[0].video_fps == 0 && 
							facesave.data[i].compare != 1) {
						facesave.data[i].reco = 1;
						send = 1;
					} 
					
					if (facesave.data[i].compare != 1){
						facesave.data[i].use++;
						if(facesave.data[i].use / cfg.session_cfg[0].video_fps == FACE_RECO_SEC) {
							Msg_Debug(4,"Arc detect index [%d], name = %s\r\n", i,facesave.data[i].name);
							facesave.data[i].use = 0;
						}
					}
					
					facesave.data[i].left = detectedFaces.faceRect[i].left;
					facesave.data[i].top = detectedFaces.faceRect[i].top;
					facesave.data[i].right = detectedFaces.faceRect[i].right;
					facesave.data[i].bottom = detectedFaces.faceRect[i].bottom;
					facesave.data[i].Faceid = detectedFaces.faceID[i];
					facesave.data[i].faceOrient = detectedFaces.faceOrient[i];
				}
				if(http_data_update == false) {
					//memset(&Http_face_save, 0, sizeof(Http_face_save));
					Http_face_save.count = 0;
					Http_face_save.IpAddress = LocalIpAddress;
					Http_face_save.MacAddress = LocalMacAddress;
					for (volatile int i = 0; i < facesave.count; i++){
						if(facesave.data[i].compare == 1) {
							//存储 http server 数据
							Http_face_save.data[i].Faceid = Faceid_save[i];
							Http_face_save.data[i].name = facesave.data[i].name;
							Http_face_save.count++;
							RK_MPI_SYS_SendMediaBuffer(RK_ID_VENC, 1, rga1_mb);//发送到 venc1 通道进行编码
						}
					}
				}
				if ( send == 1) {
					RK_MPI_SYS_SendMediaBuffer(RK_ID_RGA, 2, rga1_mb);
				}
			} else {
#if 1
				if(http_data_update == false) {
					//memset(&Http_face_save, 0, sizeof(Http_face_save));
					Http_face_save.count = 0;
					Http_face_save.IpAddress = LocalIpAddress;
					Http_face_save.MacAddress = LocalMacAddress;
					for (volatile int i = 0; i < facesave.count; i++){
						if(facesave.data[i].compare == 1) {
							//存储 http server 数据
							Http_face_save.data[i].Faceid = Faceid_save[i];
							Http_face_save.data[i].name = facesave.data[i].name;
							RK_MPI_SYS_SendMediaBuffer(RK_ID_VENC, 1, rga1_mb);//发送到 venc1 通道进行编码
							Http_face_save.count++;
						}
					}
				}
#endif
			}
			facesave.count = detectedFaces.faceNum;
			RK_MPI_MB_ReleaseBuffer(rga1_mb);
			rga1_ready = true;
		} else {
			usleep(1000);
		}
	}
}

/*******************************
描述  ：Rga2 通道回调
参数  ：获取到的 MEDIA_BUFFER
返回值：无
*******************************/
void rga2_packet_cb(volatile MEDIA_BUFFER mb) {
	int ret = 0;
	if (Quit)
		return;

	for (volatile int i = 0; i < facesave.count; i++){
		if ( facesave.data[i].reco == 1 ) {
			facesave.data[i].reco = 0;
			SingleDetectedFaces.faceRect.left = facesave.data[i].left;
			SingleDetectedFaces.faceRect.top = facesave.data[i].top;
			SingleDetectedFaces.faceRect.right = facesave.data[i].right;
			SingleDetectedFaces.faceRect.bottom = facesave.data[i].bottom;
			SingleDetectedFaces.faceOrient = facesave.data[i].faceOrient;

			ret = ASFFaceFeatureExtractEx(handle, &offscreen1, &SingleDetectedFaces, &feature1, ASF_REGISTER, 0);  
			if (ret != MOK && ret != MERR_FSDK_FACEFEATURE_MISSFACE ) {
				Msg_Debug(2,"ASFFaceFeatureExtractEx 1 failed: %x\n", ret);
			} else {
				fun_data.confidenceLevel = 0;
				sq->compare();
				if ( fun_data.confidenceLevel > 0.8 ) {
					Faceid_save[i] = fun_data.Faceid;
					facesave.data[i].name = fun_data.name;
					facesave.data[i].path = IMG_SAVE_PATH;
					facesave.data[i].path = facesave.data[i].path + facesave.data[i].name + ".jpg";
					facesave.data[i].img = cv::imread(facesave.data[i].path.c_str());
					cv::Point p1(0, 0);
					cv::Point p2(80, 80);
					cv::rectangle(facesave.data[i].img, p1, p2, colorArray[i], 5);
					facesave.data[i].compare = 1;

				}
			}
		}
	}
	RK_MPI_MB_ReleaseBuffer(mb);
}


/*******************************
描述  ：Venc 0 通道回调
参数  ：获取到的 MEDIA_BUFFER
返回值：无
*******************************/
MEDIA_BUFFER Venc0_mb;
bool Venc0_ready = true;
void venc_packet_cb0(MEDIA_BUFFER mb) {
	if(Venc0_ready == true) {
                Venc0_mb = RK_MPI_MB_Copy(mb, RK_TRUE);
        	Venc0_ready = false;
	}
	RK_MPI_MB_ReleaseBuffer(mb);

}

/*******************************
描述  ：Venc0 通道回调处理线程
参数  ：用户传参数，默认为 NULL
返回值：无
*******************************/
static void *_venc_packet_cb0(void *data) {
	while(Quit == false) {
		if (Venc0_ready == false) {
			Msg_Debug(5,"venc0 size %d\r\n",RK_MPI_MB_GetSize(Venc0_mb));
			rtspp[0]->write((unsigned char*)RK_MPI_MB_GetPtr(Venc0_mb), RK_MPI_MB_GetSize(Venc0_mb));
			RK_MPI_MB_ReleaseBuffer(Venc0_mb);
			Venc0_ready = true;
		} else {
			usleep(1000);
		}
	}
}

/*******************************
描述  ：Venc 1 通道回调
参数  ：获取到的 MEDIA_BUFFER
返回值：无
*******************************/
MEDIA_BUFFER Venc1_mb;
bool Venc1_ready = true;
void venc_packet_cb1(MEDIA_BUFFER mb) {
	if(Venc1_ready == true) {
                Venc1_mb = RK_MPI_MB_Copy(mb, RK_TRUE);
        	Venc1_ready = false;
	}
	RK_MPI_MB_ReleaseBuffer(mb);

}

/*******************************
描述  ：Venc1 通道回调处理线程
参数  ：用户传参数，默认为 NULL
返回值：无
*******************************/
static void *_venc_packet_cb1(void *data) {
	while(Quit == false) {
		if (Venc1_ready == false) {
			Msg_Debug(5,"venc1 size %d\r\n",RK_MPI_MB_GetSize(Venc1_mb));
			Base64 *base = new Base64();
			Http_face_save.img.clear();
			Http_face_save.img = base->Encode((const unsigned char*)RK_MPI_MB_GetPtr(Venc1_mb), RK_MPI_MB_GetSize(Venc1_mb));
			delete base;

			http_data_update = true;
			RK_MPI_MB_ReleaseBuffer(Venc1_mb);
			Venc1_ready = true;
		} else {
			usleep(1000);
		}
	}
}


static void *httpSend(void *data) {
        {
                CURL *curl;
                CURLcode res;

                /* In windows, this will init the winsock stuff */
                curl_global_init(CURL_GLOBAL_ALL);

                /* get a curl handle */
                curl = curl_easy_init();
                if(curl) {
                        /* First set the URL that is about to receive our POST. This URL can
                           just as well be a https:// URL if that is what should receive the
                           data. */
                        while(Quit == false){
                                if (http_data_update == true) {
                                        curl_easy_setopt(curl, CURLOPT_URL, httpUrl);
                                        /* Now specify the POST data */
                                        //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "num=12345");
					string cmd_count = "count=";
					string cmd_MacAddress = "MacAddress=";
					string cmd_IpAddress = "IpAddress=";
					string cmd_Faceid = "Faceid=";
					string cmd_name = "name=";
					string cmd_img = "img=";

					cmd_count.append(std::to_string(Http_face_save.count));
					cmd_count.append("&");
					cmd_MacAddress.append(Http_face_save.MacAddress);
					cmd_MacAddress.append("&");
					cmd_IpAddress.append(Http_face_save.IpAddress);
					cmd_IpAddress.append("&");
					if(Http_face_save.count == 0) {
						cmd_Faceid.append("null&");
						cmd_name.append("null&");
					} else {
						for(int i = 0; i < Http_face_save.count; i++) {
							cmd_Faceid.append(std::to_string(Http_face_save.data[i].Faceid));
							cmd_name.append(Http_face_save.data[i].name);
							if(i != Http_face_save.count - 1) {
								cmd_Faceid.append(",");
								cmd_name.append(",");
							} else {
								cmd_Faceid.append("&");
								cmd_name.append("&");
							}
						}
					}
					cmd_img.append(Http_face_save.img);

					cmd_count.append(cmd_MacAddress);
					cmd_count.append(cmd_IpAddress);
					cmd_count.append(cmd_Faceid);
					cmd_count.append(cmd_name);
					std::cout<<"http_data is "<<cmd_count.c_str()<<endl;
					cmd_count.append(cmd_img);

                                        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cmd_count.c_str());

                                        /* Perform the request, res will get the return code */
                                        res = curl_easy_perform(curl);
                                        /* Check for errors */
                                        if(res != CURLE_OK)
                                                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                                                                curl_easy_strerror(res));
                                        http_data_update = false;
                                } else {
                                        usleep(50);
                                }
                                //std::cout<<httpstr<<std::endl;
                        }
                        /* always cleanup */
                        curl_easy_cleanup(curl);
                }
                curl_global_cleanup();
                return 0;
        }
}

string GetLocalIP(char *ethName)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    string ipAddress;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
       if (ifa->ifa_addr == NULL)
           continue;

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET)
        {
            printf("interfac: %s, ip: %s\n", ifa->ifa_name, inet_ntoa(((struct sockaddr_in*)ifa->ifa_addr)->sin_addr));
	    if(0 == strcmp(ifa->ifa_name, ethName)) {
		ipAddress = inet_ntoa(((struct sockaddr_in*)ifa->ifa_addr)->sin_addr);
   		freeifaddrs(ifaddr);
		return ipAddress;
	    }
        }
   }

   freeifaddrs(ifaddr);
   return NULL;
}

string GetLocalMac(char *ethName)
{
    int sock_mac;

    struct ifreq ifr_mac;
    string eth_mac_addr;
    char mac_addr[30];

    sock_mac = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_mac == -1)
    {
        perror("create socket falise...mac/n");
        return "";
    }

    memset(&ifr_mac,0,sizeof(ifr_mac));
    strncpy(ifr_mac.ifr_name, ethName, sizeof(ifr_mac.ifr_name)-1);

    if( (ioctl( sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0)
    {
        printf("mac ioctl error/n");
        return "";
    }

    sprintf(mac_addr,"%02x%02x%02x%02x%02x%02x",
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);

    printf("local mac:%s /n",mac_addr);

    close(sock_mac);
    eth_mac_addr = mac_addr;
    return eth_mac_addr;
}

/* 主函数 */
int main(int argc, char **argv){

	RK_S32 s32CamId = 0;
#ifdef RKAIQ
	RK_BOOL bMultictx = RK_FALSE;
#endif
	if(argc != 2) {
		Msg_Error(0,"argc != 2\r\n Usage: %s rtsp_server_url\r\n",argv[0]);
		return -1;
	}
/* 加载字体 */
	ft2 = cv::freetype::createFreeType2();
	ft2->loadFontData("/usr/share/simhei.ttf", 0);
/* 初始化 log 系统 */
	log_pri_init();

/* 参数设置 */
	load_cfg(CFG_PATH);
	int ret = 0;
	LocalIpAddress = GetLocalIP((char *)ETH_NAME);
	LocalMacAddress = GetLocalMac((char *)ETH_NAME);
	Http_face_save.IpAddress = LocalIpAddress;
	Http_face_save.MacAddress = LocalMacAddress;
/* ffrtsp 初始化 */
	string urlmain = argv[1];
	urlmain += "main";
	string urlsub = argv[1];
	urlsub += "sub";
	rtspp[0] = new rtsppush(urlmain,H264M_1920_1080_25FPS);
	rtspp[0]->run();

	RK_U32 u32Width = 1920;
	cfg.session_cfg[0].u32Width = u32Width;
	RK_U32 u32Height = 1080;
	cfg.session_cfg[0].u32Height = u32Height;
	
	CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;

	cfg.session_cfg[0].video_type = enCodecType;
	
	cfg.session_cfg[0].video_num = 25;
	cfg.session_cfg[0].video_den = 1;
	cfg.session_cfg[0].video_fps = 25;

/* 初始化 RKMEDIA */
	RK_MPI_SYS_Init();

	int media = 3;
	char cmd[256];
	snprintf(
		cmd, sizeof(cmd),
		"media-ctl -d /dev/media%d -l '\"rkispp_input_image\":0->\"rkispp-subdev\":0[1]'",
		media);
	printf("cmd = %s\n", cmd);
	system(cmd);
	snprintf(cmd, sizeof(cmd),
		"media-ctl -d /dev/media%d --set-v4l2 '\"rkispp-subdev\":0[fmt:YUYV8_2X8/1920x1080]'",
		media);
	printf("cmd = %s\n", cmd);
	system(cmd);
	snprintf(cmd, sizeof(cmd),
		"media-ctl -d /dev/media%d --set-v4l2 '\"rkispp-subdev\":2[fmt:YUYV8_2X8/1920x1080]'",
		media);
	printf("cmd = %s\n", cmd);
	system(cmd);

	printf("#CameraIdx: %d\n\n", s32CamId);
	if (cfg.session_cfg[0].iqfilesPath) {
#ifdef RKAIQ
	printf("#Rkaiq XML DirPath: %s\n", cfg.session_cfg[0].iqfilesPath);
	printf("#bMultictx: %d\n\n", bMultictx);
	rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
	int fps = 25;
	SAMPLE_COMM_ISP_Init(s32CamId, hdr_mode, bMultictx, cfg.session_cfg[0].iqfilesPath);
	SAMPLE_COMM_ISP_Run(s32CamId);
	SAMPLE_COMM_ISP_SetFrameRate(s32CamId, fps);
#endif
  }

/* ARC 授权 */
	ret = printSDKInfo();
	if (ret) {
		printf("-------arc sdk init failed %d-------\n", ret);
		return -1;
	}
	ret = ArcBuffer();
	if (ret) {
		printf("-------arc buffer init failed %d-------\n", ret);
		return -1;
	}
/* 数据库初始化 */
	fun_data.handle = &handle;
	fun_data.data = &feature1;
	sq = new dsqlite(DB_PATH,compare,&fun_data);
	if( sq == NULL) {
		Msg_Error(0,"No sunch dbfile\r\n");
		return -1;
	}

	MPP_CHN_S ViChn;
	ViChn.enModId = RK_ID_VI;
	ViChn.s32DevId = 0;
	ViChn.s32ChnId = 0;
	/* Create VI */
	VI_CHN_ATTR_S vi_chn_attr;
	vi_chn_attr.pcVideoNode = "rkispp_scale0";
	vi_chn_attr.u32BufCnt = 4;
	vi_chn_attr.u32Width = u32Width;
	vi_chn_attr.u32Height = u32Height;
	vi_chn_attr.enPixFmt = IMAGE_TYPE_NV12;
	vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
	ret = RK_MPI_VI_SetChnAttr(s32CamId, ViChn.s32ChnId, &vi_chn_attr);
	ret |= RK_MPI_VI_EnableChn(s32CamId, ViChn.s32ChnId);
	if (ret) {
		printf("Create vi[1] failed! ret=%d\n", ret);
		return -1;
	}

	ViChn.enModId = RK_ID_VI;
	ViChn.s32DevId = 1;
	ViChn.s32ChnId = 1;
	memset(&vi_chn_attr, 0, sizeof(vi_chn_attr));
	vi_chn_attr.pcVideoNode = "/dev/video26";
	vi_chn_attr.u32BufCnt = 4;
	vi_chn_attr.u32Width = u32Width;
	vi_chn_attr.u32Height = u32Height;
	vi_chn_attr.enPixFmt = IMAGE_TYPE_NV12;
	vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
	ret = RK_MPI_VI_SetChnAttr(s32CamId, ViChn.s32ChnId, &vi_chn_attr);
	ret |= RK_MPI_VI_EnableChn(s32CamId, ViChn.s32ChnId);
	if (ret) {
		printf("Create vi[1] failed! ret=%d\n", ret);
		return -1;
	}

	ViChn.enModId = RK_ID_VI;
	ViChn.s32DevId = 2;
	ViChn.s32ChnId = 2;
	memset(&vi_chn_attr, 0, sizeof(vi_chn_attr));
	vi_chn_attr.pcVideoNode = "/dev/video27";
	vi_chn_attr.u32BufCnt = 4;
	vi_chn_attr.u32Width = SMALL_W;
	vi_chn_attr.u32Height = SMALL_H;
	vi_chn_attr.enPixFmt = IMAGE_TYPE_NV12;
	vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
	ret = RK_MPI_VI_SetChnAttr(s32CamId, ViChn.s32ChnId, &vi_chn_attr);
	ret |= RK_MPI_VI_EnableChn(s32CamId, ViChn.s32ChnId);
	if (ret) {
		printf("Create vi[2] failed! ret=%d\n", ret);
		return -1;
	}

	VP_CHN_ATTR_S vp_chn_attr;
	vp_chn_attr.pcVideoNode = "/dev/video25";
	vp_chn_attr.u32BufCnt = 3;
	vp_chn_attr.u32Width = 1920;
	vp_chn_attr.u32Height = 1080;
	vp_chn_attr.enPixFmt = IMAGE_TYPE_NV12;
	vp_chn_attr.enWorkMode = VP_WORK_MODE_NORMAL;
	ret = RK_MPI_VP_SetChnAttr(0, 0, &vp_chn_attr);
	ret |= RK_MPI_VP_EnableChn(0, 0);
	if (ret) {
		printf("Create vp[0] failed! ret=%d\n", ret);
		return -1;
	}

	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	stSrcChn.enModId = RK_ID_VI;
	stSrcChn.s32ChnId = 0;
	stDestChn.enModId = RK_ID_VP;
	stDestChn.s32ChnId = 0;
	ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (ret) {
		printf("Bind vi[0] to vp[0] failed! ret=%d\n", ret);
		return -1;
	}


/* RGA 初始化 */
	/* RGA0 */
	RGA_ATTR_S stRgaAttr;
	stRgaAttr.bEnBufPool = RK_TRUE;
	stRgaAttr.u16BufPoolCnt = 2;
	stRgaAttr.u16Rotaion = 0;
	stRgaAttr.stImgIn.u32X = 0;
	stRgaAttr.stImgIn.u32Y = 0;
	stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;
	stRgaAttr.stImgIn.u32Width = u32Width;
	stRgaAttr.stImgIn.u32Height = u32Height;
	stRgaAttr.stImgIn.u32HorStride = u32Width;
	stRgaAttr.stImgIn.u32VirStride = u32Height;
	stRgaAttr.stImgOut.u32X = 0;
	stRgaAttr.stImgOut.u32Y = 0;
	stRgaAttr.stImgOut.imgType = IMAGE_TYPE_RGB888;
	stRgaAttr.stImgOut.u32Width = u32Width;
	stRgaAttr.stImgOut.u32Height = u32Height;
	stRgaAttr.stImgOut.u32HorStride = u32Width;
	stRgaAttr.stImgOut.u32VirStride = u32Height;
	MPP_CHN_S RgaChn;
	RgaChn.enModId = RK_ID_RGA;
	RgaChn.s32DevId = 0;
	RgaChn.s32ChnId = 0;
	ret = RK_MPI_RGA_CreateChn(RgaChn.s32ChnId, &stRgaAttr);
	if (ret) {
		Msg_Error(0,"ERROR: Create stRgaChn[%d] falied! ret=%d\n", RgaChn.s32ChnId, ret);
	}

	ret = RK_MPI_SYS_RegisterOutCb(&RgaChn, rga0_packet_cb);
	if (ret) {
		Msg_Error(0,"ERROR: Create stRgaChn[%d] falied! ret=%d\n", RgaChn.s32ChnId, ret);
		return 0;
	}
	
	pthread_t rga0_thread;
	pthread_create(&rga0_thread, NULL, _rga0_packet_cb, NULL); 


	ViChn.s32ChnId = 1;
	ret = RK_MPI_SYS_Bind(&ViChn,&RgaChn);
	if (ret) {
		Msg_Error(0,"ERROR: Bind Vdec[%d] and rga[%d] failed! ret=%d\n", ViChn.s32ChnId, RgaChn.s32ChnId, ret);
		return -1;
	}


	/* RGA1 */
	stRgaAttr.bEnBufPool = RK_TRUE;
	stRgaAttr.u16BufPoolCnt = 2;
	stRgaAttr.u16Rotaion = 0;
	stRgaAttr.stImgIn.u32X = 0;
	stRgaAttr.stImgIn.u32Y = 0;
	stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;
	stRgaAttr.stImgIn.u32Width = SMALL_W;
	stRgaAttr.stImgIn.u32Height = SMALL_H;
	stRgaAttr.stImgIn.u32HorStride = SMALL_W;
	stRgaAttr.stImgIn.u32VirStride = SMALL_H;
	stRgaAttr.stImgOut.u32X = 0;
	stRgaAttr.stImgOut.u32Y = 0;
	stRgaAttr.stImgOut.imgType = IMAGE_TYPE_NV12;
	stRgaAttr.stImgOut.u32Width = SMALL_W;
	stRgaAttr.stImgOut.u32Height = SMALL_H;
	stRgaAttr.stImgOut.u32HorStride = SMALL_W;
	stRgaAttr.stImgOut.u32VirStride = SMALL_H;
	RgaChn.s32ChnId = 1;
	ret = RK_MPI_RGA_CreateChn(RgaChn.s32ChnId, &stRgaAttr);
	if (ret) {
		Msg_Error(0,"ERROR: Create stRgaChn[%d] falied! ret=%d\n", RgaChn.s32ChnId, ret);
	}
	
	ret = RK_MPI_SYS_RegisterOutCb(&RgaChn, rga1_packet_cb);
	if (ret) {
		Msg_Error(0,"ERROR: Create stRgaChn[%d] falied! ret=%d\n", RgaChn.s32ChnId, ret);
		return 0;
	}
	
	pthread_t rga1_thread;
	pthread_create(&rga1_thread, NULL, _rga1_packet_cb, NULL); 
	
	pthread_t display_thread;
	pthread_create(&display_thread, NULL, display_fun, NULL); 
	
	ViChn.s32ChnId = 2;
	ret = RK_MPI_SYS_Bind(&ViChn,&RgaChn);
	if (ret) {
		Msg_Error(0,"ERROR: Bind Vdec[%d] and rga[%d] failed! ret=%d\n",ViChn.s32ChnId, RgaChn.s32ChnId, ret);
		return -1;
	}

	/* RGA2 */
	stRgaAttr.stImgIn.u32Width = SMALL_W;
	stRgaAttr.stImgIn.u32Height = SMALL_H;
	stRgaAttr.stImgIn.u32HorStride = SMALL_W;
	stRgaAttr.stImgIn.u32VirStride = SMALL_H;
	RgaChn.s32ChnId = 2;
	ret = RK_MPI_RGA_CreateChn(RgaChn.s32ChnId, &stRgaAttr);
	if (ret) {
		Msg_Error(0,"ERROR: Create stRgaChn[%d] falied! ret=%d\n", RgaChn.s32ChnId, ret);
	}

	ret = RK_MPI_SYS_RegisterOutCb(&RgaChn, rga2_packet_cb);
	if (ret) {
		Msg_Error(0,"ERROR: Create stRgaChn[%d] falied! ret=%d\n", RgaChn.s32ChnId, ret);
		return 0;
	}

	/* RGA3 */
	stRgaAttr.stImgIn.u32Width = u32Width;
	stRgaAttr.stImgIn.u32Height = u32Height;
	stRgaAttr.stImgIn.u32HorStride = u32Width;
	stRgaAttr.stImgIn.u32VirStride = u32Height;
	stRgaAttr.stImgIn.imgType = IMAGE_TYPE_RGB888;
	stRgaAttr.stImgOut.u32Width = u32Width;
	stRgaAttr.stImgOut.u32Height = u32Height;
	stRgaAttr.stImgOut.u32HorStride = u32Width;
	stRgaAttr.stImgOut.u32VirStride = u32Height;

	RgaChn.s32ChnId = 3;
	ret = RK_MPI_RGA_CreateChn(RgaChn.s32ChnId, &stRgaAttr);
	if (ret) {
		Msg_Error(0,"ERROR: Create stRgaChn[%d] falied! ret=%d\n", RgaChn.s32ChnId, ret);
	}

/* 编码器初始化 */
	/* VENC 0 */
	VENC_CHN_ATTR_S venc_chn_attr; 
	memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
	switch (enCodecType) { 
		case RK_CODEC_TYPE_H265:
			venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H265;
			venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
			venc_chn_attr.stRcAttr.stH265Cbr.u32Gop = cfg.session_cfg[0].video_fps;
			venc_chn_attr.stRcAttr.stH265Cbr.u32BitRate = u32Width * u32Height * 4; 
			venc_chn_attr.stRcAttr.stH265Cbr.fr32DstFrameRateDen = cfg.session_cfg[0].video_den;
			venc_chn_attr.stRcAttr.stH265Cbr.fr32DstFrameRateNum = cfg.session_cfg[0].video_num;
			venc_chn_attr.stRcAttr.stH265Cbr.u32SrcFrameRateDen = cfg.session_cfg[0].video_den;
			venc_chn_attr.stRcAttr.stH265Cbr.u32SrcFrameRateNum = cfg.session_cfg[0].video_num;
			break;
		case RK_CODEC_TYPE_H264:
		default:
			venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H264;
			venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
			venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = cfg.session_cfg[0].video_fps;
			venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = u32Width * u32Height * 4;
			venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = cfg.session_cfg[0].video_den;
			venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = cfg.session_cfg[0].video_num;
			venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = cfg.session_cfg[0].video_den;
			venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = cfg.session_cfg[0].video_num;
			break;
	}
	venc_chn_attr.stVencAttr.imageType = IMAGE_TYPE_NV12;
	venc_chn_attr.stVencAttr.u32PicWidth = u32Width;
	venc_chn_attr.stVencAttr.u32PicHeight = u32Height;
	venc_chn_attr.stVencAttr.u32VirWidth = u32Width;
	venc_chn_attr.stVencAttr.u32VirHeight = u32Height;
	venc_chn_attr.stVencAttr.u32Profile = 77; //编码等级 77,是中级 66,基础等级，100,高级
	MPP_CHN_S VencChn;
	VencChn.enModId = RK_ID_VENC;
	VencChn.s32DevId = 0;
	VencChn.s32ChnId = 0;

	ret = RK_MPI_VENC_CreateChn(VencChn.s32ChnId, &venc_chn_attr);
	if (ret) {
		Msg_Error(0,"ERROR: create VENC[%d] error! ret=%d\n",VencChn.s32ChnId, ret);
		return -1;
	}
	
	ret = RK_MPI_SYS_RegisterOutCb(&VencChn, venc_packet_cb0);
	if (ret) {
		Msg_Error(0,"ERROR: register output callback for VENC[%d] error! ret=%d\n",VencChn.s32ChnId, ret);
		return -1;
	}
	
	RK_U32 u32Rotation = 0;
	VENC_ROTATION_E enRotation;
	switch (u32Rotation) {
		case 0:
			enRotation = VENC_ROTATION_0;
			break;
		case 90:
			enRotation = VENC_ROTATION_90;
			break;
		case 180:
			enRotation = VENC_ROTATION_180;
			break;
		case 270:
			enRotation = VENC_ROTATION_270;
			break;
		default:
			printf("Invalid rotation(%d), should be 0/90/180/270\n", u32Rotation);
			return -1;
	}

	MPP_CHN_S VencResPicChn;
	VencResPicChn.enModId = RK_ID_VENC;
	VencResPicChn.s32DevId = 0;
	VencResPicChn.s32ChnId = 1;
	VENC_CHN_ATTR_S venc_res_pic_chn_attr;
	memset(&venc_res_pic_chn_attr, 0, sizeof(venc_res_pic_chn_attr));
	venc_res_pic_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_JPEG;
	venc_res_pic_chn_attr.stVencAttr.imageType = IMAGE_TYPE_NV12;
	venc_res_pic_chn_attr.stVencAttr.u32PicWidth = SMALL_W;
	venc_res_pic_chn_attr.stVencAttr.u32PicHeight = SMALL_H;
	venc_res_pic_chn_attr.stVencAttr.u32VirWidth = SMALL_W;
	venc_res_pic_chn_attr.stVencAttr.u32VirHeight = SMALL_H;
	venc_res_pic_chn_attr.stVencAttr.stAttrJpege.u32ZoomWidth = SMALL_W;
	venc_res_pic_chn_attr.stVencAttr.stAttrJpege.u32ZoomHeight = SMALL_H;
	venc_res_pic_chn_attr.stVencAttr.stAttrJpege.u32ZoomVirWidth = SMALL_W;
	venc_res_pic_chn_attr.stVencAttr.stAttrJpege.u32ZoomVirHeight = SMALL_H;
	venc_res_pic_chn_attr.stVencAttr.enRotation = enRotation;
	ret = RK_MPI_VENC_CreateChn(VencResPicChn.s32ChnId, &venc_res_pic_chn_attr);
	if (ret) {
		printf("Create Venc failed! ret=%d\n", ret);
		return -1;
	}
	ret = RK_MPI_SYS_RegisterOutCb(&VencResPicChn, venc_packet_cb1);
	if (ret) {
		Msg_Error(0,"ERROR: register output callback for VENC[%d] error! ret=%d\n", VencResPicChn.s32ChnId, ret);
		return -1;
	}

	pthread_t Venc0_thread;
	pthread_create(&Venc0_thread, NULL, _venc_packet_cb0, NULL); 

	pthread_t Venc1_thread;
	pthread_create(&Venc1_thread, NULL, _venc_packet_cb1, NULL); 

	RgaChn.s32ChnId = 3;
	
	ret = RK_MPI_SYS_Bind(&RgaChn,&VencChn);
	if (ret) {
		Msg_Error(0,"ERROR: Bind Rga[3] and Venc[%d] failed! ret=%d\n",VencChn.s32ChnId ,ret);
		return -1;
	}


        httpUrl = (char *)malloc(sizeof(cfg.session_cfg[0].httpUrl));
        memcpy(httpUrl, cfg.session_cfg[0].httpUrl, sizeof(cfg.session_cfg[0].httpUrl));
        pthread_t http_thread;
        pthread_create(&http_thread, NULL, httpSend, NULL);//发送数据到 HTTP

	while (Quit == false) {
		usleep(3000 * 1000);
	}



/* 销毁线程 */
	rtspp[0]->exit();
	rtspp[1]->exit();
	pthread_join(rga1_thread, NULL);
	pthread_join(rga0_thread, NULL);
	pthread_join(Venc0_thread, NULL);
	pthread_join(http_thread, NULL);

/* 销毁通道 */
	RK_MPI_VENC_DestroyChn(0);
	RK_MPI_VENC_DestroyChn(1);
	RK_MPI_RGA_DestroyChn(0);
	RK_MPI_RGA_DestroyChn(1);
	RK_MPI_RGA_DestroyChn(2);
	RK_MPI_RGA_DestroyChn(3);
	RK_MPI_VDEC_DestroyChn(0);

/* 关闭数据库 */
	delete sq;

	return 0;
}
