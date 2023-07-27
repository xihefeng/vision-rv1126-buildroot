// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
#include "ffrtsp/ffrtsp.hh"

#include <rga/rga.h>
#include <rga/RockchipRga.h>

#include <iostream>
#include <sys/time.h>
#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"



#define INBUF_SIZE 1024 * 10

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 1280

static bool quit = false;
static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  quit = true;
}

typedef struct rga_demo_arg_s {
  RK_U32 target_x;
  RK_U32 target_y;
  RK_U32 target_width;
  RK_U32 target_height;
  RK_U32 chn_id;
} rga_demo_arg_t;

struct rga_data {
	void*	data;
	int 	fd;
	int	width;
	int	height;
	int	format;
	int	full_width;
	int 	full_height;
	int	x_offset;
	int	y_offset;
};

//测试数据，从开发者中心获取替换
#define APPID "9wXeKtfVtcj1WSw6eUNF1BPFkGqRTrLeXBmiV25SdNwa"
#define SDKKEY "Az7NGb7AybXSEs6rwiYFM97XP8Jvn8GR7c9uDaekSree"
#define ACTIVEKEY "0985-114G-W1C8-PXUZ"  //186.216


#define NSCALE 27
#define FACENUM	10  //支持检测的人脸数不超过10个

#define SafeFree(p) { if ((p)) free(p); (p) = NULL; }
#define SafeArrayDelete(p) { if ((p)) delete [] (p); (p) = NULL; } 
#define SafeDelete(p) { if ((p)) delete (p); (p) = NULL; }

//时间戳转换为日期格式
void timestampToTime(char* timeStamp, char* dateTime, int dateTimeSize)
{
    time_t tTimeStamp = atoll(timeStamp);
    struct tm* pTm = gmtime(&tTimeStamp);
    strftime(dateTime, dateTimeSize, "%Y-%m-%d %H:%M:%S", pTm);
}

void printSDKInfo()
{
    printf("\n************* ArcFace SDK Info *****************\n");
    MRESULT res = MOK;
    res = ASFOnlineActivation((char *)APPID, (char *)SDKKEY, (char *)ACTIVEKEY);
//    res = ASFOfflineActivation("85T1113W313MLCMD.dat");
    if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res)
        printf("ASFOnlineActivation failed: %x\n", res);
    else
        printf("ASFOnlineActivation sucess************: %x\n", res);

    //采集当前设备信息，用于离线激活
    char* deviceInfo = NULL;
    res = ASFGetActiveDeviceInfo(&deviceInfo);
    if (res != MOK) {
        printf("ASFGetActiveDeviceInfo failed: %x\n", res);
    } else {
        printf("ASFGetActiveDeviceInfo sucess: %s\n", deviceInfo);
    }

    //获取激活文件信息
    ASF_ActiveFileInfo activeFileInfo = { 0 };
    res = ASFGetActiveFileInfo(&activeFileInfo);
    if (res != MOK){
        printf("ASFGetActiveFileInfo failed: %x\n", res);
    } else {
        //这里仅获取了有效期时间，还需要其他信息直接打印即可
        char startDateTime[32];
        timestampToTime(activeFileInfo.startTime, startDateTime, 32);
        printf("startTime: %s\n", startDateTime);
        char endDateTime[32];
        timestampToTime(activeFileInfo.endTime, endDateTime, 32);
        printf("endTime: %s\n", endDateTime);
    }

    //SDK版本信息
    const ASF_VERSION version = ASFGetVersion();
    printf("\nVersion:%s\n", version.Version);
    printf("BuildDate:%s\n", version.BuildDate);
    printf("CopyRight:%s\n", version.CopyRight);
}



static RK_CHAR optstr[] = "?::i:o:f:w:h:t:l:";
static void print_usage() {
  printf("usage example: rkmedia_vdec_test -w 720 -h 480 -i /userdata/out.jpeg "
         "-f 0 -t JPEG.\n");
  printf("\t-w: DisplayWidth, Default: 720\n");
  printf("\t-h: DisplayHeight, Default: 1280\n");
  printf("\t-i: InputFilePath, Default: NULL\n");
  printf("\t-f: 1:hardware; 0:software. Default:hardware\n");
  printf("\t-l: LoopSwitch; 0:NoLoop; 1:Loop. Default: 0.\n");
  printf("\t-t: codec type, Default H264, support H264/H265/JPEG.\n");
}


