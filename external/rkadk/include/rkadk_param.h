/*
 * Copyright (c) 2021 Rockchip, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __RKADK_PARAM_H__
#define __RKADK_PARAM_H__

#include "rkadk_common.h"
#include "rkadk_record.h"
#include "rkmedia_api.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* version */
#define RKADK_PARAM_VERSION "1.2.0"

#define RKADK_BUFFER_LEN 64
#define RKADK_PIX_FMT_LEN 10
#define RKADK_RC_MODE_LEN 5
#define RKADK_PATH_LEN 128

/* sensor default parameters */
#define SENSOR_MAX_WIDTH 2688
#define SENSOR_MAX_HEIGHT 1520

/* audio default parameters */
/* g711u must be 16K, g711a can be either 8K or 16K */
#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_CHANNEL 1
#define AUDIO_BIT_REAT 64000
/* mp2: 1152, mp3: 1024 */
#define AUDIO_FRAME_COUNT  1152
#define AUDIO_SAMPLE_FORMAT RK_SAMPLE_FMT_S16
#define AI_DEVICE_NAME "default"

/* video default parameters */
#define VIDEO_GOP 30
#define VIDEO_FRAME_RATE 30
#define VIDEO_PROFILE 100
#define STREAM_VIDEO_WIDTH 1280
#define STREAM_VIDEO_HEIGHT 720
#define RECORD_VIDEO_WIDTH SENSOR_MAX_WIDTH
#define RECORD_VIDEO_HEIGHT SENSOR_MAX_HEIGHT
#define RECORD_VIDEO_WIDTH_S STREAM_VIDEO_WIDTH
#define RECORD_VIDEO_HEIGHT_S STREAM_VIDEO_HEIGHT
#define PHOTO_VIDEO_WIDTH SENSOR_MAX_WIDTH
#define PHOTO_VIDEO_HEIGHT SENSOR_MAX_HEIGHT
#define DISP_WIDTH 720
#define DISP_HEIGHT 1280

/* thumb default parameters */
#define THUMB_WIDTH 320
#define THUMB_HEIGHT 180
#define THUMB_VENC_CHN (VENC_MAX_CHN_NUM - 1)

#define RECORD_AI_CHN 0
#define RECORD_AENC_CHN 0

#define PREVIEW_AI_CHN RECORD_AI_CHN
#define PREVIEW_AENC_CHN 1

#define LIVE_AI_CHN RECORD_AI_CHN
#define LIVE_AENC_CHN RECORD_AENC_CHN

/* setting file path */
#define RKADK_DEFPARAM_PATH "/etc/rkadk/rkadk_defsetting.ini"
#define RKADK_DEFPARAM_PATH_SENSOR_PREFIX "/etc/rkadk/rkadk_defsetting_sensor"
#define RKADK_PARAM_PATH "/data/rkadk/rkadk_setting.ini"
#define RKADK_PARAM_PATH_SENSOR_PREFIX "/data/rkadk/rkadk_setting_sensor"

/* Resolution */
#define RKADK_WIDTH_720P 1280
#define RKADK_HEIGHT_720P 720

#define RKADK_WIDTH_1080P 1920
#define RKADK_HEIGHT_1080P 1080

#define RKADK_WIDTH_1296P 2340
#define RKADK_HEIGHT_1296P 1296

#define RKADK_WIDTH_1440P 2560
#define RKADK_HEIGHT_1440P 1440

#define RKADK_WIDTH_1520P 2688
#define RKADK_HEIGHT_1520P 1520

#define RKADK_WIDTH_1600P 2560
#define RKADK_HEIGHT_1600P 1600

#define RKADK_WIDTH_1620P 2880
#define RKADK_HEIGHT_1620P 1620

#define RKADK_WIDTH_1944P 2592
#define RKADK_HEIGHT_1944P 1944

#define RKADK_WIDTH_3840P 3840
#define RKADK_HEIGHT_2160P 2160

/* Resolution type */
typedef enum {
  RKADK_RES_720P = 0, /* 1280*720 */
  RKADK_RES_1080P,    /* 1920*1080 */
  RKADK_RES_1296P,    /* 2340*1296 */
  RKADK_RES_1440P,    /* 2560*1440 */
  RKADK_RES_1520P,    /* 2688*1520 */
  RKADK_RES_1600P,    /* 2560*1600 */
  RKADK_RES_1620P,    /* 2880*1620 */
  RKADK_RES_1944P,    /* 2592*1944 */
  RKADK_RES_2160P,    /* 3840*2160 */
  RKADK_RES_BUTT,
} RKADK_PARAM_RES_E;

