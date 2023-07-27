#include "sample_common_firefly_rkmedia.h"

void dump_cfg(struct Session * session) {
    printf("iq_file_path = %s.\n", session->iq_file_path);
    printf("activation_conf_path = %s.\n", session->activation_conf_path);
    printf("licSever_path = %s.\n", session->licSever_path);
    printf("video_node = %s.\n", session->video_node);
    printf("http_path = %s.\n", session->http_path);
    printf("VideoType = %d.\n", session->VideoType);
    printf("EnImageType = %u.\n", session->EnImageType);
    printf("EnVideoWidth = %u.\n", session->EnVideoWidth);
    printf("EnVideoHeight = %u.\n", session->EnVideoHeight);
    printf("InImageType = %u.\n", session->InImageType);
    printf("OutImageType = %u.\n", session->OutImageType);
    printf("InVideoWidth = %u.\n", session->InVideoWidth);
    printf("InVideoHeight = %u.\n", session->InVideoHeight);
    printf("OutVideoWidth = %u.\n", session->OutVideoWidth);
    printf("OutVideoHeight = %u.\n", session->OutVideoHeight);
}

int load_cfg(const char *cfg_file, struct Session * session, char mode) {
  // cfgline:
  // path=%s VideoType=%d width=%u height=%u
  FILE *fp = fopen(cfg_file, "r");
  char line[2048];

  if (!fp) {
    fprintf(stderr, "open %s failed\n", cfg_file);
    return -1;
  }
  printf("mode = %c\n", mode);
  memset(session, 0, sizeof(Session));
  while (fgets(line, sizeof(line) - 1, fp)) {
    const char *p;
    // char codec_type[20];
    memset(session, 0, sizeof(session));

    if (line[0] == '#' || line[0] == '\n')
      continue;
    if (line[0] == 's' && mode == 's') {
    p = strstr(line, "iq_file_path=");
    if (!p)
      continue;
    if (sscanf(p, "iq_file_path=%s", session->iq_file_path) == 0)
      continue;
    if ((p = strstr(line, "VideoNode="))) {
      if (sscanf(p,
                 "VideoNode=%s VideoType=%d EnImageType=%u EnVideoWidth=%u EnVideoHeight=%u InImageType=%u OutImageType=%u InVideoWidth=%u InVideoHeight=%u OutVideoWidth=%u OutVideoHeight=%u",
                 session->video_node,
                 &session->VideoType,
                 &session->EnImageType,
                 &session->EnVideoWidth,
                 &session->EnVideoHeight,
                 &session->InImageType,
                 &session->OutImageType,
                 &session->InVideoWidth,
                 &session->InVideoHeight,
                 &session->OutVideoWidth,
                 &session->OutVideoHeight) == 0) {
        printf("parse video file failed %s.\n", p);
      }
    }
    if (session->VideoType == RK_CODEC_TYPE_NONE) {
      printf("parse line %s failed\n", line);
    }
    } else if (line[0] == 'c' && mode == 'c') {
    p = strstr(line, "activation_conf_path=");
    if (!p)
      continue;
    if (sscanf(p, "activation_conf_path=%s licSever_path=%s",
		session->activation_conf_path,
		session->licSever_path) == 0)
      continue;
    if ((p = strstr(line, "VideoNode="))) {
      if (sscanf(p,
                 "VideoNode=%s VideoType=%d EnImageType=%u EnVideoWidth=%u EnVideoHeight=%u InImageType=%u OutImageType=%u InVideoWidth=%u InVideoHeight=%u OutVideoWidth=%u OutVideoHeight=%u",
                 session->video_node,
                 &session->VideoType,
                 &session->EnImageType,
                 &session->EnVideoWidth,
                 &session->EnVideoHeight,
                 &session->InImageType,
                 &session->OutImageType,
                 &session->InVideoWidth,
                 &session->InVideoHeight,
                 &session->OutVideoWidth,
                 &session->OutVideoHeight) == 0) {
        printf("parse video file failed %s.\n", p);
      }
    }
    if (session->VideoType == RK_CODEC_TYPE_NONE) {
      printf("parse line %s failed\n", line);
    }
  } else if (line[0] == 'h' && mode == 'h') {
    p = strstr(line, "http_path=");
    if (!p)
      continue;
    if (sscanf(p, "http_path=%s",session->http_path) == 0)
      continue;
  }
  }
  dump_cfg(session);
  fclose(fp);
  return 0;
}