/* RGA转换 */
void rga_copy(rga_data rga_src,rga_data rga_dst,RockchipRga * rkRga,bool rotation){

	int ret =0;

	rga_info_t src;
	rga_info_t dst;
		
    	memset(&src, 0, sizeof(rga_info_t));
    	src.mmuFlag = 1;
	if(rga_src.fd != -1)
    		src.fd = rga_src.fd;
	else
		src.virAddr = rga_src.data;
		
    	memset(&dst, 0, sizeof(rga_info_t));
    	dst.mmuFlag = 1;
	if(rga_dst.fd != -1)
		dst.fd = rga_dst.fd;
	else
		dst.virAddr = rga_dst.data;

        /********** set the rect_info **********/
        rga_set_rect(&src.rect, rga_src.x_offset,rga_src.y_offset,rga_src.width,rga_src.height,rga_src.full_width/*stride*/,rga_src.full_height,rga_src.format);
        rga_set_rect(&dst.rect, rga_dst.x_offset,rga_dst.y_offset,rga_dst.width,rga_dst.height,rga_dst.full_width/*stride*/,rga_dst.full_height,rga_dst.format);
		
		/************ set the rga_mod ,rotation\composition\scale\copy .... **********/
		
		/********** call rga_Interface **********/
	if(rotation == true)
            src.rotation = HAL_TRANSFORM_ROT_90;
	ret = rkRga->RkRgaBlit(&src, &dst, NULL);
}


//#define SAVE_FILE

int FFRKMedia_Vdec_Send(u_int8_t* framebuff,unsigned framesize,bool * pquit,int cur_chn)
{
    MEDIA_BUFFER mb = RK_MPI_MB_CreateBuffer(framesize, RK_FALSE, 0);
    RK_MPI_MB_SetSize(mb, framesize);
    memcpy(RK_MPI_MB_GetPtr(mb) ,framebuff , framesize);
#ifdef SAVE_FILE
    FILE *fp = fopen("test_rtsp", "a+b");
    fwrite(RK_MPI_MB_GetPtr(mb), framesize, 1, fp);
    fclose(fp);
    fp = NULL;
#else
    RK_MPI_MB_SetSize(mb, framesize);
    RK_MPI_SYS_SendMediaBuffer(RK_ID_VDEC, cur_chn, mb);
#endif
    RK_MPI_MB_ReleaseBuffer(mb);
    if (quit)
    	*pquit = true;
}


MEDIA_BUFFER dst_mb = NULL;

static void *SendMediaBuffer(void *arg) {
	while(1){
        RK_MPI_SYS_SendMediaBuffer(RK_ID_VO, 0, dst_mb);
	usleep(33 * 1000);
	}
}

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




ASF_MultiFaceInfo detectedFaces[5] = {0};