/* config param */
typedef enum {
  // CAM Dependent Param
  RKADK_PARAM_TYPE_FPS,             /* framerate */
  RKADK_PARAM_TYPE_GOP,             /* gop */
  RKADK_PARAM_TYPE_RES,             /* specify RKADK_PARAM_RES_E(record) */
  RKADK_PARAM_TYPE_PHOTO_RES,       /* specify RKADK_PARAM_RES_E(photo) */
  RKADK_PARAM_TYPE_CODEC_TYPE,      /* specify RKADK_PARAM_CODEC_CFG_S */
  RKADK_PARAM_TYPE_BITRATE,         /* specify RKADK_PARAM_BITRATE_S */
  RKADK_PARAM_TYPE_FLIP,            /* bool */
  RKADK_PARAM_TYPE_MIRROR,          /* bool */
  RKADK_PARAM_TYPE_LDC,             /* ldc level [0,255] */
  RKADK_PARAM_TYPE_ANTIFOG,         /* antifog value, [0,10] */
  RKADK_PARAM_TYPE_WDR,             /* wdr level, [0,10] */
  RKADK_PARAM_TYPE_HDR,             /* 0: normal, 1: HDR2, 2: HDR3, [0,2] */
  RKADK_PARAM_TYPE_REC,             /* record  enable, bool*/
  RKADK_PARAM_TYPE_RECORD_TYPE,     /* specify RKADK_REC_TYPE_E */
  RKADK_PARAM_TYPE_RECORD_TIME,     /* specify RKADK_PARAM_REC_TIME_S, record time(s) */
  RKADK_PARAM_TYPE_PRE_RECORD_TIME, /* pre record time, unit in second(s) */
  RKADK_PARAM_TYPE_PRE_RECORD_MODE, /* pre record mode, specify MUXER_PRE_RECORD_MODE_E */
  RKADK_PARAM_TYPE_SPLITTIME, /* specify RKADK_PARAM_REC_TIME_S, manual splite time(s) */
  RKADK_PARAM_TYPE_FILE_CNT,  /* record file count, maximum RECORD_FILE_NUM_MAX */
  RKADK_PARAM_TYPE_LAPSE_INTERVAL, /* specify RKADK_PARAM_REC_TIME_S, lapse interval(s) */
  RKADK_PARAM_TYPE_LAPSE_MULTIPLE, /* lapse multiple */
  RKADK_PARAM_TYPE_PHOTO_ENABLE,   /* photo enable, bool*/
  RKADK_PARAM_TYPE_SNAP_NUM,       /* continue snap num */

  // COMM Dependent Param
  RKADK_PARAM_TYPE_REC_UNMUTE,      /* record audio mute, bool */
  RKADK_PARAM_TYPE_AUDIO,           /* speaker enable, bool */
  RKADK_PARAM_TYPE_VOLUME,          /* speaker volume, [0,100] */
  RKADK_PARAM_TYPE_MIC_UNMUTE,      /* mic(mute) enable, bool */
  RKADK_PARAM_TYPE_MIC_VOLUME,      /* mic volume, [0,100] */
  RKADK_PARAM_TYPE_OSD,             /* show osd or not, bool */
  RKADK_PARAM_TYPE_OSD_TIME_FORMAT, /* osd format for time */
  RKADK_PARAM_TYPE_BOOTSOUND,       /* boot sound enable, bool */
  RKADK_PARAM_TYPE_OSD_SPEED,       /* speed osd enable, bool */
  RKADK_PARAM_TYPE_BUTT
} RKADK_PARAM_TYPE_E;

typedef enum {
  RKADK_STREAM_TYPE_VIDEO_MAIN,
  RKADK_STREAM_TYPE_VIDEO_SUB,
  RKADK_STREAM_TYPE_SNAP,
  RKADK_STREAM_TYPE_PREVIEW,
  RKADK_STREAM_TYPE_LIVE,
  RKADK_STREAM_TYPE_DISP,
  RKADK_STREAM_TYPE_BUTT
} RKADK_STREAM_TYPE_E;

