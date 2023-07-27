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

#define SCREEN_WIDTH 4096
#define SCREEN_HEIGHT 1440
//2K 2560x1440
//4K 3840×2160 

#define IMG_WIDTH 2560
#define IMG_HEIGHT 1440
#define CELING_2_POWER(x,a)  (((x) + ((a)-1)) & (~((a) - 1)))

//#define SAVE_FILE

#ifdef  SAVE_FILE
FILE *fp = NULL;
#define SAVE_FILE_NAME "/userdata/test_rtsp.264"
#endif
struct RTSP_PUSH_INFO ffrtsp_push[MAXFFRTSPChn];


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
void rga_copy(rga_data rga_src,rga_data rga_dst,RockchipRga * rkRga){

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

	ret = rkRga->RkRgaBlit(&src, &dst, NULL);
}



int FFRKMedia_Vdec_Send(u_int8_t* framebuff,unsigned framesize,bool * pquit,int cur_chn)
{
    MEDIA_BUFFER mb = RK_MPI_MB_CreateBuffer(framesize, RK_FALSE, 0);
    RK_MPI_MB_SetSize(mb, framesize);
    memcpy(RK_MPI_MB_GetPtr(mb) ,framebuff , framesize);
    RK_MPI_MB_SetSize(mb, framesize);
    RK_MPI_SYS_SendMediaBuffer(RK_ID_VDEC, cur_chn, mb);
    RK_MPI_MB_ReleaseBuffer(mb);
    if (quit)
    	*pquit = true;
}


MEDIA_BUFFER dst_mb = NULL;

static void *SendMediaBuffer(void *arg) {
	while(!quit){
        	RK_MPI_SYS_SendMediaBuffer(RK_ID_RGA, 0, dst_mb);
        	//RK_MPI_SYS_SendMediaBuffer(RK_ID_VO, 0, dst_mb);
		usleep(30 * 1000);
	}
}

static void *GetVenBuffer(void *arg) {
	while(!quit){
#ifdef SAVE_FILE
		MEDIA_BUFFER buffer;
		// send video buffer
		buffer = RK_MPI_SYS_GetMediaBuffer(
				RK_ID_VENC, 0, -1);
    		fwrite(RK_MPI_MB_GetPtr(buffer), RK_MPI_MB_GetSize(buffer), 1, fp);
		RK_MPI_MB_ReleaseBuffer(buffer);
#else
		if(ffrtsp_push[0].fp == NULL) {
			continue;
		}
		MEDIA_BUFFER buffer;
		// send video buffer
		buffer = RK_MPI_SYS_GetMediaBuffer(
				RK_ID_VENC, 0, -1);
		if (RK_MPI_MB_GetPtr(buffer) != NULL) {
			fwrite(RK_MPI_MB_GetPtr(buffer), RK_MPI_MB_GetSize(buffer), 1, ffrtsp_push[0].fp);
			RK_MPI_MB_ReleaseBuffer(buffer);
		}
#endif
	}
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


	while (!quit) {
		src_mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VDEC, crop_arg->chn_id, -1);
                
		//构建原始数据
		rga_src.fd = RK_MPI_MB_GetFD(src_mb);
		if(rga_src.fd == -1)
			continue;

		rga_src.x_offset = 512;//将图像往右挪，需要修改该数值并修改 rga_src.width
		rga_src.y_offset = 0;
		rga_src.format = RK_FORMAT_YCrCb_420_SP;
		rga_src.full_width = IMG_WIDTH;
		rga_src.full_height = IMG_HEIGHT;
		rga_src.width = IMG_WIDTH - 512;
		rga_src.height = IMG_HEIGHT;
		//构建目标数据

		rga_dst.fd = RK_MPI_MB_GetFD(dst_mb);
		rga_dst.format = RK_FORMAT_YCrCb_420_SP;
		rga_dst.full_width = crop_arg->target_width;
		rga_dst.full_height = crop_arg->target_height;
		rga_dst.width = crop_arg->target_width / 2;
		rga_dst.height = crop_arg->target_height;
		rga_dst.x_offset = crop_arg->target_x;
		rga_dst.y_offset = crop_arg->target_y;	

		rga_copy(rga_src,rga_dst,&rkRga);
                rga_src.fd = -1;
		RK_MPI_MB_ReleaseBuffer(src_mb);

	}
	return NULL;
}

