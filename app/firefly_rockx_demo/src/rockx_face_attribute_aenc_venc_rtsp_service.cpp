#include <sys/time.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

#include "xhlpr_api.h"
#include "xhlpr_type.h"

#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include "librtsp/rtsp_demo.h"

#include <sys/types.h>
#include <sys/un.h>

#ifdef RKAIQ
#include "sample_common.h"
#endif
#include "sample_common.h"
#include "sample_common_firefly_rkmedia.h"
#include "rkmedia_api.h"

#include <fstream>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>

MEDIA_BUFFER DrawLastBuffer;
MEDIA_BUFFER DrawNowBuffer;

volatile int DrawGetFlag = 0;

unsigned long get_time(void)
{
        struct timeval ts;
        gettimeofday(&ts, NULL);
        return (ts.tv_sec * 1000 + ts.tv_usec / 1000);
}

static bool quit = false;
static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  quit = true;
}

static RK_CHAR optstr[] = "?::M:c:";
static const struct option long_options[] = {
    {"aiq", optional_argument, NULL, 'a'},
    {"camid", required_argument, NULL, 'I'},
    {"multictx", required_argument, NULL, 'M'},
    {"help", optional_argument, NULL, '?'},
    {NULL, 0, NULL, 0},
};

static void print_usage(const RK_CHAR *name) {
  printf("usage example:\n");
#ifdef RKAIQ
  printf("\t%s [-a [iqfiles_dir]]"
         "[-I 0] "
         "[-M 0] "
         "\n",
         name);
  printf("\t-a | --aiq: enable aiq with dirpath provided, eg:-a "
         "/oem/etc/iqfiles/, "
         "set dirpath empty to using path by default, without this option aiq "
         "should run in other application\n");
  printf("\t-M | --multictx: switch of multictx in isp, set 0 to disable, set "
         "1 to enable. Default: 0\n");
#else
  printf("\t%s [-I 0]\n", name);
#endif
  printf("\t-I | --camid: camera ctx id, Default 0\n");
}

rtsp_demo_handle g_rtsplive = NULL;
static rtsp_session_handle g_rtsp_session;

rtsp_demo_handle g_audiortsplive = NULL;
static rtsp_session_handle g_audiortsp_session;

typedef struct AudioParams_ {
  RK_U32 u32SampleRate;
  RK_U32 u32ChnCnt;
  SAMPLE_FORMAT_E enSampleFmt;
  const RK_CHAR *pOutPath;
} AudioParams;


#define AAC_PROFILE_LOW 1

typedef struct AacFreqIdx_ {
  RK_S32 u32SmpRate;
  RK_U8 u8FreqIdx;
} AacFreqIdx;

AacFreqIdx AacFreqIdxTbl[13] = {{96000, 0}, {88200, 1}, {64000, 2},  {48000, 3},
                                {44100, 4}, {32000, 5}, {24000, 6},  {22050, 7},
                                {16000, 8}, {12000, 9}, {11025, 10}, {8000, 11},
                                {7350, 12}};

static void GetAdtsHeader(RK_U8 *pu8AdtsHdr, RK_S32 u32SmpRate, RK_U8 u8Channel,
                          RK_U32 u32DataLen) {
  RK_U8 u8FreqIdx = 0;
  for (int i = 0; i < 13; i++) {
    if (u32SmpRate == AacFreqIdxTbl[i].u32SmpRate) {
      u8FreqIdx = AacFreqIdxTbl[i].u8FreqIdx;
      break;
    }
  }

  RK_U32 u32PacketLen = u32DataLen + 7;
  pu8AdtsHdr[0] = 0xFF;
  pu8AdtsHdr[1] = 0xF1;
  pu8AdtsHdr[2] =
      ((AAC_PROFILE_LOW) << 6) + (u8FreqIdx << 2) + (u8Channel >> 2);
  pu8AdtsHdr[3] = (((u8Channel & 3) << 6) + (u32PacketLen >> 11));
  pu8AdtsHdr[4] = ((u32PacketLen & 0x7FF) >> 3);
  pu8AdtsHdr[5] = (((u32PacketLen & 7) << 5) + 0x1F);
  pu8AdtsHdr[6] = 0xFC;
}

