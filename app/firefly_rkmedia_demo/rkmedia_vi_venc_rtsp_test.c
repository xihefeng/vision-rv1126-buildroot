// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "common/sample_common.h"
#include "librtsp/rtsp_demo.h"
#include "rkmedia_api.h"
#include "rkmedia_venc.h"

rtsp_demo_handle g_rtsplive = NULL;
static rtsp_session_handle g_rtsp_session;
static bool quit = false;
static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  quit = true;
}

void video_packet_cb(MEDIA_BUFFER mb) { // Venc 数据结收 rtsp 发送回调函数
  static RK_S32 packet_cnt = 0;
  if (quit)
    return;

  printf("#Get packet-%d, size %zu\n", packet_cnt, RK_MPI_MB_GetSize(mb));

  if (g_rtsplive && g_rtsp_session) {
    rtsp_tx_video(g_rtsp_session, RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetSize(mb),
                  RK_MPI_MB_GetTimestamp(mb)); // RTSP 发送，通过获取 MB 编码数据虚拟地址，大小，时间戳，然后发送
    rtsp_do_event(g_rtsplive);
  }

  RK_MPI_MB_ReleaseBuffer(mb);
  packet_cnt++;
}

static RK_CHAR optstr[] = "?::a::w:h:c:e:d:I:M:";
static const struct option long_options[] = {
    {"aiq", optional_argument, NULL, 'a'},
    {"device_name", required_argument, NULL, 'd'},
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"frame_cnt", required_argument, NULL, 'c'},
    {"encode", required_argument, NULL, 'e'},
    {"camid", required_argument, NULL, 'I'},
    {"multictx", required_argument, NULL, 'M'},
    {"help", optional_argument, NULL, '?'},
    {NULL, 0, NULL, 0},
};

static void print_usage(const RK_CHAR *name) {
  printf("usage example:\n");
#ifdef RKAIQ
  printf("\t%s [-a [iqfiles_dir]] [-w 1920] "
         "[-h 1080]"
         "[-d rkispp_scale0] "
         "[-e 0] "
         "[-I 0] "
         "[-M 0] "
         "\n",
         name);
  printf("\t-a | --aiq: enable aiq with dirpath provided, eg:-a "
         "/oem/etc/iqfiles/, "
         "set dirpath emtpty to using path by default, without this option aiq "
         "should run in other application\n");
  printf("\t-M | --multictx: switch of multictx in isp, set 0 to disable, set "
         "1 to enable. Default: 0\n");
#else
  printf("\t%s [-w 1920] "
         "[-h 1080]"
         "[-I 0] "
         "[-d rkispp_scale0] "
         "[-e 0] "
         "\n",
         name);
#endif
  printf("\t-I | --camid: camera ctx id, Default 0\n");
  printf("\t-w | --width: VI width, Default:1920\n");
  printf("\t-h | --heght: VI height, Default:1080\n");
  printf("\t-d | --device_name set pcDeviceName, Default:rkispp_scale0, "
         "Option:[rkispp_scale0, rkispp_scale1, rkispp_scale2]\n");
  printf("\t-e | --encode: encode type, Default:h264, Value:h264, h265\n");
}

int main(int argc, char *argv[]) {
  RK_U32 u32Width = 1920;
  RK_U32 u32Height = 1080;
  RK_CHAR *pDeviceName = "rkispp_scale0";
  RK_CHAR *pIqfilesPath = NULL;
  CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;
  RK_CHAR *pCodecName = "H264";
  RK_S32 s32CamId = 0;
#ifdef RKAIQ
  RK_BOOL bMultictx = RK_FALSE;
#endif
  int c;
  int ret = 0;
  while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
    const char *tmp_optarg = optarg;
    switch (c) {
    case 'a':
      if (!optarg && NULL != argv[optind] && '-' != argv[optind][0]) {
        tmp_optarg = argv[optind++];
      }
      if (tmp_optarg) {
        pIqfilesPath = (char *)tmp_optarg;
      } else {
        pIqfilesPath = "/oem/etc/iqfiles";
      }
      break;
    case 'w':
      u32Width = atoi(optarg);
      break;
    case 'h':
      u32Height = atoi(optarg);
      break;
    case 'd':
      pDeviceName = optarg;
      break;
    case 'e':
      if (!strcmp(optarg, "h264")) {
        enCodecType = RK_CODEC_TYPE_H264;
        pCodecName = "H264";
      } else if (!strcmp(optarg, "h265")) {
        enCodecType = RK_CODEC_TYPE_H265;
        pCodecName = "H265";
      } else {
        printf("ERROR: Invalid encoder type.\n");
        return 0;
      }
      break;
    case 'I':
      s32CamId = atoi(optarg);
      break;
#ifdef RKAIQ
    case 'M':
      if (atoi(optarg)) {
        bMultictx = RK_TRUE;
      }
      break;
#endif
    case '?':
    default:
      print_usage(argv[0]);
      return 0;
    }
  }

  printf("#Device: %s\n", pDeviceName);
  printf("#CodecName:%s\n", pCodecName);
  printf("#Resolution: %dx%d\n", u32Width, u32Height);
  printf("#CameraIdx: %d\n\n", s32CamId);
#ifdef RKAIQ
  printf("#bMultictx: %d\n\n", bMultictx);
  printf("#Aiq xml dirpath: %s\n\n", pIqfilesPath);
