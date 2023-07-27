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
/*******************************
 全局定义
*******************************/

rtsppush * rtspp[2];

#define CFG_PATH "/userdata/rkmedia_rtspget_arc_rtsp_test_sdk/ffarc_rv1126/ffarc.cfg"
#define DB_PATH "/userdata/rkmedia_rtspget_arc_rtsp_test_sdk/ffarc_rv1126/firefly.db"
#define IMG_SAVE_PATH "/userdata/rkmedia_rtspget_arc_rtsp_test_sdk/ffarc_rv1126/image_save/"
#define FONT_DATA_PATH "/userdata/rkmedia_rtspget_arc_rtsp_test_sdk/ffarc_rv1126/simhei.ttf" 
bool Quit = false;
//测试数据，联系商务领取
#define CELING_2_POWER(x,a)  (((x) + ((a)-1)) & (~((a) - 1))) 
#define FLOOR_2_POWER(x,a)   ((x) & (~((a) - 1 )))
#define NSCALE 27
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
						"appid=%s sdkkey=%s activekey=%s",
						cfg.session_cfg[count].appid,
						cfg.session_cfg[count].sdkkey,
						cfg.session_cfg[count].activekey) == 0 ) {
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
	RK_MPI_MB_BeginCPUAccess(mb, RK_FALSE);
	RK_MPI_MB_SetSize(mb, framesize);
	memcpy(RK_MPI_MB_GetPtr(mb) ,framebuff , framesize);
	RK_MPI_MB_EndCPUAccess(mb, RK_FALSE);
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
					Msg_Debug(4,"Arc detect index [%d], name = %s\r\n", i,facesave.data[i].name.c_str());
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
			RK_MPI_MB_BeginCPUAccess(rga1_mb, RK_FALSE);
			ColorSpaceConversion(SMALL_W, SMALL_H, ASVL_PAF_NV12, (MUInt8*)RK_MPI_MB_GetPtr(rga1_mb), offscreen1);
			RK_MPI_MB_EndCPUAccess(rga1_mb, RK_FALSE);
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
						send = 1;
					/* 连续识别 10 次，如果都不成功就不再继续识别了 */
					} else if ( facesave.data[i].use % cfg.session_cfg[0].video_fps < 3 && 
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
				if ( send == 1) {
					RK_MPI_SYS_SendMediaBuffer(RK_ID_RGA, 2, rga1_mb);
				}
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
void rga2_packet_cb(MEDIA_BUFFER mb) {
	int ret = 0;
	if (Quit)
		return;

	for (int i = 0; i < facesave.count; i++){
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
			RK_MPI_MB_BeginCPUAccess(Venc0_mb, RK_FALSE);
			rtspp[0]->write((unsigned char*)RK_MPI_MB_GetPtr(Venc0_mb), RK_MPI_MB_GetSize(Venc0_mb));
			RK_MPI_MB_EndCPUAccess(Venc0_mb, RK_FALSE);
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
			RK_MPI_MB_BeginCPUAccess(Venc1_mb, RK_FALSE);
			rtspp[1]->write((unsigned char*)RK_MPI_MB_GetPtr(Venc1_mb), RK_MPI_MB_GetSize(Venc1_mb));
			RK_MPI_MB_EndCPUAccess(Venc1_mb, RK_FALSE);
			RK_MPI_MB_ReleaseBuffer(Venc1_mb);
			Venc1_ready = true;
		} else {
			usleep(1000);
		}
	}
}