int SAMPLE_COMMON_VI_Start(struct Session *session, char *VideoNode, RK_S32 s32Chnid) {
	int ret;
        VI_CHN_ATTR_S vi_chn_attr;
        vi_chn_attr.pcVideoNode = VideoNode;
        vi_chn_attr.u32BufCnt = 3;
        vi_chn_attr.u32Width = session->InVideoWidth;
        vi_chn_attr.u32Height = session->InVideoHeight;
        vi_chn_attr.enPixFmt = session->InImageType;
        vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
        ret = RK_MPI_VI_SetChnAttr(s32CamId, s32Chnid, &vi_chn_attr);
        ret |= RK_MPI_VI_EnableChn(s32CamId, s32Chnid);
        if (ret) {
                printf("Create vi[%d] failed! ret=%d\n", s32Chnid, ret);
		return -1;
        }
	return 0;
}

int SAMPLE_COMMON_RGA_Start(struct Session *session, RK_S32 s32Chnid) {
	int ret;
	RGA_ATTR_S stRgaAttr;
        memset(&stRgaAttr, 0, sizeof(stRgaAttr));
        stRgaAttr.bEnBufPool = RK_TRUE;
        stRgaAttr.u16BufPoolCnt = 2;
        stRgaAttr.u16Rotaion = 0;
        stRgaAttr.stImgIn.u32X = 0;
        stRgaAttr.stImgIn.u32Y = 0;
        stRgaAttr.stImgIn.imgType = session->InImageType;
        stRgaAttr.stImgIn.u32Width = session->InVideoWidth;
        stRgaAttr.stImgIn.u32Height = session->InVideoHeight;
        stRgaAttr.stImgIn.u32HorStride = session->InVideoWidth;
        stRgaAttr.stImgIn.u32VirStride = session->InVideoHeight;
        stRgaAttr.stImgOut.u32X = 0;
        stRgaAttr.stImgOut.u32Y = 0;
        stRgaAttr.stImgOut.imgType = session->OutImageType;
        stRgaAttr.stImgOut.u32Width = session->OutVideoWidth;
        stRgaAttr.stImgOut.u32Height = session->OutVideoHeight;
        stRgaAttr.stImgOut.u32HorStride = session->OutVideoWidth;
        stRgaAttr.stImgOut.u32VirStride = CELING_2_POWER(session->OutVideoHeight, 16);
        ret = RK_MPI_RGA_CreateChn(s32Chnid, &stRgaAttr);
        if (ret) {
                printf("Create rga[%d] falied! ret=%d\n", s32Chnid, ret);
		return -1;
        }
	return 0;
}
int SAMPLE_COMMON_Bind(MOD_ID_E SrcModId, int SrcChnId, MOD_ID_E DestModId, int DestChnId) {
	int ret;
	char const *rkModId[] = {"RK_ID_UNKNOW",
				 "RK_ID_VB",
				 "RK_ID_SYS",
				 "RK_ID_VDEC",
				 "RK_ID_VENC",
				 "RK_ID_H264E",
				 "RK_ID_JPEGE",
				 "RK_ID_H265E",
				 "RK_ID_VO",
				 "RK_ID_VI",
				 "RK_ID_VP",
				 "RK_ID_AIO",
				 "RK_ID_AI",
				 "RK_ID_AO",
				 "RK_ID_AENC",
				 "RK_ID_ADEC",
				 "RK_ID_ALGO_MD",
				 "RK_ID_ALGO_OD",
				 "RK_ID_RGA",
				 "RK_ID_VMIX",
				 "RK_ID_MUXER",
				 "RK_ID_BUTT"};
        MPP_CHN_S stSrcChn;
        MPP_CHN_S stDestChn;

        printf("#Bind %s[%d] to %s[%d]....\n", rkModId[SrcModId], SrcChnId, rkModId[DestModId], DestChnId);
        stSrcChn.enModId = SrcModId;
        stSrcChn.s32ChnId = SrcChnId;
        stDestChn.enModId = DestModId;
        stDestChn.s32ChnId = DestChnId;
        ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (ret) {
		printf("ERROR: Bind %s[%d] and %s[%d] failed! ret=%d\n", rkModId[SrcModId], SrcChnId, rkModId[DestModId], DestChnId, ret);
        	return -1;
        }
	return 0;
}

