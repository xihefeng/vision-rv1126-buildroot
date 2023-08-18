#include <stdio.h>  
#include <cstring>
#include "rkmedia_api.h"

#define s32CamId 0
#define CELING_2_POWER(x,a)  (((x) + ((a)-1)) & (~((a) - 1)))

struct tpoints {
	float x;
	float y;
};

struct plate_data {
	struct tpoints points[4];
	float points_score;
};

struct share_data {
	char http_data[1024];
	char http_data_update;
	struct plate_data tplate[9];
	char plate_data_update;
	int count;
};

struct Session {
	char http_path[64];
	char activation_conf_path[128];
	char licSever_path[128];
	char iq_file_path[128];
	char video_node[64];
	CODEC_TYPE_E VideoType;
	IMAGE_TYPE_E EnImageType;
	RK_U32 EnVideoWidth;
	RK_U32 EnVideoHeight;
	IMAGE_TYPE_E InImageType;
	IMAGE_TYPE_E OutImageType;
	RK_U32 InVideoWidth;
	RK_U32 InVideoHeight;
	RK_U32 OutVideoWidth;
	RK_U32 OutVideoHeight;
};

void dump_cfg(struct Session * session);
int load_cfg(const char *cfg_file, struct Session * session, char mode);
int SAMPLE_COMMON_VI_Start(struct Session *session, char *VideoNode, RK_S32 s32Chnid);
int SAMPLE_COMMON_RGA_Start(struct Session *session, RK_S32 s32Chnid);
int SAMPLE_COMMON_Bind(MOD_ID_E SrcModId, int SrcChnId, MOD_ID_E DestModId, int DestChnId);
int SAMPLE_COMMON_UnBind(MOD_ID_E SrcModId, int SrcChnId, MOD_ID_E DestModId, int DestChnId);
int SAMPLE_COMMON_VENC_Start(struct Session *session, RK_S32 s32Chnid);