static void *AudioRtspStream(void *data) {
//	AudioParams *pstAudioParams = (AudioParams *)data;
	RK_U8 aac_header[7];
	int ret = 0;

	RK_U32 u32SampleRate = 16000;
	RK_U32 u32BitRate = 64000; // 64kbps
	RK_U32 u32ChnCnt = 2;
	RK_U32 u32FrameCnt = 1152; // always 1024 for aac
	SAMPLE_FORMAT_E enSampleFmt = RK_SAMPLE_FMT_S16;
	// default:CARD=rockchiprk809co
	RK_CHAR *pDeviceName = "default";
	RK_CHAR *pOutPath = "/tmp/aenc.adts";

	AudioParams stAudioParams;
	stAudioParams.u32SampleRate = u32SampleRate;
	stAudioParams.u32ChnCnt = u32ChnCnt;
	stAudioParams.enSampleFmt = enSampleFmt;
	stAudioParams.pOutPath = pOutPath;    

	MPP_CHN_S mpp_chn_ai, mpp_chn_aenc;
	mpp_chn_ai.enModId = RK_ID_AI;
	mpp_chn_ai.s32ChnId = 0;
	mpp_chn_aenc.enModId = RK_ID_AENC;
	mpp_chn_aenc.s32ChnId = 0;

	// 1. create AI
	AI_CHN_ATTR_S ai_attr;
	ai_attr.pcAudioNode = pDeviceName;
	ai_attr.enSampleFormat = enSampleFmt;
	ai_attr.u32NbSamples = u32FrameCnt;
	ai_attr.u32SampleRate = u32SampleRate;
	ai_attr.u32Channels = u32ChnCnt;
	ai_attr.enAiLayout = AI_LAYOUT_NORMAL;
	ret = RK_MPI_AI_SetChnAttr(mpp_chn_ai.s32ChnId, &ai_attr);
	ret |= RK_MPI_AI_EnableChn(mpp_chn_ai.s32ChnId);
	if (ret) {
		printf("Create AI[0] failed! ret=%d\n", ret);
		return NULL;
	}

	// 2. create AENC
	AENC_CHN_ATTR_S aenc_attr;
	aenc_attr.enCodecType = RK_CODEC_TYPE_MP2;
	aenc_attr.u32Bitrate = u32BitRate;
	aenc_attr.u32Quality = 1;
	aenc_attr.stAencMP3.u32Channels = u32ChnCnt;
	aenc_attr.stAencMP3.u32SampleRate = u32SampleRate;
	ret = RK_MPI_AENC_CreateChn(mpp_chn_aenc.s32ChnId, &aenc_attr);
	if (ret) {
		printf("Create AENC[0] failed! ret=%d\n", ret);
		return NULL;
	}

	// 3. bind AI-AENC
	ret = RK_MPI_SYS_Bind(&mpp_chn_ai, &mpp_chn_aenc);
	if (ret) {
		printf("Bind AI[0] to AENC[0] failed! ret=%d\n", ret);
		return NULL;
	}

	MEDIA_BUFFER buffer;
	while(!quit) {
		buffer = RK_MPI_SYS_GetMediaBuffer(
				RK_ID_AENC, 0, -1);
		if (!buffer)
			continue;

		GetAdtsHeader(aac_header, u32SampleRate,
				u32ChnCnt, RK_MPI_MB_GetSize(buffer));
		char *adts_aac = (char *)malloc(RK_MPI_MB_GetSize(buffer) + sizeof(aac_header));

		memcpy(adts_aac, aac_header, 7);
		adts_aac += 7; //偏移 7 位来接收 aac 裸流数据
		memcpy(adts_aac, RK_MPI_MB_GetPtr(buffer), RK_MPI_MB_GetSize(buffer));
		adts_aac -= 7; //偏移回退 7 位来指向帧的头部数据

		if (g_audiortsplive && g_audiortsp_session) {
			rtsp_tx_audio(g_audiortsp_session, (unsigned char *)adts_aac, (RK_MPI_MB_GetSize(buffer) + 7),
					RK_MPI_MB_GetTimestamp(buffer));
			rtsp_do_event(g_audiortsplive);
		}

		RK_MPI_MB_ReleaseBuffer(buffer);
	}
	return NULL;
}

static void *VideoRtspStream(void *data) {
	struct Session * session = (Session *)data;
	int ret = 0;

	session->InImageType = IMAGE_TYPE_RGB888;
	session->OutImageType = IMAGE_TYPE_NV12;
	ret = SAMPLE_COMMON_RGA_Start(session, 1);
	if(ret) {
		return NULL;
	}

	ret = SAMPLE_COMMON_VENC_Start(session, 0);
	if(ret) {
		return NULL;
	}

	ret = SAMPLE_COMMON_Bind(RK_ID_RGA, 1, RK_ID_VENC, 0);
	if(ret) {
		return NULL;
	}

	MEDIA_BUFFER buffer;
	while(!quit) {
		buffer = RK_MPI_SYS_GetMediaBuffer(
				RK_ID_VENC, 0, -1);
		if (!buffer)
			continue;

		if (g_rtsplive && g_rtsp_session) {
			rtsp_tx_video(g_rtsp_session, (unsigned char*)RK_MPI_MB_GetPtr(buffer), RK_MPI_MB_GetSize(buffer),
					RK_MPI_MB_GetTimestamp(buffer));
			rtsp_do_event(g_rtsplive);
		}

		RK_MPI_MB_ReleaseBuffer(buffer);
	}
	return NULL;
}