typedef struct {
  RKADK_STREAM_TYPE_E enStreamType;
  RKADK_CODEC_TYPE_E enCodecType;
} RKADK_PARAM_CODEC_CFG_S;

typedef struct {
  RKADK_STREAM_TYPE_E enStreamType;
  RKADK_U32 u32Bitrate;
} RKADK_PARAM_BITRATE_S;

typedef struct {
  RKADK_STREAM_TYPE_E enStreamType;
  RKADK_U32 time;
} RKADK_PARAM_REC_TIME_S;

typedef struct {
  RKADK_STREAM_TYPE_E enStreamType;
  RKADK_U32 u32Gop;
} RKADK_PARAM_GOP_S;

typedef struct tagRKADK_PARAM_VERSION_S {
  char version[RKADK_BUFFER_LEN];
} RKADK_PARAM_VERSION_S;

typedef struct tagRKADK_PARAM_VI_CFG_S {
  RKADK_U32 chn_id;
  char device_name[RKADK_BUFFER_LEN];
  RKADK_U32 buf_cnt;
  RKADK_U32 width;
  RKADK_U32 height;
  char pix_fmt[RKADK_PIX_FMT_LEN]; /* options: NV12/NV16/YUYV/FBC0/FBC2 */

  /* options: NONE/RECORD_MAIN/RECORD_SUB/PREVIEW/PHOTO/LIVE/DISP */
  char module [RKADK_BUFFER_LEN];
} RKADK_PARAM_VI_CFG_S;

typedef struct tagRKADK_PARAM_COMM_CFG_S {
  RKADK_U32 sensor_count;
  bool rec_unmute;          /* false:disable record audio, true:enable */
  bool enable_speaker;      /* speaker enable, default true */
  RKADK_U32 speaker_volume; /* speaker volume, [0,100] */
  bool mic_unmute;          /* 0:close mic(mute),  1:open mic(unmute) */
  RKADK_U32 mic_volume;     /* mic input volume, [0,100] */
  RKADK_U32 osd_time_format;
  bool osd;        /* Whether to display OSD */
  bool boot_sound; /* boot sound */
  bool osd_speed;  /* speed osd */
} RKADK_PARAM_COMM_CFG_S;

typedef struct tagRKADK_PARAM_SENSOR_CFG_S {
  RKADK_U32 max_width;
  RKADK_U32 max_height;
  RKADK_U32 framerate;
  bool enable_record; /* record  enable*/
  bool enable_photo;  /* photo enable, default true */
  bool flip;          /* FLIP */
  bool mirror;        /* MIRROR */
  RKADK_U32 ldc;      /* LDC level, [0,255]*/
  RKADK_U32 wdr;      /* WDR level, [0,10] */
  RKADK_U32 hdr;      /* hdr, [0: normal, 1: HDR2, 2: HDR3] */
  RKADK_U32 antifog;  /* antifog value, [0,10] */
} RKADK_PARAM_SENSOR_CFG_S;

typedef struct tagRKADK_PARAM_AUDIO_CFG_S {
  char audio_node[RKADK_BUFFER_LEN];
  SAMPLE_FORMAT_E sample_format;
  RKADK_U32 channels;
  RKADK_U32 samplerate;
  RKADK_U32 samples_per_frame;
  RKADK_U32 bitrate;
  AI_LAYOUT_E ai_layout;
  RKADK_VQE_MODE_E vqe_mode;
  RKADK_CODEC_TYPE_E codec_type;
} RKADK_PARAM_AUDIO_CFG_S;

typedef struct tagRKADK_PARAM_VENC_PARAM_S {
  /* rc param */
  RKADK_S32 first_frame_qp; /* start QP value of the first frame, default: -1 */
  RKADK_U32 qp_step;
  RKADK_U32 max_qp; /* max QP: [8, 51], default: 48 */
  RKADK_U32 min_qp; /* min QP: [0, 48], can't be larger than max_qp, default: 8 */
  RKADK_U32 row_qp_delta_i; /* only CBR, [0, 10], default: 1 */
  RKADK_U32 row_qp_delta_p; /* only CBR, [0, 10], default: 2 */
  bool hier_qp_en;
  char hier_qp_delta[RKADK_BUFFER_LEN];
  char hier_frame_num[RKADK_BUFFER_LEN];

  bool full_range;
  bool scaling_list;
} RKADK_PARAM_VENC_PARAM_S;

