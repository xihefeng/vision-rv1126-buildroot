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
#include <getopt.h>
#include <sys/types.h>


#include "common/sample_common.h"
#include "rkmedia_api.h"
#include "rkmedia_vdec.h"
#include "librtsp/rtsp_demo.h"
#include "rkmedia_venc.h"
#include "ffrtsp/ffrtsp.hh"



struct RTSP_PUSH_INFO ffrtsp_push[MAXFFRTSPChn];

#define INBUF_SIZE 1024 * 10

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 1280


static bool quit = false;
static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  quit = true;
}


//#define SAVE_FILE

#ifdef SAVE_FILE_VENC

static FILE *g_output_file;
void video_packet_cb(MEDIA_BUFFER mb) {
  static RK_S32 packet_cnt = 0;
  if (quit)
    return;

  const char *nalu_type = "Jpeg data";
  switch (RK_MPI_MB_GetFlag(mb)) {
  case VENC_NALU_IDRSLICE:
    nalu_type = "IDR Slice";
    break;
  case VENC_NALU_PSLICE:
    nalu_type = "P Slice";
    break;
  default:
    break;
  }

  if (g_output_file) {
    fwrite(RK_MPI_MB_GetPtr(mb), 1, RK_MPI_MB_GetSize(mb), g_output_file);
    printf("#Write packet-%d, %s, size %zu\n", packet_cnt, nalu_type,
           RK_MPI_MB_GetSize(mb));
  } else {
    printf("#Get packet-%d, %s, size %zu\n", packet_cnt, nalu_type,
           RK_MPI_MB_GetSize(mb));
  }
  RK_MPI_MB_TsNodeDump(mb);
  RK_MPI_MB_ReleaseBuffer(mb);

  packet_cnt++;
}
#else 
void video_packet_cb(MEDIA_BUFFER mb) { // Venc 数据结收 rtsp 发送回调函数
  static RK_S32 packet_cnt = 0;
  if (quit)
    return;

  printf("#Get packet-%d, size %zu\n", packet_cnt, RK_MPI_MB_GetSize(mb));
  
  fwrite((const uint8_t*)RK_MPI_MB_GetPtr(mb),RK_MPI_MB_GetSize(mb), 1, ffrtsp_push[0].fp);

  RK_MPI_MB_ReleaseBuffer(mb);
}

#endif

static RK_CHAR optstr[] = "?::i:o:f:w:h:t:l:";

static void *GetMediaBuffer(void *arg) {
  (void)arg;
  
  int ret = 0;
  
  RK_U32 u32Width = 1920;
  RK_U32 u32Height = 1080;
  CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;
  RK_CHAR *pCodecName = (char*)"H264";
  RK_S32 s32CamId = 0;

  MPP_CHN_S VdecChn;
  VdecChn.enModId = RK_ID_VDEC;
  VdecChn.s32DevId = 0;
  VdecChn.s32ChnId = 0;

     
  printf("#CodecName:%s\n", pCodecName);
  printf("#Resolution: %dx%d\n", u32Width, u32Height);
  printf("#CameraIdx: %d\n\n", s32CamId);

  // 这里可以学习 Venc 硬件编码器数据结构配置
  VENC_CHN_ATTR_S venc_chn_attr; 
  memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
  switch (enCodecType) { // 选择编码类型，这个通过曾需 -t 参数传入
  case RK_CODEC_TYPE_H265:
    venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H265; // stVencAttr 编码器属性，这里配置编码类型
    venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR; // stRcAttr 码率控制器属性 编码模式
    venc_chn_attr.stRcAttr.stH265Cbr.u32Gop = 30;//I 帧间隔，I 帧指基础的参考帧。
    venc_chn_attr.stRcAttr.stH265Cbr.u32BitRate = u32Width * u32Height; //平均比特率 bps
    // frame rate: in 30/1, out 30/1.
    venc_chn_attr.stRcAttr.stH265Cbr.fr32DstFrameRateDen = 1;//目标帧率分母
    venc_chn_attr.stRcAttr.stH265Cbr.fr32DstFrameRateNum = 30;//目标帧率分子
    venc_chn_attr.stRcAttr.stH265Cbr.u32SrcFrameRateDen = 1;//数据源帧率分母
    venc_chn_attr.stRcAttr.stH265Cbr.u32SrcFrameRateNum = 30;//数据源帧率分子
/*****************************************************************
这里解释一下，CBR模式是指码率恒定编码，VBR模式指动态，VCBR是VBR升级版
*****************************************************************/
    break;
  case RK_CODEC_TYPE_H264: //H264 与 H265 类似这里不做介绍
  default:
    venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H264;
    venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
    venc_chn_attr.stRcAttr.stH264Vbr.u32Gop = 30;
    //venc_chn_attr.stRcAttr.stH264Vbr.u32BitRate = u32Width * u32Height * 2;
    venc_chn_attr.stRcAttr.stH264Vbr.u32MaxBitRate = u32Width * u32Height * 2;
    // frame rate: in 30/1, out 30	/1.
    venc_chn_attr.stRcAttr.stH264Vbr.fr32DstFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Vbr.fr32DstFrameRateNum = 30;
    venc_chn_attr.stRcAttr.stH264Vbr.u32SrcFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Vbr.u32SrcFrameRateNum = 30;
    break;
  }
  venc_chn_attr.stVencAttr.imageType = IMAGE_TYPE_NV12; //输入图片的格式，和 VI 输出保持一致
  venc_chn_attr.stVencAttr.u32PicWidth = u32Width; //编码图像宽度，单位像素点
  venc_chn_attr.stVencAttr.u32PicHeight = u32Height;//编码图像高度，单位像素点
  venc_chn_attr.stVencAttr.u32VirWidth = u32Width;//stride 宽度，必须 16 对齐
  venc_chn_attr.stVencAttr.u32VirHeight = u32Height + 8;// stride 高度，必须 16 对齐
  venc_chn_attr.stVencAttr.u32Profile = 77; //编码等级 77,是中级 66,基础等级，100,高级
  ret = RK_MPI_VENC_CreateChn(0, &venc_chn_attr);//创建通道
  if (ret) {
    printf("ERROR: create VENC[0] error! ret=%d\n", ret);
    return NULL;
  }

  //这里可以学习通道结构数据配置，RKMedia是通过通道把各个模块的数据流串起来的
  MPP_CHN_S stEncChn; //定义模块设备通道结构体。
  stEncChn.enModId = RK_ID_VENC;//模块，这里使用Venc 模块
  stEncChn.s32DevId = 0;
  stEncChn.s32ChnId = 0;//通道号
  ret = RK_MPI_SYS_RegisterOutCb(&stEncChn, video_packet_cb); //注册数据输出回调。
  if (ret) {
    printf("ERROR: register output callback for VENC[0] error! ret=%d\n", ret);
    return NULL;
  }
  
  // Venc 模块数据输入端
  MPP_CHN_S stDestChn;
  stDestChn.enModId = RK_ID_VENC;
  stDestChn.s32DevId = 0;
  stDestChn.s32ChnId = 0;

  // VDEC->VO 绑定 VDEC 和 VENC 设备
  ret = RK_MPI_SYS_Bind(&VdecChn, &stDestChn);
  if (ret) {
    printf("Bind VDEC[0] to VO[0] failed! ret=%d\n", ret);
    quit = false;
    return NULL;
  }

  while (!quit) {
    usleep(500000);
  }

  ret = RK_MPI_SYS_UnBind(&VdecChn, &stDestChn);
  if (ret)
    printf("UnBind VDECp[0] to VO[0] failed! ret=%d\n", ret);

  ret = RK_MPI_VENC_DestroyChn(0);
  if (ret) {
    printf("ERROR: Destroy VENC[0] error! ret=%d\n", ret);
    return NULL;
  }

  return NULL;
}