int SAMPLE_COMMON_UnBind(MOD_ID_E SrcModId, int SrcChnId, MOD_ID_E DestModId, int DestChnId) {
	int ret;
	char const *rkModId[] = {"RK_ID_UNKNOW", 
				 "RK_ID_VB",
				 "RK_ID_SYS",
				 "RK_ID_VDEC",
				 "RK_ID_VENC",
				 "RK_ID_H264E",
				 "RK_ID_JPEGE",
				 "RK_ID_H265E",
				 "RK_ID_VO",
				 "RK_ID_VI",
				 "RK_ID_AIO",
				 "RK_ID_AI",
				 "RK_ID_AO",
				 "RK_ID_AENC",
				 "RK_ID_ADEC",
				 "RK_ID_ALGO_MD",
				 "RK_ID_ALGO_OD",
				 "RK_ID_RGA"};
        MPP_CHN_S stSrcChn;
        MPP_CHN_S stDestChn;

        printf("#UnBind %s[%d] to %s[%d]....\n", rkModId[SrcModId], SrcChnId, rkModId[DestModId], DestChnId);
        stSrcChn.enModId = SrcModId;
        stSrcChn.s32ChnId = SrcChnId;
        stDestChn.enModId = DestModId;
        stDestChn.s32ChnId = DestChnId;
        ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
        if (ret) {
		printf("ERROR: UnBind %s[%d] and %s[%d] failed! ret=%d\n", rkModId[SrcModId], SrcChnId, rkModId[DestModId], DestChnId, ret);
        	return -1;
        }
	return 0;
}

int SAMPLE_COMMON_VENC_Start(struct Session *session, RK_S32 s32Chnid) {
	int ret;
	// 这里可以学习 Venc 硬件编码器数据结构配置
	VENC_CHN_ATTR_S venc_chn_attr; 
	memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
	venc_chn_attr.stVencAttr.enType = session->VideoType;
	venc_chn_attr.stVencAttr.imageType = session->EnImageType; //输入图片的格式，和 VI 输出保持一致
	venc_chn_attr.stVencAttr.u32PicWidth = session->EnVideoWidth; //编码图像宽度，单位像素点
	venc_chn_attr.stVencAttr.u32PicHeight = session->EnVideoHeight;//编码图像高度，单位像素点
	venc_chn_attr.stVencAttr.u32VirWidth =  session->EnVideoWidth;//stride 宽度，必须 16 对齐
	venc_chn_attr.stVencAttr.u32VirHeight = CELING_2_POWER(session->EnVideoHeight, 16);// stride 高度，必须 16 对齐
	venc_chn_attr.stVencAttr.u32Profile = 77; //编码等级 77,是中级 66,基础等级，100,高级

	switch(venc_chn_attr.stVencAttr.enType) {
	case RK_CODEC_TYPE_H264:
		venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
		venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = 30;
		venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = session->EnVideoWidth * session->EnVideoHeight;
		venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
		venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 25;
		venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
		venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 25;
		break;
	case RK_CODEC_TYPE_H265:
		venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
		venc_chn_attr.stRcAttr.stH265Vbr.u32Gop = 30;
		venc_chn_attr.stRcAttr.stH265Vbr.u32MaxBitRate = session->EnVideoWidth * session->EnVideoHeight;
		venc_chn_attr.stRcAttr.stH265Vbr.fr32DstFrameRateDen = 1;
		venc_chn_attr.stRcAttr.stH265Vbr.fr32DstFrameRateNum = 25;
		venc_chn_attr.stRcAttr.stH265Vbr.u32SrcFrameRateDen = 1;
		venc_chn_attr.stRcAttr.stH265Vbr.u32SrcFrameRateNum = 25;
		break;
	default:
		printf("error: video codec not support.\n");
		break;
	}

	ret = RK_MPI_VENC_CreateChn(s32Chnid, &venc_chn_attr);//创建通道
	if (ret) {
		printf("ERROR: create VENC[%d] error! ret=%d\n", s32Chnid, ret);
		return -1;
	}
	return 0;
}
