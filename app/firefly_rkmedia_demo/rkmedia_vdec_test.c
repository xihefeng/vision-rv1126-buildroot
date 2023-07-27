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

#define INBUF_SIZE 1024 * 10

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 1280

static bool quit = false;
static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  quit = true;
}

static void *GetMediaBuffer(void *arg) {
  (void)arg;
  MEDIA_BUFFER mb = NULL;
  int ret = 0;

  MPP_CHN_S VdecChn, VoChn;
  VdecChn.enModId = RK_ID_VDEC;
  VdecChn.s32DevId = 0;
  VdecChn.s32ChnId = 0;
  VoChn.enModId = RK_ID_VO;
  VoChn.s32DevId = 0;
  VoChn.s32ChnId = 0;

  mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VDEC, 0, 5000);//从指定通道中获取数据，模块ID，通道ID，阻塞时间
  if (!mb) {
    printf("RK_MPI_SYS_GetMediaBuffer get null buffer in 5s...\n");
    return NULL;
  }

  MB_IMAGE_INFO_S stImageInfo = {0};
  ret = RK_MPI_MB_GetImageInfo(mb, &stImageInfo);//指定图像中获取图像信息 ,宽高，图片格式
  if (ret) {
    printf("Get image info failed! ret = %d\n", ret);
    RK_MPI_MB_ReleaseBuffer(mb);
    return NULL;
  }

  printf("Get Frame:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
         "timestamp:%lld, ImgInfo:<wxh %dx%d, fmt 0x%x>\n",
         RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetFD(mb), RK_MPI_MB_GetSize(mb),
         RK_MPI_MB_GetModeID(mb), RK_MPI_MB_GetChannelID(mb),
         RK_MPI_MB_GetTimestamp(mb), stImageInfo.u32Width,
         stImageInfo.u32Height, stImageInfo.enImgType);
  RK_MPI_MB_ReleaseBuffer(mb);
  
  //这里可以学习 VO 设备数据结构配置，这里应用用不到 VO 暂时跳过
  VO_CHN_ATTR_S stVoAttr = {0};
  memset(&stVoAttr, 0, sizeof(stVoAttr));
  stVoAttr.pcDevNode = "/dev/dri/card0";
  stVoAttr.emPlaneType = VO_PLANE_OVERLAY;
  stVoAttr.enImgType = stImageInfo.enImgType;
  stVoAttr.u16Zpos = 1;
  stVoAttr.u32Width = SCREEN_WIDTH;
  stVoAttr.u32Height = SCREEN_HEIGHT;
  stVoAttr.stImgRect.s32X = 0;
  stVoAttr.stImgRect.s32Y = 0;
  stVoAttr.stImgRect.u32Width = stImageInfo.u32Width;
  stVoAttr.stImgRect.u32Height = stImageInfo.u32Height;
  stVoAttr.stDispRect.s32X = 0;
  stVoAttr.stDispRect.s32Y = 0;
  stVoAttr.stDispRect.u32Width = stImageInfo.u32Width;
  stVoAttr.stDispRect.u32Height = stImageInfo.u32Height;
  ret = RK_MPI_VO_CreateChn(0, &stVoAttr);
  if (ret) {
    printf("Create VO[0] failed! ret=%d\n", ret);
    quit = false;
    return NULL;
  }

  // VDEC->VO 绑定 VDEC 和 VO 设备
  ret = RK_MPI_SYS_Bind(&VdecChn, &VoChn);
  if (ret) {
    printf("Bind VDEC[0] to VO[0] failed! ret=%d\n", ret);
    quit = false;
    return NULL;
  }

  while (!quit) {
    usleep(500000);
  }

  ret = RK_MPI_SYS_UnBind(&VdecChn, &VoChn);
  if (ret)
    printf("UnBind VDECp[0] to VO[0] failed! ret=%d\n", ret);

  ret = RK_MPI_VO_DestroyChn(VoChn.s32ChnId);
  if (ret)
    printf("Destroy VO[0] failed! ret=%d\n", ret);

  return NULL;
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