typedef struct tagRKADK_PARAM_VENC_ATTR_S {
  RKADK_U32 width;
  RKADK_U32 height;
  RKADK_U32 bitrate;
  RKADK_U32 gop;
  RKADK_U32 profile;
  RKADK_CODEC_TYPE_E codec_type;
  RKADK_U32 venc_chn;
  char rc_mode[RKADK_RC_MODE_LEN]; /* options: CBR/VBR/AVBR */
  RKADK_PARAM_VENC_PARAM_S venc_param;
} RKADK_PARAM_VENC_ATTR_S;

typedef struct {
  RKADK_U32 u32ViChn;
  VI_CHN_ATTR_S stChnAttr;
} RKADK_PRAAM_VI_ATTR_S;

typedef struct tagRKADK_PARAM_REC_TIME_CFG_S {
  RKADK_U32 record_time;
  RKADK_U32 splite_time;
  RKADK_U32 lapse_interval;
} RKADK_PARAM_REC_TIME_CFG_S;

typedef struct tagRKADK_PARAM_REC_CFG_S {
  RKADK_REC_TYPE_E record_type;
  RKADK_U32 pre_record_time;
  MUXER_PRE_RECORD_MODE_E pre_record_mode;
  RKADK_U32 lapse_multiple;
  RKADK_U32 file_num;
  RKADK_PARAM_REC_TIME_CFG_S record_time_cfg[RECORD_FILE_NUM_MAX];
  RKADK_PARAM_VENC_ATTR_S attribute[RECORD_FILE_NUM_MAX];
  RKADK_PRAAM_VI_ATTR_S vi_attr[RECORD_FILE_NUM_MAX];
} RKADK_PARAM_REC_CFG_S;

typedef struct tagRKADK_PARAM_STREAM_CFG_S {
  RKADK_PARAM_VENC_ATTR_S attribute;
  RKADK_PRAAM_VI_ATTR_S vi_attr;
} RKADK_PARAM_STREAM_CFG_S;

typedef struct tagRKADK_PARAM_PHOTO_CFG_S {
  RKADK_U32 image_width;
  RKADK_U32 image_height;
  RKADK_U32 snap_num;
  RKADK_U32 venc_chn;
  RKADK_PRAAM_VI_ATTR_S vi_attr;
} RKADK_PARAM_PHOTO_CFG_S;

typedef struct tagRKADK_PARAM_DISP_CFG_S {
  RKADK_U32 width;
  RKADK_U32 height;
  // rga
  bool enable_buf_pool;
  RKADK_U32 buf_pool_cnt;
  RKADK_U32 rotaion;
  RKADK_U32 rga_chn;
  // vo
  char device_node[RKADK_BUFFER_LEN];
  VO_PLANE_TYPE_E plane_type;
  char img_type[RKADK_PIX_FMT_LEN]; /* specify IMAGE_TYPE_E: NV12/RGB888... */
  RKADK_U32 z_pos;
  RKADK_U32 vo_chn;
  RKADK_PRAAM_VI_ATTR_S vi_attr;
} RKADK_PARAM_DISP_CFG_S;

typedef struct tagRKADK_PARAM_MEDIA_CFG_S {
  RKADK_PARAM_VI_CFG_S stViCfg[RKADK_ISPP_VI_NODE_CNT];
  RKADK_PARAM_REC_CFG_S stRecCfg;
  RKADK_PARAM_STREAM_CFG_S stStreamCfg;
  RKADK_PARAM_STREAM_CFG_S stLiveCfg;
  RKADK_PARAM_PHOTO_CFG_S stPhotoCfg;
  RKADK_PARAM_DISP_CFG_S stDispCfg;
} RKADK_PARAM_MEDIA_CFG_S;

typedef struct tagRKADK_PARAM_THUMB_CFG_S {
  // 4 alignment
  RKADK_U32 thumb_width;
  // 2 alignment
  RKADK_U32 thumb_height;
  RKADK_U32 venc_chn;
} RKADK_PARAM_THUMB_CFG_S;