//static void SAMPLE_COMMON_VENC_Start(struct Session *session) {
static void SAMPLE_COMMON_VENC_Start(void) {
  VENC_CHN_ATTR_S venc_chn_attr;
  int fps = 25;
  memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));

  venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H264;

  venc_chn_attr.stVencAttr.imageType = IMAGE_TYPE_NV12;

  venc_chn_attr.stVencAttr.u32PicWidth = SCREEN_WIDTH;
  venc_chn_attr.stVencAttr.u32PicHeight = SCREEN_HEIGHT;
  venc_chn_attr.stVencAttr.u32VirWidth = SCREEN_WIDTH;
  venc_chn_attr.stVencAttr.u32VirHeight = SCREEN_HEIGHT;
  venc_chn_attr.stVencAttr.u32Profile = 77;

  switch (RK_CODEC_TYPE_H264) {
  case RK_CODEC_TYPE_H264:
    venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = fps;
    venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate =
        SCREEN_WIDTH * SCREEN_HEIGHT * fps / 14;
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = fps;
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = fps;
    break;
  case RK_CODEC_TYPE_H265:
    venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
    venc_chn_attr.stRcAttr.stH265Vbr.u32Gop = fps;
    venc_chn_attr.stRcAttr.stH265Vbr.u32MaxBitRate =
        SCREEN_WIDTH * SCREEN_HEIGHT * fps / 14;
    venc_chn_attr.stRcAttr.stH265Vbr.fr32DstFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH265Vbr.fr32DstFrameRateNum = fps;
    venc_chn_attr.stRcAttr.stH265Vbr.u32SrcFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH265Vbr.u32SrcFrameRateNum = fps;
    break;
  default:
    printf("error: video codec not support.\n");
    break;
  }

  RK_MPI_VENC_CreateChn(0, &venc_chn_attr);
}

static void SAMPLE_COMMON_RGA_Start() {
  int ret = 0;
  RGA_ATTR_S stRgaAttr;
  stRgaAttr.bEnBufPool = RK_TRUE;
  stRgaAttr.u16BufPoolCnt = 2;
  stRgaAttr.u16Rotaion = 0;
  stRgaAttr.stImgIn.u32X = 0;
  stRgaAttr.stImgIn.u32Y = 0;
  stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;
  stRgaAttr.stImgIn.u32Width = SCREEN_WIDTH;
  stRgaAttr.stImgIn.u32Height = SCREEN_HEIGHT;
  stRgaAttr.stImgIn.u32HorStride = SCREEN_WIDTH;
  stRgaAttr.stImgIn.u32VirStride = SCREEN_HEIGHT;
  stRgaAttr.stImgOut.u32X = 0;
  stRgaAttr.stImgOut.u32Y = 0;
  stRgaAttr.stImgOut.imgType = IMAGE_TYPE_NV12;
  stRgaAttr.stImgOut.u32Width = SCREEN_WIDTH;
  stRgaAttr.stImgOut.u32Height = SCREEN_HEIGHT;
  stRgaAttr.stImgOut.u32HorStride = SCREEN_WIDTH;
  stRgaAttr.stImgOut.u32VirStride = SCREEN_HEIGHT;
  ret = RK_MPI_RGA_CreateChn(0, &stRgaAttr);
  if (ret) {
    printf("ERROR: Create rga[0] falied! ret=%d\n", ret);
  }
}