static void *ArcBuffer(void *arg) {
	printf("#Start %s thread, arg:%p\n", __func__, arg);
	rga_demo_arg_t *crop_arg = (rga_demo_arg_t *)arg;
	int ret;
	MEDIA_BUFFER src_mb = NULL;

        ASVLOFFSCREEN offscreen1 = {0};
	ASF_SingleFaceInfo SingleDetectedFaces = {0};
	ASF_FaceFeature feature1 = {0};
	ASF_FaceFeature copyfeature1 = {0};

	MHandle handle = NULL;
	//初始化引擎
	MInt32 initMask = ASF_FACE_DETECT | ASF_FACERECOGNITION ;



	ret = ASFInitEngine(ASF_DETECT_MODE_VIDEO, ASF_OP_0_ONLY,
			NSCALE, FACENUM, initMask, &handle);
	if (ret != MOK)
		printf("ASFInitEngine failed: %x\n", ret);
	else
		printf("ASFInitEngine sucess: %x\n", ret);


	while (1) {
		src_mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, crop_arg->chn_id * 2 + 1, -1);
		if ((MUInt8*)RK_MPI_MB_GetPtr(src_mb) == NULL )
			continue;
        	ColorSpaceConversion(640, 480, ASVL_PAF_NV12, (MUInt8*)RK_MPI_MB_GetPtr(src_mb), offscreen1);
		ret = ASFDetectFacesEx(handle, &offscreen1, &detectedFaces[crop_arg->chn_id]);
		if (ret != MOK || detectedFaces[crop_arg->chn_id].faceNum < 1) {
			//printf("ASFDetectFaces 1 failed \n");
		} else {
			for (int i = 0; i < detectedFaces[crop_arg->chn_id].faceNum; i++){
				SingleDetectedFaces.faceRect.left = detectedFaces[crop_arg->chn_id].faceRect[i].left;
				SingleDetectedFaces.faceRect.top = detectedFaces[crop_arg->chn_id].faceRect[i].top;
				SingleDetectedFaces.faceRect.right = detectedFaces[crop_arg->chn_id].faceRect[i].right;
				SingleDetectedFaces.faceRect.bottom = detectedFaces[crop_arg->chn_id].faceRect[i].bottom;
				SingleDetectedFaces.faceOrient = detectedFaces[crop_arg->chn_id].faceOrient[i];
			}
			//printf("left:%d , top:%d , right:%d , bottom:%d.\r\n",detectedFaces[0].faceRect[0].left,detectedFaces[0].faceRect[0].top,detectedFaces[0].faceRect[0].right,detectedFaces[0].faceRect[0].bottom);
		}

		RK_MPI_MB_ReleaseBuffer(src_mb);

	}
	return NULL;

}

static void *GetMediaBuffer(void *arg) {
	printf("#Start %s thread, arg:%p\n", __func__, arg);
	rga_demo_arg_t *crop_arg = (rga_demo_arg_t *)arg;
	int ret;
	MEDIA_BUFFER src_mb = NULL;
	RockchipRga rkRga;
	rkRga.RkRgaInit();
	rga_data rga_src;
	rga_data rga_dst;
	rga_src.fd = -1;
	float x_rate = (float)1920 / 640;
	float y_rate = (float)1088 / 480;

	while (1) {
		src_mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, crop_arg->chn_id * 2 + 0, -1);
		for (int i = 0; i < detectedFaces[crop_arg->chn_id].faceNum; i++){
			int x = detectedFaces[crop_arg->chn_id].faceRect[i].left * x_rate;
			int y = detectedFaces[crop_arg->chn_id].faceRect[i].top * y_rate;
			int w = (detectedFaces[crop_arg->chn_id].faceRect[i].right -
					detectedFaces[crop_arg->chn_id].faceRect[i].left) *
				x_rate;
			int h = (detectedFaces[crop_arg->chn_id].faceRect[i].bottom -
					detectedFaces[crop_arg->chn_id].faceRect[i].top) *
				y_rate;
			if (x < 0)
				x = 0;
			if (y < 0)
				y = 0;
			while ((uint32_t)(x + w) >= 1920) {
				w -= 16;
			}
			while ((uint32_t)(y + h) >= 1088) {
				h -= 16;
			}
			nv12_border((char *)RK_MPI_MB_GetPtr(src_mb),
					1920,
					1088, x, y,w , h, 0, 0,
					255);//这里通过YUV像素点数据直接偏移覆盖来画框，没有使用 OPENCV
		}               
		//构建原始数据
		rga_src.fd = RK_MPI_MB_GetFD(src_mb);
		if(rga_src.fd == -1)
			continue;

	
		rga_src.x_offset = 0;
		rga_src.y_offset = 0;
		rga_src.format = RK_FORMAT_YCrCb_420_SP;
		rga_src.full_width = 1920;
		rga_src.full_height = 1088;
		rga_src.width = 1920;
		rga_src.height = 1088;
		//构建目标数据

		rga_dst.fd = RK_MPI_MB_GetFD(dst_mb);
		rga_dst.format = RK_FORMAT_YCrCb_420_SP;
		rga_dst.full_width = crop_arg->target_width;
		rga_dst.full_height = crop_arg->target_height;
		rga_dst.width = crop_arg->target_width / 2;
		rga_dst.height = crop_arg->target_height / 2;
		rga_dst.x_offset = crop_arg->target_x;
		rga_dst.y_offset = crop_arg->target_y;	

		rga_copy(rga_src,rga_dst,&rkRga,true);
                rga_src.fd = -1;


		RK_MPI_MB_ReleaseBuffer(src_mb);

	}
	return NULL;
}