/* 主函数 */
int main(int argc, char **argv){

	if(argc != 3) {
		Msg_Error(0,"argc != 3\r\n Usage: %s ipc_camera_url rtsp_server_url\r\n",argv[0]);
		return -1;
	}
/* 加载字体 */
	ft2 = cv::freetype::createFreeType2();
	ft2->loadFontData(FONT_DATA_PATH, 0);
/* 初始化 log 系统 */
	log_pri_init();

/* 参数设置 */
	load_cfg(CFG_PATH);
	int ret = 0;
	
/* ffrtsp 初始化 */
	string urlmain = argv[2];
	urlmain += "main";
	string urlsub = argv[2];
	urlsub += "sub";
	rtspget rtspg(argv[1],FFRTSP_Get,NULL);
	rtspp[0] = new rtsppush(urlmain,H264M_1920_1080_25FPS);
	rtspp[1] = new rtsppush(urlsub,H264M_640_360_25FPS);
	//rtspp[1] = new rtsppush(urlsub,H264M_1920_1080_25FPS);
	rtspp[0]->run();
	rtspp[1]->run();
	ret = rtspg.init();
	if(ret){
		Msg_Error(0,"Open IPC Camera err\r\n");
		return -1;
	}

	RK_U32 u32Width = rtspg.w;
	cfg.session_cfg[0].u32Width = u32Width;
	RK_U32 u32Height = rtspg.h;
	cfg.session_cfg[0].u32Height = u32Height;
	
	CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;
	if( rtspg.codec_type == "h264" ) {
		enCodecType = RK_CODEC_TYPE_H264;
	} else if (rtspg.codec_type == "h265" ) {
		enCodecType = RK_CODEC_TYPE_H265;
	} else {
		Msg_Error(5,"Codec typec not support \r\n");
		return -1;
	}
	cfg.session_cfg[0].video_type = enCodecType;
	
	cfg.session_cfg[0].video_num = rtspg.num;
	cfg.session_cfg[0].video_den = rtspg.den;
	cfg.session_cfg[0].video_fps = rtspg.num / rtspg.den;
	if (rtspg.num % rtspg.den > 0)
		cfg.session_cfg[0].video_fps += 1;


/* 初始化 RKMEDIA */
	RK_MPI_SYS_Init();

/* ARC 授权 */
	ret = printSDKInfo();
	if (ret) {
		return -1;
	}
	ret = ArcBuffer();
	if (ret) {
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

/* 解码器初始化 */
	VDEC_CHN_ATTR_S stVdecAttr;
	stVdecAttr.enCodecType = enCodecType;
	stVdecAttr.enMode = VIDEO_MODE_STREAM;
	stVdecAttr.enDecodecMode = VIDEO_DECODEC_HADRWARE;
	MPP_CHN_S VdecChn;
	VdecChn.enModId = RK_ID_VDEC;
	VdecChn.s32DevId = 0;
	VdecChn.s32ChnId = 0;
	ret = RK_MPI_VDEC_CreateChn(VdecChn.s32ChnId, &stVdecAttr);//创建解码器通道 
	if (ret) {
		Msg_Error(0,"Create Vdec[%d] failed! ret=%d\n", VdecChn.s32ChnId,ret);
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
	stRgaAttr.stImgOut.u32VirStride = CELING_2_POWER(u32Height,16);
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


	ret = RK_MPI_SYS_Bind(&VdecChn,&RgaChn);
	if (ret) {
		Msg_Error(0,"ERROR: Bind Vdec[%d] and rga[%d] failed! ret=%d\n", VdecChn.s32ChnId, RgaChn.s32ChnId, ret);
		return -1;
	}


	/* RGA1 */
	stRgaAttr.stImgOut.imgType = IMAGE_TYPE_NV12;
	stRgaAttr.stImgOut.u32Width = SMALL_W;
	stRgaAttr.stImgOut.u32Height = SMALL_H;
	stRgaAttr.stImgOut.u32HorStride = SMALL_W;
	stRgaAttr.stImgOut.u32VirStride = CELING_2_POWER(SMALL_H,16);
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
	
	ret = RK_MPI_SYS_Bind(&VdecChn,&RgaChn);
	if (ret) {
		Msg_Error(0,"ERROR: Bind Vdec[%d] and rga[%d] failed! ret=%d\n",VdecChn.s32ChnId, RgaChn.s32ChnId, ret);
		return -1;
	}

	/* RGA2 */
	stRgaAttr.stImgIn.u32Width = SMALL_W;
	stRgaAttr.stImgIn.u32Height = SMALL_H;
	stRgaAttr.stImgIn.u32HorStride = SMALL_W;
	stRgaAttr.stImgIn.u32VirStride = CELING_2_POWER(SMALL_H,16);
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
	stRgaAttr.stImgOut.u32VirStride = CELING_2_POWER(u32Height,16);

	RgaChn.s32ChnId = 3;
	ret = RK_MPI_RGA_CreateChn(RgaChn.s32ChnId, &stRgaAttr);
	if (ret) {
		Msg_Error(0,"ERROR: Create stRgaChn[%d] falied! ret=%d\n", RgaChn.s32ChnId, ret);
	}

	/* RGA4 */
	stRgaAttr.stImgIn.u32Width = u32Width;
	stRgaAttr.stImgIn.u32Height = u32Height;
	stRgaAttr.stImgIn.u32HorStride = u32Width;
	stRgaAttr.stImgIn.u32VirStride = u32Height;
	stRgaAttr.stImgIn.imgType = IMAGE_TYPE_RGB888;
	stRgaAttr.stImgOut.u32Width = SMALL_W;
	stRgaAttr.stImgOut.u32Height = SMALL_H;
	stRgaAttr.stImgOut.u32HorStride = SMALL_W;
	stRgaAttr.stImgOut.u32VirStride = CELING_2_POWER(SMALL_H,16);
	RgaChn.s32ChnId = 4;

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
	venc_chn_attr.stVencAttr.u32VirHeight = CELING_2_POWER(u32Height,16);
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
	
	pthread_t Venc0_thread;
	pthread_create(&Venc0_thread, NULL, _venc_packet_cb0, NULL); 

	RgaChn.s32ChnId = 3;
	
	ret = RK_MPI_SYS_Bind(&RgaChn,&VencChn);
	if (ret) {
		Msg_Error(0,"ERROR: Bind Rga[3] and Venc[%d] failed! ret=%d\n",VencChn.s32ChnId ,ret);
		return -1;
	}
	
	/* VENC 1 */
	venc_chn_attr.stVencAttr.u32PicWidth = SMALL_W;
	venc_chn_attr.stVencAttr.u32PicHeight = SMALL_H;
	venc_chn_attr.stVencAttr.u32VirWidth = SMALL_W;
	venc_chn_attr.stVencAttr.u32VirHeight = CELING_2_POWER(SMALL_H,16);
	venc_chn_attr.stRcAttr.stH265Cbr.u32BitRate = SMALL_W * SMALL_H * 4; 
	venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = SMALL_W * SMALL_H * 4;
	VencChn.s32DevId = 0;
	VencChn.s32ChnId = 1;

	ret = RK_MPI_VENC_CreateChn(VencChn.s32ChnId, &venc_chn_attr);
	if (ret) {
		Msg_Error(0,"ERROR: create VENC[%d] error! ret=%d\n",VencChn.s32ChnId, ret);
		return -1;
	}

	ret = RK_MPI_SYS_RegisterOutCb(&VencChn, venc_packet_cb1);
	if (ret) {
		Msg_Error(0,"ERROR: register output callback for VENC[%d] error! ret=%d\n",VencChn.s32ChnId, ret);
		return -1;
	}
	
	pthread_t Venc1_thread;
	pthread_create(&Venc1_thread, NULL, _venc_packet_cb1, NULL); 

	RgaChn.s32ChnId = 4;
	ret = RK_MPI_SYS_Bind(&RgaChn,&VencChn);
	if (ret) {
		Msg_Error(0,"ERROR: Bind Rga[%d] and Venc[%d] failed! ret=%d\n",RgaChn.s32ChnId ,VencChn.s32ChnId, ret);
		return -1;
	}


/* 开始取流 */
	rtspg.run();

	while (Quit == false) {
		usleep(3000 * 1000);
	}



/* 销毁线程 */

	rtspg.exit();
	rtspp[0]->exit();
	rtspp[1]->exit();
	pthread_join(rga1_thread, NULL);
	pthread_join(rga0_thread, NULL);
	pthread_join(Venc1_thread, NULL);
	pthread_join(Venc0_thread, NULL);

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
