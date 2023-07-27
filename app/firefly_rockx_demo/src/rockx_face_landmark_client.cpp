#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>   
#include <pthread.h>
#include <sys/time.h>
#include <string>
#include <iostream>
#include <vector>

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

#include <sys/types.h>
#include <sys/un.h>

#include <fstream>

#ifdef RKAIQ
#include "sample_common.h"
#endif
#include "rkmedia_api.h"
#include "sample_common_firefly_rkmedia.h"

#include <sys/stat.h>
#include <sys/mman.h>

#include <memory.h>
#include "rockx/rockx.h"

struct share_data *posixsm_share;
MEDIA_BUFFER FaceLandmarkBuffer;
rockx_config_t rockx_configs;
volatile int FaceLandmarkGetFlag = 0;

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

void FaceLandmarkCb(MEDIA_BUFFER buffer)
{
	if(FaceLandmarkGetFlag == 0) {
		FaceLandmarkBuffer = RK_MPI_MB_Copy(buffer, RK_TRUE);
		FaceLandmarkGetFlag = 1;
	}
	RK_MPI_MB_ReleaseBuffer(buffer);
}

static void *FaceLandmarkStream(void *data) {
	struct Session * session = (Session *)data;
	int ret;

	ret = SAMPLE_COMMON_RGA_Start(session, 0);
	if(ret) {
		return NULL;
	}

	MPP_CHN_S stRgaChn0;
	stRgaChn0.enModId = RK_ID_RGA;
	stRgaChn0.s32ChnId = 0;
	RK_MPI_SYS_RegisterOutCb(&stRgaChn0, FaceLandmarkCb);

	ret = SAMPLE_COMMON_Bind(RK_ID_VI, 1, RK_ID_RGA, 0);
	if(ret) {
		return NULL;
	}

	int landmark_count = 68;
	rockx_module_t rockx_module;
	if (landmark_count == 5) {
		rockx_module = ROCKX_MODULE_FACE_LANDMARK_5;
	} else if (landmark_count == 68) {
		rockx_module = ROCKX_MODULE_FACE_LANDMARK_68;
	} else {
		printf("error landmark count must 5 or 68\n");
		return NULL;
	}

	rockx_handle_t face_det_handle;

	// create a face detection handle
	ret = rockx_create(&face_det_handle, ROCKX_MODULE_FACE_DETECTION, &rockx_configs, sizeof(rockx_config_t));
	if (ret != ROCKX_RET_SUCCESS) {
		printf("init rockx module ROCKX_MODULE_FACE_DETECTION error %d\n", ret);
		return NULL;
	}

	rockx_handle_t face_landmark_handle;

	// create a face landmark handle
	ret = rockx_create(&face_landmark_handle, rockx_module, &rockx_configs, sizeof(rockx_config_t));
	if (ret != ROCKX_RET_SUCCESS) {
		printf("init rockx module ROCKX_MODULE_FACE_LANDMARK_68 error %d\n", ret);
		return NULL;
	}
	// create rockx_face_array_t for store result
	rockx_object_array_t face_array;
	memset(&face_array, 0, sizeof(rockx_object_array_t));

        rockx_face_landmark_t out_landmark;
        rockx_face_angle_t out_angle;

	rockx_image_t input_image;
	input_image.size = session->OutVideoWidth * session->OutVideoHeight * 3;
	input_image.width = session->OutVideoWidth;
	input_image.height = session->OutVideoHeight;
	input_image.pixel_format = ROCKX_PIXEL_FORMAT_RGB888;
	while (!quit) {
		if(FaceLandmarkGetFlag == 1) {
			if (RK_MPI_MB_GetPtr(FaceLandmarkBuffer) == NULL){
				FaceLandmarkGetFlag = 0;
        			continue;
			}

			input_image.data = (uint8_t *)RK_MPI_MB_GetPtr(FaceLandmarkBuffer);

			// detect face
			ret = rockx_face_detect(face_det_handle, &input_image, &face_array, nullptr);
			if (ret != ROCKX_RET_SUCCESS) {
				printf("rockx_face_detect error %d\n", ret);
				return NULL;
			}

			/*************** FACE Landmark ***************/

			// detect landmark for every face
			for (int i = 0; i < face_array.count; i++) {
				memset(&out_landmark, 0, sizeof(rockx_face_landmark_t));
				memset(&out_angle, 0, sizeof(rockx_face_angle_t));

				ret = rockx_face_landmark(face_landmark_handle, &input_image, &face_array.object[i].box, &out_landmark);
/*
				if (landmark_count == 5) {
					rockx_image_t out_img;
					memset(&out_img, 0, sizeof(rockx_image_t));
					ret = rockx_face_align(face_landmark_handle, &input_image, &face_array.object[i].box, &out_landmark, &out_img);
//					rockx_image_write("./aligned.jpg", &out_img);
					rockx_image_release(&out_img);
				}
*/
				ret = rockx_face_pose(&out_landmark, &out_angle);
/*
				printf("face %d:\n landmarks_count=%d landmark_score=%f angle=(pitch:%f yaw:%f roll:%f)\n", i,
					out_landmark.landmarks_count, out_landmark.score,
					out_angle.pitch, out_angle.yaw, out_angle.roll);
				for(int j = 0; j < out_landmark.landmarks_count; j++) {
					printf("landmark %d %d %d\n", j, out_landmark.landmarks[j].x, out_landmark.landmarks[j].y);
					//rockx_image_draw_circle(&input_image, {out_landmark.landmarks[j].x, out_landmark.landmarks[j].y}, 2, {0, 255, 0}, -1);
				}
*/
			}

			if (posixsm_share->rockx_data_update == 0) {
				memcpy(&posixsm_share->face_array, &face_array, sizeof(rockx_object_array_t));
				memcpy(&posixsm_share->out_landmark, &out_landmark, sizeof(rockx_face_landmark_t));
				memcpy(&posixsm_share->out_angle, &out_angle, sizeof(rockx_face_angle_t));
				// process result
				for (int i = 0; i < posixsm_share->face_array.count; i++) {
					posixsm_share->face_array.object[i].box.left *= (float)((float)session->EnVideoWidth / session->OutVideoWidth);
					posixsm_share->face_array.object[i].box.top *= (float)((float)session->EnVideoHeight / session->OutVideoHeight);
					posixsm_share->face_array.object[i].box.right *= (float)((float)session->EnVideoWidth / session->OutVideoWidth);
					posixsm_share->face_array.object[i].box.bottom *= (float)((float)session->EnVideoHeight / session->OutVideoHeight);
				}
        			for(int j = 0; j < out_landmark.landmarks_count; j++) {
					posixsm_share->out_landmark.landmarks[j].x *= (float)((float)session->EnVideoWidth / session->OutVideoWidth);
					posixsm_share->out_landmark.landmarks[j].y *= (float)((float)session->EnVideoHeight / session->OutVideoHeight);
				}
				posixsm_share->rockx_data_update = 1;
			}
    			RK_MPI_MB_ReleaseBuffer(FaceLandmarkBuffer);
    			FaceLandmarkGetFlag = 0;
    		}
	}
	// release handle
	rockx_destroy(face_det_handle);
	rockx_destroy(face_landmark_handle);
	return NULL;
}