//#define SAVE_FILE_RTSP

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
    RK_MPI_SYS_SendMediaBuffer(RK_ID_VDEC, 0, mb);
#endif
    RK_MPI_MB_ReleaseBuffer(mb);
    if (quit == true)
    	*pquit = true;
}

static void *rtsppushbuff(void *data) {
    ffrtsph264Push((struct RTSP_PUSH_INFO *)data);
}

int main(int argc, char *argv[]) {
  CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;
  int c, ret;
  
  if(argc != 2){
	printf("Usage: %s [IP camera url]\r\n",argv[0]);
  	return -1;
  }
	  

#ifdef SAVE_FILE
  RK_CHAR *pOutPath = (char*)"/save.h264";
  if (pOutPath) {
    g_output_file = fopen(pOutPath, "w");
    if (!g_output_file) {
      printf("ERROR: open file: %s fail, exit\n", pOutPath);
      return 0;
    }
  }
#endif
  signal(SIGINT, sigterm_handler);

  RK_MPI_SYS_Init();//初始化 MPI 系统

  // VDEC
  // 这里可以学习解码器数据结构配置
  VDEC_CHN_ATTR_S stVdecAttr;
  stVdecAttr.enCodecType = enCodecType; //解码格式
  //stVdecAttr.enImageType = IMAGE_TYPE_NV12;; //解码后输出格式
  stVdecAttr.enMode = VIDEO_MODE_FRAME; //解码输入模式 帧/流 
  if (stVdecAttr.enCodecType == RK_CODEC_TYPE_JPEG) {//硬
    stVdecAttr.enMode = VIDEO_MODE_FRAME;//帧
  } else {
    stVdecAttr.enMode = VIDEO_MODE_STREAM;//流
  }
  stVdecAttr.enDecodecMode = VIDEO_DECODEC_HADRWARE;

  ret = RK_MPI_VDEC_CreateChn(0, &stVdecAttr);//创建解码器通道 
  if (ret) {
    printf("Create Vdec[0] failed! ret=%d\n", ret);
    return -1;
  }
  
  /* 推流 */ //rtsp://168.168.101.137:8554/H264_stream_0
  pthread_t rtsppush_thread;
  ffrtsp_push[0].idex = 0;
  ffrtsp_push[0].port = 8554;
  pthread_create(&rtsppush_thread, NULL, rtsppushbuff, (void *)&ffrtsp_push[0]); 

  pthread_t read_thread;
  pthread_create(&read_thread, NULL, GetMediaBuffer, NULL);//绑定 VI VO 输出图片格式。
  
  /* 取流 */
  struct FFRTSPGet ffrtsp_get;
  ffrtsp_get.callback = FFRKMedia_Vdec_Send;
  ffrtsp_get.count = 1;
  ffrtsp_get.ffrtsp_get_info[0].url = argv[1];
  ffrtspGet(ffrtsp_get);
 
  quit = true;
  pthread_join(read_thread, NULL);

  RK_MPI_VDEC_DestroyChn(0);

  return 0;
}