typedef struct tagPARAM_CFG_S {
  RKADK_PARAM_VERSION_S stVersion;
  RKADK_PARAM_COMM_CFG_S stCommCfg;
  RKADK_PARAM_AUDIO_CFG_S stAudioCfg;
  RKADK_PARAM_THUMB_CFG_S stThumbCfg;
  RKADK_PARAM_SENSOR_CFG_S stSensorCfg[RKADK_MAX_SENSOR_CNT];
  RKADK_PARAM_MEDIA_CFG_S stMediaCfg[RKADK_MAX_SENSOR_CNT];
} RKADK_PARAM_CFG_S;

/* Param Context */
typedef struct {
  bool bInit;                /* module init status */
  pthread_mutex_t mutexLock; /* param lock, protect pstCfg */
  RKADK_PARAM_CFG_S stCfg;   /* param config */
  char path[RKADK_PATH_LEN];
  char sensorPath[RKADK_MAX_SENSOR_CNT][RKADK_PATH_LEN];
} RKADK_PARAM_CONTEXT_S;

/**
 * @brief     Parameter Module Init
 * @param[in] globalSetting: global setting ini file path
 * @param[in] sesnorSettingArrary: sensor setting ini file path arrary
 * @return    0 success,non-zero error code.
 */
RKADK_S32 RKADK_PARAM_Init(char *globalSetting, char **sesnorSettingArrary);

/**
 * @brief     Parameter Module Deinit
 * @return    0 success,non-zero error code.
 */
RKADK_S32 RKADK_PARAM_Deinit(RKADK_VOID);

/**
* @brief         get cam related Item Values.
* @param[in]     s32CamID: the specify cam id,valid value range:
* [0,RKADK_PDT_MEDIA_VCAP_DEV_MAX_CNT]
* @param[in]     enType: param type
* @param[in]     pvParam: param value
* @return 0      success,non-zero error code.
*/
RKADK_S32 RKADK_PARAM_GetCamParam(RKADK_S32 s32CamID,
                                  RKADK_PARAM_TYPE_E enParamType,
                                  RKADK_VOID *pvParam);

/**
* @brief         set cam related Item Values.
* @param[in]     enWorkMode: workmode
* @param[in]     s32CamID: the specify cam id,valid value range:
* [0,RKADK_PDT_MEDIA_VCAP_DEV_MAX_CNT]
* @param[in]     enType: param type
* @param[in]     pvParam: param value
* @return        0 success,non-zero error code.
*/
RKADK_S32 RKADK_PARAM_SetCamParam(RKADK_S32 s32CamID,
                                  RKADK_PARAM_TYPE_E enParamType,
                                  const RKADK_VOID *pvParam);

/**
 * @brief        get common parameter configure
 * @param[in]    enParamType : param type
 * @param[out]   pvParam : param value
 * @return       0 success,non-zero error code.
 */
RKADK_S32 RKADK_PARAM_GetCommParam(RKADK_PARAM_TYPE_E enParamType,
                                   RKADK_VOID *pvParam);

/**
 * @brief        set common parameter configure
 * @param[in]    enParamType : param type
 * @param[out]   pvParam : param value
 * @return       0 success,non-zero error code.
 */
RKADK_S32 RKADK_PARAM_SetCommParam(RKADK_PARAM_TYPE_E enParamType,
                                   const RKADK_VOID *pvParam);

/**
 * @brief        defualt all parameters configure
 * @return       0 success,non-zero error code.
 */
RKADK_S32 RKADK_PARAM_SetDefault(RKADK_VOID);

/**
 * @brief        RKADK_PARAM_RES_E to width and height
 * @return       0 success,non-zero error code.
 */
RKADK_S32 RKADK_PARAM_GetResolution(RKADK_PARAM_RES_E type, RKADK_U32 *width,
                                    RKADK_U32 *height);

/**
 * @brief        width and height to RKADK_PARAM_RES_E
 * @return       0 success,non-zero error code.
 */
RKADK_PARAM_RES_E RKADK_PARAM_GetResType(RKADK_U32 width, RKADK_U32 height);

/**
 * @brief        get venc chn id
 * @return       venc chn id success,-1 error code.
 */
RKADK_S32 RKADK_PARAM_GetVencChnId(RKADK_U32 u32CamId,
                                   RKADK_STREAM_TYPE_E enStrmType);

#ifdef __cplusplus
}
#endif
#endif