void DrawCb(MEDIA_BUFFER buffer)
{
	if(DrawGetFlag == 0) {
		DrawLastBuffer = RK_MPI_MB_Copy(buffer, RK_TRUE);
		DrawGetFlag = 1;
	} else if (DrawGetFlag == 1) {
		DrawNowBuffer = RK_MPI_MB_Copy(buffer, RK_TRUE);
		DrawGetFlag = 2;
	}
	RK_MPI_MB_ReleaseBuffer(buffer);
}

static void *DrawStream(void *data) {
	struct Session * session = (Session *)data;
	int ret = 0;
	cv::Mat * img = NULL;

	int fd = shm_open("posixsm", O_CREAT | O_RDWR, 0666);
	ftruncate(fd, sizeof(struct share_data));
	struct share_data *posixsm_share = (struct share_data *)mmap(NULL, sizeof(struct share_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	posixsm_share->http_data_update = 0;
	posixsm_share->rockx_data_update = 0;

	session->InImageType = IMAGE_TYPE_NV12;
	session->OutImageType = IMAGE_TYPE_RGB888;
	ret = SAMPLE_COMMON_RGA_Start(session, 0);
	if(ret) {
		return NULL;
	}

	MPP_CHN_S stRgaChn0;
	stRgaChn0.enModId = RK_ID_RGA;
	stRgaChn0.s32ChnId = 0;
	RK_MPI_SYS_RegisterOutCb(&stRgaChn0, DrawCb);

	ret = SAMPLE_COMMON_Bind(RK_ID_VI, 0, RK_ID_RGA, 0);
	if(ret) {
		return NULL;
	}

	while(!quit) {
		if(DrawGetFlag == 2) {
			RK_MPI_MB_BeginCPUAccess(DrawLastBuffer, RK_FALSE);
			if (posixsm_share->rockx_data_update == 1) {
					img = new cv::Mat(cv::Size(session->EnVideoWidth, session->EnVideoHeight), CV_8UC3,
						(char *)RK_MPI_MB_GetPtr(DrawLastBuffer));

				// process result
				for (int i = 0; i < posixsm_share->face_array.count; i++) {
					if(posixsm_share->face_array.object[i].score > 0.8) {
						cv::rectangle(*img, 
							cv::Point(posixsm_share->face_array.object[i].box.left, posixsm_share->face_array.object[i].box.top), 
							cv::Point(posixsm_share->face_array.object[i].box.right, posixsm_share->face_array.object[i].box.bottom), 
							cv::Scalar(0, 0, 255), 2);
						cv::putText(*img, std::to_string(posixsm_share->face_array.object[i].score),
							cv::Point(posixsm_share->face_array.object[i].box.left, posixsm_share->face_array.object[i].box.top - 4),
							2, 1, cv::Scalar(255, 255, 255));

						cv::putText(*img, "age:",
							cv::Point(posixsm_share->face_array.object[i].box.left, posixsm_share->face_array.object[i].box.top - 30),
							2, 1, cv::Scalar(255, 255, 255));
						cv::putText(*img, std::to_string(posixsm_share->gender_age.age),
							cv::Point(posixsm_share->face_array.object[i].box.left + 80, posixsm_share->face_array.object[i].box.top - 30),
							2, 1, cv::Scalar(255, 255, 255));
					cv::putText(*img, "sex:",
							cv::Point(posixsm_share->face_array.object[i].box.left, posixsm_share->face_array.object[i].box.top - 60),
							2, 1, cv::Scalar(255, 255, 255));
						if(posixsm_share->gender_age.gender == 1) {
							cv::putText(*img, "man",
								cv::Point(posixsm_share->face_array.object[i].box.left + 80, posixsm_share->face_array.object[i].box.top - 60),
								2, 1, cv::Scalar(255, 255, 255));
						} else if (posixsm_share->gender_age.gender == 2) {
							cv::putText(*img, "woman",
								cv::Point(posixsm_share->face_array.object[i].box.left + 80, posixsm_share->face_array.object[i].box.top - 60),
								2, 1, cv::Scalar(255, 255, 255));
						}
					}
				}
				posixsm_share->rockx_data_update = 0;
			}
			RK_MPI_MB_EndCPUAccess(DrawLastBuffer, RK_FALSE);
			RK_MPI_SYS_SendMediaBuffer(RK_ID_RGA, 1, DrawLastBuffer);
			//delete img;
			RK_MPI_MB_ReleaseBuffer(DrawLastBuffer);//拷贝会重新申请内存，所以拷贝前需要将 buffer release
			DrawLastBuffer = RK_MPI_MB_Copy(DrawNowBuffer, RK_TRUE);
			RK_MPI_MB_ReleaseBuffer(DrawNowBuffer);
			DrawGetFlag = 1;
		} else {
			usleep(5000);
		}
	}
	munmap(posixsm_share, sizeof(struct share_data)); //取消内存映射
	return NULL;
}

int main(int argc, char *argv[]) {

	int ret = 0;
	struct Session *session_cfg;
	session_cfg = (Session *)malloc(sizeof(Session));

	char *cfg_file_path = NULL;
#ifdef RKAIQ
	RK_BOOL bMultictx = RK_FALSE;
#endif
	int c;
	while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
		const char *tmp_optarg = optarg;
		switch (c) {
			case 'c':
				if (!optarg && NULL != argv[optind] && '-' != argv[optind][0]) {
					tmp_optarg = argv[optind++];
				}
				if (tmp_optarg) {
					cfg_file_path = (char *)tmp_optarg;
				} else {
					cfg_file_path = (char *)"/usr/share/xhlpr_app/xhlpr_app.cfg";
				}
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

	load_cfg(cfg_file_path, session_cfg, 's');
	printf("#CameraIdx: %d\n\n", s32CamId);
	if (session_cfg->iq_file_path) {
#ifdef RKAIQ
		printf("#Rkaiq XML DirPath: %s\n", session_cfg->iq_file_path);
		printf("#bMultictx: %d\n\n", bMultictx);
		rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
		int fps = 25;
		SAMPLE_COMM_ISP_Init(s32CamId, hdr_mode, bMultictx, session_cfg->iq_file_path);
		SAMPLE_COMM_ISP_Run(s32CamId);
		SAMPLE_COMM_ISP_SetFrameRate(s32CamId, fps);
		SAMPLE_COMM_ISP_SET_mirror(s32CamId, 1);
#endif
	}
	

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

	ret = SAMPLE_COMMON_VI_Start(session_cfg, session_cfg->video_node, 0);
	if(ret) {
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

	// init rtsp
	g_rtsplive = create_rtsp_demo(8554);
	g_rtsp_session = rtsp_new_session(g_rtsplive, "/H264_stream_0");
	CODEC_TYPE_E enCodecType = session_cfg->VideoType;
	if (enCodecType == RK_CODEC_TYPE_H264) {
		rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H264, NULL, 0); 
	} else if (enCodecType == RK_CODEC_TYPE_H265) {
		rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H265, NULL, 0); 
	} else {
		printf("not support other type\n");
		return -1;
	}
	rtsp_sync_video_ts(g_rtsp_session, rtsp_get_reltime(), rtsp_get_ntptime());


	g_audiortsplive = create_rtsp_demo(8555);
	g_audiortsp_session = rtsp_new_session(g_audiortsplive, "/audio_stream_0");

	rtsp_set_audio(g_audiortsp_session, RTSP_CODEC_ID_AUDIO_MP3, NULL, 0); 
	rtsp_sync_audio_ts(g_audiortsp_session, rtsp_get_reltime(), rtsp_get_ntptime());


	pthread_t draw_thread;
	pthread_create(&draw_thread, NULL, DrawStream, session_cfg);

	usleep(50000);
	pthread_t video_rtsp_thread;
	pthread_create(&video_rtsp_thread, NULL, VideoRtspStream, session_cfg);

	pthread_t audio_rtsp_thread;
	pthread_create(&audio_rtsp_thread, NULL, AudioRtspStream, NULL);

	while (!quit) {
		usleep(100000);
	}

	SAMPLE_COMMON_UnBind(RK_ID_VI, 0, RK_ID_RGA, 0);
	SAMPLE_COMMON_UnBind(RK_ID_RGA, 1, RK_ID_VENC, 0);
	RK_MPI_VENC_DestroyChn(0);
	RK_MPI_RGA_DestroyChn(0);
	RK_MPI_RGA_DestroyChn(1);
	RK_MPI_VI_DisableChn(s32CamId, 0);
	pthread_join(video_rtsp_thread, NULL);
	pthread_join(audio_rtsp_thread, NULL);
	pthread_join(draw_thread, NULL);
	return 0;
}