int main(int argc, char **argv)  
{  
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
	load_cfg(cfg_file_path, session_cfg, 'c');

	RK_MPI_SYS_Init();

	ret = SAMPLE_COMMON_VI_Start(session_cfg, session_cfg->video_node, 1);
	if(ret) {
		return -1;
	}
	//初始化 rockx 数据库
	rockx_add_config(&rockx_configs, (char *)ROCKX_CONFIG_DATA_PATH, "/usr/share/rockx-data-rv1109/");

	int fd = shm_open("posixsm", O_RDWR, 0666);
	ftruncate(fd, sizeof(struct share_data));
	posixsm_share = (struct share_data *)mmap(NULL, sizeof(struct share_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	posixsm_share->rockx_data_update = 0;

	pthread_t face_landmark_thread;
	pthread_create(&face_landmark_thread, NULL, FaceLandmarkStream, session_cfg);//车牌检测

	while (!quit) {
		usleep(100000);
	}
	munmap(posixsm_share, sizeof(struct share_data)); //取消内存映射
	SAMPLE_COMMON_UnBind(RK_ID_VI, 1, RK_ID_RGA, 0);
	SAMPLE_COMMON_UnBind(RK_ID_VI, 1, RK_ID_RGA, 1);
	RK_MPI_RGA_DestroyChn(0);
	RK_MPI_RGA_DestroyChn(1);
	RK_MPI_VI_DisableChn(s32CamId, 1);
	pthread_join(face_landmark_thread, NULL);
	return 0;
}