static void SAMPLE_COMMON_RGA_Start(int vdec_chn_id) {
  int ret = 0;
  RGA_ATTR_S stRgaAttr;
  stRgaAttr.bEnBufPool = RK_TRUE;
  stRgaAttr.u16BufPoolCnt = 2;
  stRgaAttr.u16Rotaion = 0;
  stRgaAttr.stImgIn.u32X = 0;
  stRgaAttr.stImgIn.u32Y = 0;
  stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;
  stRgaAttr.stImgIn.u32Width = 1920;
  stRgaAttr.stImgIn.u32Height = 1088;
  stRgaAttr.stImgIn.u32HorStride = 1920;
  stRgaAttr.stImgIn.u32VirStride = 1088;
  stRgaAttr.stImgOut.u32X = 0;
  stRgaAttr.stImgOut.u32Y = 0;
  stRgaAttr.stImgOut.imgType = IMAGE_TYPE_NV12;
  stRgaAttr.stImgOut.u32Width = 1920;
  stRgaAttr.stImgOut.u32Height = 1088;
  stRgaAttr.stImgOut.u32HorStride = 1920;
  stRgaAttr.stImgOut.u32VirStride = 1088;
  ret = RK_MPI_RGA_CreateChn(vdec_chn_id * 2 + 0, &stRgaAttr);
  if (ret) {
    printf("ERROR: Create rga[0] falied! ret=%d\n", ret);
  }

  stRgaAttr.stImgOut.u32Width = 640;
  stRgaAttr.stImgOut.u32Height = 480;
  stRgaAttr.stImgOut.u32HorStride = 640;
  stRgaAttr.stImgOut.u32VirStride = 480;

  ret = RK_MPI_RGA_CreateChn(vdec_chn_id * 2 + 1, &stRgaAttr);
  if (ret) {
    printf("ERROR: Create rga[1] falied! ret=%d\n", ret);
  }
}


