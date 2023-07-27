#include <stdio.h>  
#include <cstring>
#include <string>
#include <vector>
#include "rkmedia_api.h"
#include "rockx/rockx.h"

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
	char rockx_data_update;
	rockx_keypoints_array_t body_array;
	rockx_keypoints_t finger;
	rockx_object_array_t head_array;
	rockx_object_array_t face_array;
	rockx_face_attribute_t gender_age;
	rockx_object_array_t out_track_objects;
	rockx_face_landmark_t out_landmark;
        rockx_face_angle_t out_angle;
	float similarity;
	char person_name[10];
	rockx_object_array_t person_array;
	rockx_face_mask_array_t face_mask_array;
};

const std::vector<std::pair<int,int>> posePairs_v1 = {
	{1,2}, {1,5}, {2,3}, {3,4}, {5,6}, {6,7},
	{1,8}, {8,9}, {9,10}, {1,11}, {11,12}, {12,13},
	{1,0}, {0,14}, {14,16}, {0,15}, {15,17}
};

const std::vector<std::pair<int,int>> posePairs_v2 = {
	{2,3}, {3,4}, {5,6}, {6,7},
	{8,9}, {9,10}, {11,12}, {12,13},
	{1,0}, {0,14}, {14,16}, {0,15}, {15,17},
	{2,5}, {8,11}, {2,8}, {5,11}
};

const std::vector<std::pair<int,int>> posePairs_finger = {
	{0,1}, {1,2}, {2,3}, {3,4}, {0,5}, {5,6}, {6,7},
	{7,8}, {0,9}, {9,10}, {10,11}, {11,12}, {0,13}, {13,14},
	{14,15}, {15,16}, {0,17}, {17,18}, {18,19}, {19,20}
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