#endif

  if (pIqfilesPath) { // RKAIQ iqfile 路径
#ifdef RKAIQ //RKAIQ 初始化 RKMedia 依赖于 RKAIQ 如果此初始化在其他进程进行了就不需要再次初始化，否则报错
    rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL; // 工作模式，这里使用正常模式，不开启 HDR
    int fps = 30;
    SAMPLE_COMM_ISP_Init(s32CamId, hdr_mode, bMultictx, pIqfilesPath);
    SAMPLE_COMM_ISP_Run(s32CamId);
    SAMPLE_COMM_ISP_SetFrameRate(s32CamId, fps);
#endif
  }

  // init rtsp 这里暂时没有找到源码，RK 提供的是 rtsp 静态编译库
  g_rtsplive = create_rtsp_demo(554);
  g_rtsp_session = rtsp_new_session(g_rtsplive, "/live/main_stream");
  
  if (enCodecType == RK_CODEC_TYPE_H264) { //选择 RTSP 推流类型
    rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H264, NULL, 0);
  } else if (enCodecType == RK_CODEC_TYPE_H265) {
    rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H265, NULL, 0);
  } else {
    printf("not support other type\n");
    return -1;
  }
  rtsp_sync_video_ts(g_rtsp_session, rtsp_get_reltime(), rtsp_get_ntptime()); //rtsp 同步当前网络时间

  RK_MPI_SYS_Init();//初始化 RKMedia MPI 系统

  // 这里可以学习 VI 输入模块的数据结构配置
  VI_CHN_ATTR_S vi_chn_attr;// VI 通道描述
  vi_chn_attr.pcVideoNode = pDeviceName;//设备节点，这里对应 rkisp 的设备节点
  vi_chn_attr.u32BufCnt = 3; // VI 捕获视频缓冲区计数
  vi_chn_attr.u32Width = u32Width; //video 分辨率宽度
  vi_chn_attr.u32Height = u32Height; //video 分辨率高度
  vi_chn_attr.enPixFmt = IMAGE_TYPE_NV12; //video 图像格式
  vi_chn_attr.enBufType = VI_CHN_BUF_TYPE_MMAP;
  vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL; // VI 通道工作模式
  ret = RK_MPI_VI_SetChnAttr(s32CamId, 0, &vi_chn_attr); // 设置通道属性
  ret |= RK_MPI_VI_EnableChn(s32CamId, 0); // 启用VI通道
  if (ret) {
    printf("ERROR: create VI[0] error! ret=%d\n", ret);
    return 0;
  }

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
    venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = 30;
    venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = u32Width * u32Height;
    // frame rate: in 30/1, out 30/1.
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;
    break;
  }
  venc_chn_attr.stVencAttr.imageType = IMAGE_TYPE_NV12; //输入图片的格式，和VI 输出保持一致
  venc_chn_attr.stVencAttr.u32PicWidth = u32Width; //编码图像宽度，单位像素点
  venc_chn_attr.stVencAttr.u32PicHeight = u32Height;//编码图像高度，单位像素点
  venc_chn_attr.stVencAttr.u32VirWidth = u32Width;//stride 宽度，必须 16 对齐
  venc_chn_attr.stVencAttr.u32VirHeight = u32Height;// stride 高度，必须 16 对齐
  venc_chn_attr.stVencAttr.u32Profile = 77; //编码等级 77,是中级 66,基础等级，100,高级
  ret = RK_MPI_VENC_CreateChn(0, &venc_chn_attr);//创建通道
  if (ret) {
    printf("ERROR: create VENC[0] error! ret=%d\n", ret);
    return 0;
  }

  //这里可以学习通道结构数据配置，RKMedia是通过通道把各个模块的数据流串起来的
  MPP_CHN_S stEncChn; //定义模块设备通道结构体。
  stEncChn.enModId = RK_ID_VENC;//模块，这里使用Venc 模块
  stEncChn.s32DevId = 0;
  stEncChn.s32ChnId = 0;//通道号
  ret = RK_MPI_SYS_RegisterOutCb(&stEncChn, video_packet_cb); //注册数据输出回调。
  if (ret) {
    printf("ERROR: register output callback for VENC[0] error! ret=%d\n", ret);
    return 0;
  }
  
  // VI 模块数据输出端
  MPP_CHN_S stSrcChn;
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = 0;
  stSrcChn.s32ChnId = 0;
  
  // Venc 模块数据输入端
  MPP_CHN_S stDestChn;
  stDestChn.enModId = RK_ID_VENC;
  stDestChn.s32DevId = 0;
  stDestChn.s32ChnId = 0;
  ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);//两端绑定
  if (ret) {
    printf("ERROR: Bind VI[0] and VENC[0] error! ret=%d\n", ret);
    return 0;
  }

  printf("%s initial finish\n", __func__);
  signal(SIGINT, sigterm_handler);
  while (!quit) {
    usleep(500000);
  }

  printf("%s exit!\n", __func__);
  if (g_rtsplive)
    rtsp_del_demo(g_rtsplive);

  // unbind first
  ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
  if (ret) {
    printf("ERROR: UnBind VI[0] and VENC[0] error! ret=%d\n", ret);
    return 0;
  }
  // destroy venc before vi
  ret = RK_MPI_VENC_DestroyChn(0);
  if (ret) {
    printf("ERROR: Destroy VENC[0] error! ret=%d\n", ret);
    return 0;
  }
  // destroy vi
  ret = RK_MPI_VI_DisableChn(s32CamId, 0);
  if (ret) {
    printf("ERROR: Destroy VI[0] error! ret=%d\n", ret);
    return 0;
  }

  if (pIqfilesPath) {
#ifdef RKAIQ
    SAMPLE_COMM_ISP_Stop(s32CamId);
#endif
  }
  return 0;
}