int main(int argc, char *argv[]) {
  RK_CHAR *pcFileName = NULL;
  RK_U32 u32DispWidth = 720;
  RK_U32 u32DispHeight = 1280;
  RK_BOOL bIsHardware = RK_TRUE;
  RK_U32 u32Loop = 0;
  CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;
  int c, ret;

  while ((c = getopt(argc, argv, optstr)) != -1) {
    switch (c) {
    case 'w':
      u32DispWidth = atoi(optarg);
      break;
    case 'h':
      u32DispHeight = atoi(optarg);
      break;
    case 'i':
      pcFileName = optarg;
      break;
    case 'f':
      bIsHardware = atoi(optarg) ? RK_TRUE : RK_FALSE;
      break;
    case 'l':
      u32Loop = atoi(optarg);
      break;
    case 't':
      if (strcmp(optarg, "H264") == 0) {
        enCodecType = RK_CODEC_TYPE_H264;
      } else if (strcmp(optarg, "H265") == 0) {
        enCodecType = RK_CODEC_TYPE_H265;
      } else if (strcmp(optarg, "JPEG") == 0) {
        enCodecType = RK_CODEC_TYPE_JPEG;
      }
      break;
    case '?':
    default:
      print_usage();
      return 0;
    }
  }
  // 本 DEMO 不需要使用 MIPI 摄像头，所以不需要初始化 RKAIQ
  printf("#FileName: %s\n", pcFileName);
  printf("#Display wxh: %dx%d\n", u32DispWidth, u32DispHeight);
  printf("#Decode Mode: %s\n", bIsHardware ? "Hardware" : "Software");
  printf("#Loop Cnt: %d\n", u32Loop);

  signal(SIGINT, sigterm_handler);

  FILE *infile = fopen(pcFileName, "rb"); //待解码文件，h264或者h265
  if (!infile) {
    fprintf(stderr, "Could not open %s\n", pcFileName);
    return 0;
  }

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

  ret = RK_MPI_VDEC_CreateChn(0, &stVdecAttr);//创建解码器通道 
  if (ret) {
    printf("Create Vdec[0] failed! ret=%d\n", ret);
    return -1;
  }

  pthread_t read_thread;
  pthread_create(&read_thread, NULL, GetMediaBuffer, NULL);//绑定 VI VO 输出图片格式。

  int data_size;
  int read_size;
  if (stVdecAttr.enMode == VIDEO_MODE_STREAM) {
    data_size = INBUF_SIZE;//流模式设置一个解码数据包大小
  } else if (stVdecAttr.enMode == VIDEO_MODE_FRAME) {
    fseek(infile, 0, SEEK_END);
    data_size = ftell(infile);//帧模式直接读取文件大小
    fseek(infile, 0, SEEK_SET);
  }

  while (!quit) {
    MEDIA_BUFFER mb = RK_MPI_MB_CreateBuffer(data_size, RK_FALSE, 0);//创建普通数据缓存区，缓存区大小，是否创建 DMA BUFF
  RETRY:
    /* read raw data from the input file */
    read_size = fread(RK_MPI_MB_GetPtr(mb), 1, data_size, infile);//把一个包的数据读到缓存中
    if (!read_size || feof(infile)) {
      if (u32Loop) {//设置循环次数
        fseek(infile, 0, SEEK_SET);
        goto RETRY;
      } else {
        RK_MPI_MB_ReleaseBuffer(mb);
        break;
      }
    }
    RK_MPI_MB_SetSize(mb, read_size);//设置 BUFF 为读出来的数据大小
    printf("#Send packet(%p, %zuBytes) to VDEC[0].\n", RK_MPI_MB_GetPtr(mb),
           RK_MPI_MB_GetSize(mb));//打印发送多大数据到 VDEC
    ret = RK_MPI_SYS_SendMediaBuffer(RK_ID_VDEC, 0, mb);//发送 MB BUFF 到 VDEC
    RK_MPI_MB_ReleaseBuffer(mb);

    usleep(30 * 1000);
  }

  quit = true;
  pthread_join(read_thread, NULL);

  RK_MPI_VDEC_DestroyChn(0);
  fclose(infile);

  return 0;
}