int main(int argc, char *argv[]) {
  RK_U32 u32DispWidth = 720;
  RK_U32 u32DispHeight = 1280;
  RK_BOOL bIsHardware = RK_TRUE;
  RK_U32 u32Loop = 0;
  CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;
  int c, ret;
  
  // 本 DEMO 不需要使用 MIPI 摄像头，所以不需要初始化 RKAIQ
  printf("#Display wxh: %dx%d\n", u32DispWidth, u32DispHeight);
  printf("#Decode Mode: %s\n", bIsHardware ? "Hardware" : "Software");
  printf("#Loop Cnt: %d\n", u32Loop);

  //signal(SIGINT, sigterm_handler);

  RK_MPI_SYS_Init();//初始化 MPI 系统

  // VDEC
  // 这里可以学习解码器数据结构配置
  VDEC_CHN_ATTR_S stVdecAttr;
  stVdecAttr.enCodecType = enCodecType; //解码格式
  stVdecAttr.enMode = VIDEO_MODE_FRAME; //解码输入模式 帧/流 
  if (bIsHardware) { //支持软/硬解码，这里通过输入参数判断
    if (stVdecAttr.enCodecType == RK_CODEC_TYPE_JPEG) {//硬
      stVdecAttr.enMode = VIDEO_MODE_FRAME;//帧
    } else {
      stVdecAttr.enMode = VIDEO_MODE_STREAM;//流
    }
    stVdecAttr.enDecodecMode = VIDEO_DECODEC_HADRWARE;
  } else {//软
    stVdecAttr.enMode = VIDEO_MODE_FRAME;
    stVdecAttr.enDecodecMode = VIDEO_DECODEC_SOFTWARE;
  }
    
  VO_CHN_ATTR_S stVoAttr = {0};
  memset(&stVoAttr, 0, sizeof(stVoAttr));
  stVoAttr.pcDevNode = "/dev/dri/card0";
  stVoAttr.emPlaneType = VO_PLANE_OVERLAY;
  stVoAttr.enImgType = IMAGE_TYPE_NV12;
  stVoAttr.u16Zpos = 1;
  stVoAttr.u32Width = SCREEN_WIDTH;
  stVoAttr.u32Height = SCREEN_HEIGHT;
  stVoAttr.stImgRect.s32X = 0;
  stVoAttr.stImgRect.s32Y = 0;
  stVoAttr.stImgRect.u32Width = SCREEN_WIDTH;
  stVoAttr.stImgRect.u32Height = SCREEN_HEIGHT;
  stVoAttr.stDispRect.s32X = 0;
  stVoAttr.stDispRect.s32Y = 0;
  stVoAttr.stDispRect.u32Width = SCREEN_WIDTH;
  stVoAttr.stDispRect.u32Height = SCREEN_HEIGHT;
  ret = RK_MPI_VO_CreateChn(0, &stVoAttr);
  if (ret) {
    printf("Create VO[0] failed! ret=%d\n", ret);
    quit = false;
    return -1;
  }

  MPP_CHN_S VdecChn,VoChn;
  VdecChn.enModId = RK_ID_VDEC;
  VdecChn.s32DevId = 0;

  rga_demo_arg_t demo_arg[20];
  pthread_t read_thread[20];
  pthread_t arc_thread[20];
  
  int i = 0;

  struct FFRTSPGet ffrtsp_get;
  ffrtsp_get.callback = FFRKMedia_Vdec_Send;
  ffrtsp_get.count = argc - 1;

  MB_IMAGE_INFO_S stImageInfo = {
	  SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH,
	  SCREEN_HEIGHT, IMAGE_TYPE_NV12};
  dst_mb = RK_MPI_MB_CreateImageBuffer(&stImageInfo, RK_TRUE, 0);
  if (!dst_mb) {
	  printf("ERROR: RK_MPI_MB_CreateImageBuffer get null buffer!\n");
  }

  printf("\n************* Face Recognition *****************\n");
  MRESULT res = MOK;


  //激活与打印一些信息
  printSDKInfo();


  MPP_CHN_S RgaChn;
  RgaChn.enModId = RK_ID_RGA;
  RgaChn.s32DevId = 0;
  RgaChn.s32ChnId = 0;




for(i = 1; i < argc; i++) {

  VdecChn.s32ChnId = i - 1;
  int x = (i - 1) / 2;
  int y = (i - 1) % 2;
  demo_arg[i - 1].target_x = (SCREEN_WIDTH / 2) * x;
  demo_arg[i - 1].target_y = (SCREEN_HEIGHT / 2) * y;
  demo_arg[i - 1].target_width = SCREEN_WIDTH;
  demo_arg[i - 1].target_height = SCREEN_HEIGHT;
  demo_arg[i - 1].chn_id = VdecChn.s32ChnId;

  ret = RK_MPI_VDEC_CreateChn(VdecChn.s32ChnId, &stVdecAttr);//创建解码器通道 
  if (ret) {
    printf("Create Vdec[0] failed! ret=%d\n", ret);
    return -1;
  }
  

  SAMPLE_COMMON_RGA_Start(VdecChn.s32ChnId);
 
  RgaChn.s32ChnId = VdecChn.s32ChnId * 2 + 0;
  ret = RK_MPI_SYS_Bind(&VdecChn,&RgaChn);
  if (ret) {
	  printf("ERROR: Bind Vdec[0] and rga[0] failed! ret=%d\n", ret);
	  return -1;
  }
  
  RgaChn.s32ChnId = VdecChn.s32ChnId * 2 + 1;
  ret = RK_MPI_SYS_Bind(&VdecChn,&RgaChn);
  if (ret) {
	  printf("ERROR: Bind Vdec[0] and rga[1] failed! ret=%d\n", ret);
	  return -1;
  }


  ffrtsp_get.ffrtsp_get_info[i - 1].url = argv[i];

  pthread_create(&read_thread[i - 1], NULL, GetMediaBuffer, &demo_arg[i - 1]);
  pthread_create(&arc_thread[i - 1], NULL, ArcBuffer, &demo_arg[i - 1]);

}
  pthread_t vo_thread;
  pthread_create(&vo_thread, NULL, SendMediaBuffer, NULL);
  
  ffrtspGet(ffrtsp_get);

  quit = true;

  RK_MPI_VDEC_DestroyChn(0);

  return 0;
}