static void *rtsppushbuff(void *data) {
  switch (((struct RTSP_PUSH_INFO *)data)->type) {
  case RK_CODEC_TYPE_H264:
    ((struct RTSP_PUSH_INFO *)data)->type = RTSP_CODEC_H264;
    break;
  case RK_CODEC_TYPE_H265:
    ((struct RTSP_PUSH_INFO *)data)->type = RTSP_CODEC_H265;
    break;
  default:
    printf("error: video codec not support.\n");
    break;
  }
  ffrtsph264Push((struct RTSP_PUSH_INFO *)data);

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

  signal(SIGINT, sigterm_handler);

  RK_MPI_SYS_Init();//初始化 MPI 系统

#ifdef SAVE_FILE
  fp = fopen(SAVE_FILE_NAME, "a+b");
#endif
  // VDEC
  // 这里可以学习解码器数据结构配置
  VDEC_CHN_ATTR_S stVdecAttr;
  stVdecAttr.enCodecType = enCodecType; 		//解码格式
  stVdecAttr.enMode = VIDEO_MODE_FRAME; 		//解码输入模式 帧/流 
  if (bIsHardware) { 					//支持软/硬解码，这里通过输入参数判断
    if (stVdecAttr.enCodecType == RK_CODEC_TYPE_JPEG) { //硬
      stVdecAttr.enMode = VIDEO_MODE_FRAME;		//帧
    } else {
      stVdecAttr.enMode = VIDEO_MODE_STREAM;		//流
    }
    stVdecAttr.enDecodecMode = VIDEO_DECODEC_HADRWARE;
  } else {						//软
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

  MPP_CHN_S VdecChn, VoChn, RgaChn, VenChn;
  VdecChn.enModId = RK_ID_VDEC;
  VdecChn.s32DevId = 0;

  rga_demo_arg_t demo_arg[20];
  pthread_t read_thread[20];
  
  int i = 0;

  struct FFRTSPGet ffrtsp_get;
  ffrtsp_get.callback = FFRKMedia_Vdec_Send;
  ffrtsp_get.count = argc - 1;

  ffrtsp_push[0].idex = 0;
  ffrtsp_push[0].port = 8554;
  ffrtsp_push[0].type = RK_CODEC_TYPE_H264;

  MB_IMAGE_INFO_S stImageInfo = {
        SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH,
        SCREEN_HEIGHT, IMAGE_TYPE_NV12};
  dst_mb = RK_MPI_MB_CreateImageBuffer(&stImageInfo, RK_TRUE, 0);
    if (!dst_mb) {
      printf("ERROR: RK_MPI_MB_CreateImageBuffer get null buffer!\n");
    }

    RgaChn.enModId = RK_ID_RGA;
    RgaChn.s32ChnId = 0;
    SAMPLE_COMMON_RGA_Start();

    printf("VENC create\n");
    VenChn.enModId = RK_ID_VENC;
    VenChn.s32ChnId = 0;
    SAMPLE_COMMON_VENC_Start();

    ret = RK_MPI_SYS_Bind(&RgaChn, &VenChn);//RGA 绑定 VENC
    if (ret) {
       printf("ERROR: Bind rga[0] and stRgaVENChn[0] failed! ret=%d\n", ret);
       return -1;
    }
for(i = 1; i < argc; i++) {


  VdecChn.s32ChnId = i - 1;
  int x = (i - 1) % 2;
  int y = (i - 1) / 2;
  demo_arg[i - 1].target_x = (SCREEN_WIDTH / 2) * x;
  demo_arg[i - 1].target_y = (SCREEN_HEIGHT / 2) * y;
  demo_arg[i - 1].target_width = SCREEN_WIDTH;
  demo_arg[i - 1].target_height = SCREEN_HEIGHT;
  demo_arg[i - 1].chn_id = VdecChn.s32ChnId;

  printf(" demo_arg[%d - 1].target_x = %d\n", i,  demo_arg[i - 1].target_x);
  printf(" demo_arg[%d - 1].target_y = %d\n", i,  demo_arg[i - 1].target_y);
  printf(" demo_arg[%d - 1].target_width = %d\n", i, demo_arg[i - 1].target_width);
  printf(" demo_arg[%d - 1].target_height = %d\n", i, demo_arg[i - 1].target_height);

  ret = RK_MPI_VDEC_CreateChn(VdecChn.s32ChnId, &stVdecAttr);//创建解码器通道 
  if (ret) {
    printf("Create Vdec[0] failed! ret=%d\n", ret);
    return -1;
  }
  
  ffrtsp_get.ffrtsp_get_info[i - 1].url = argv[i];

  pthread_create(&read_thread[i - 1], NULL, GetMediaBuffer, &demo_arg[i - 1]);

}
  pthread_t rtsppush_thread, getvendata_thread, rga_thread;
  // RTSP 推流线程
  pthread_create(&rtsppush_thread, NULL, rtsppushbuff, (void *)&ffrtsp_push[0]);
  // 获取 VENC 数据
  pthread_create(&getvendata_thread, NULL, GetVenBuffer, NULL);
  // 将数据送入 RGA
  pthread_create(&rga_thread, NULL, SendMediaBuffer, NULL);
  
  ffrtspGet(ffrtsp_get);
  
  while(!quit) {
  	usleep(500 * 1000);
  }

  pthread_join(rtsppush_thread, NULL);
  pthread_join(getvendata_thread, NULL);
  pthread_join(rga_thread, NULL);

  RK_MPI_SYS_UnBind(&RgaChn,&VenChn);

  RK_MPI_VDEC_DestroyChn(VdecChn.s32DevId);
  RK_MPI_VENC_DestroyChn(VenChn.s32ChnId);
  RK_MPI_VO_DestroyChn(0);
#ifdef SAVE_FILE
  fclose(fp);
#endif
  return 0;
}
