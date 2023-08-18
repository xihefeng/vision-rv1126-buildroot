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
int count;
volatile int plate_ocr_count = 0;
volatile char plate_ocr_flag = 1; //车牌号识别数据拷贝标志位

MEDIA_BUFFER PlateDetectBuffer;
MEDIA_BUFFER PlateOCRBuffer;

volatile int PlateDetectGetFlag = 0;
volatile int PlateOCRGetFlag = 0;

rockx_object_array_t carplate_ocr_array;
rockx_config_t rockx_configs;

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

void PlateDetectCb(MEDIA_BUFFER buffer)
{
	if(PlateDetectGetFlag == 0) {
		PlateDetectBuffer = RK_MPI_MB_Copy(buffer, RK_TRUE);
		PlateDetectGetFlag = 1;
	}
	RK_MPI_MB_ReleaseBuffer(buffer);
}

void PlateOCRCb(MEDIA_BUFFER buffer)
{
	if(PlateOCRGetFlag == 0) {
		PlateOCRBuffer = RK_MPI_MB_Copy(buffer, RK_TRUE);
		PlateOCRGetFlag = 1;
	}
	RK_MPI_MB_ReleaseBuffer(buffer);
}

static void *PlateDetectStream(void *data) {
	struct Session * session = (Session *)data;
	int ret;

	ret = SAMPLE_COMMON_RGA_Start(session, 0);
	if(ret) {
		return NULL;
	}

	MPP_CHN_S stRgaChn0;
	stRgaChn0.enModId = RK_ID_RGA;
	stRgaChn0.s32ChnId = 0;
	RK_MPI_SYS_RegisterOutCb(&stRgaChn0, PlateDetectCb);

	ret = SAMPLE_COMMON_Bind(RK_ID_VI, 1, RK_ID_RGA, 0);
	if(ret) {
		return NULL;
	}
	rockx_handle_t carplate_det_handle;
	ret = rockx_create(&carplate_det_handle, ROCKX_MODULE_CARPLATE_DETECTION, &rockx_configs, sizeof(rockx_config_t));
	if (ret != ROCKX_RET_SUCCESS) {
		printf("init rockx module ROCKX_MODULE_CARPLATE_DETECTION error %d\n", ret);
		return NULL;
	}

	rockx_object_array_t carplate_det_array;
	memset(&carplate_det_array, 0, sizeof(rockx_object_array_t));
	
	while (!quit) {
		if(PlateDetectGetFlag == 1) {
			if (RK_MPI_MB_GetPtr(PlateDetectBuffer) == NULL){
				PlateDetectGetFlag = 0;
        			continue;
			}

			rockx_image_t input_image;
			input_image.data = (uint8_t *)RK_MPI_MB_GetPtr(PlateDetectBuffer);
			input_image.size = session->OutVideoWidth * session->OutVideoHeight * 3;
			input_image.width = session->OutVideoWidth;
			input_image.height = session->OutVideoHeight;
			input_image.pixel_format = ROCKX_PIXEL_FORMAT_RGB888;

			// detect carplate
			ret = rockx_carplate_detect(carplate_det_handle, &input_image, &carplate_det_array, nullptr);
			if (ret != ROCKX_RET_SUCCESS) {
				printf("rockx_carplate_detect error %d\n", ret);
				return NULL;
			}

			if (plate_ocr_flag == 1) {
				plate_ocr_flag = 0;
				plate_ocr_count = carplate_det_array.count;
				memcpy(&carplate_ocr_array, &carplate_det_array, (sizeof(rockx_object_array_t)));
				for(int j = 0; j < carplate_det_array.count; j++) {
					//memcpy(plate_ocr + j, plate_det + j, (sizeof(PlateInfo)));
				}
			}
			if (posixsm_share->plate_data_update == 0) {
				posixsm_share->count = carplate_det_array.count;
				for(int j = 0; j < carplate_det_array.count; j++) {
					// 0~4 的坐标分别是矩形框的左上角，右上角，右下角，左下角
					posixsm_share->tplate[j].points[0].x = (float)((float)carplate_det_array.object[j].box.left * (float)((float)session->EnVideoWidth / session->OutVideoWidth));
					posixsm_share->tplate[j].points[0].y = (float)((float)carplate_det_array.object[j].box.bottom * (float)((float)session->EnVideoHeight / session->OutVideoHeight));
					posixsm_share->tplate[j].points[1].x = (float)((float)carplate_det_array.object[j].box.right * (float)((float)session->EnVideoWidth / session->OutVideoWidth));
					posixsm_share->tplate[j].points[1].y = (float)((float)carplate_det_array.object[j].box.bottom * (float)((float)session->EnVideoHeight / session->OutVideoHeight));
					posixsm_share->tplate[j].points[2].x = (float)((float)carplate_det_array.object[j].box.right * (float)((float)session->EnVideoWidth / session->OutVideoWidth));
					posixsm_share->tplate[j].points[2].y = (float)((float)carplate_det_array.object[j].box.top * (float)((float)session->EnVideoHeight / session->OutVideoHeight));
					posixsm_share->tplate[j].points[3].x = (float)((float)carplate_det_array.object[j].box.left * (float)((float)session->EnVideoWidth / session->OutVideoWidth));
					posixsm_share->tplate[j].points[3].y = (float)((float)carplate_det_array.object[j].box.top * (float)((float)session->EnVideoHeight / session->OutVideoHeight));
					/*
					for(int p = 0; p < 4; p++) {
						printf("(%f, %f)\n", posixsm_share->tplate[j].points[p].x, posixsm_share->tplate[j].points[p].y);
					}
					*/
					posixsm_share->tplate[j].points_score = carplate_det_array.object[j].score;
				}
				posixsm_share->plate_data_update = 1;
			}
			RK_MPI_MB_ReleaseBuffer(PlateDetectBuffer);
			PlateDetectGetFlag = 0;
		}
	}
	rockx_destroy(carplate_det_handle);
	return NULL;
}

static void *PlateOCRStream(void *data) {
	struct Session * session = (Session *)data;
	int ret;
	std::string cmd_count;
	std::string cmd_num;
	std::string cmd_color;
	std::string cmd_type;
	std::string plate_type[] = {"unknow", "蓝牌小型车牌", "单行黄牌", "双行黄牌", "小型新能源牌", "大型新能源牌", "警车牌", "单行武警牌", "双行武警牌", "单行军车牌", "双行军车牌", "领馆车牌", "使馆车牌", "双行黄牌挂车车牌", "驾校车牌", "民航车牌", "港澳车牌", "应急车牌", "黑色特殊车牌"};
	std::string plate_color[] = {"unknow", "blue", "yellow", "white", "black", "green"};

	ret = SAMPLE_COMMON_RGA_Start(session, 1);
	if(ret) {
		return NULL;
	}

        MPP_CHN_S stRgaChn1;
	stRgaChn1.enModId = RK_ID_RGA;
	stRgaChn1.s32ChnId = 1;
	RK_MPI_SYS_RegisterOutCb(&stRgaChn1, PlateOCRCb);

	ret = SAMPLE_COMMON_Bind(RK_ID_VI, 1, RK_ID_RGA, 1);
	if(ret) {
		return NULL;
	}

	rockx_handle_t carplate_align_handle;
	rockx_handle_t carplate_recog_handle;
	ret = rockx_create(&carplate_align_handle, ROCKX_MODULE_CARPLATE_ALIGN, &rockx_configs, sizeof(rockx_config_t));
	if (ret != ROCKX_RET_SUCCESS) {
		printf("init rockx module ROCKX_MODULE_CARPLATE_ALIGN error %d\n", ret);
		return NULL;
	}

	ret = rockx_create(&carplate_recog_handle, ROCKX_MODULE_CARPLATE_RECOG, &rockx_configs, sizeof(rockx_config_t));
	if (ret != ROCKX_RET_SUCCESS) {
		printf("init rockx module ROCKX_MODULE_OBJECT_DETECTION error %d\n", ret);
		return NULL;
	}

	while (!quit) {
		if(PlateOCRGetFlag == 1) {
			if (RK_MPI_MB_GetPtr(PlateOCRBuffer) == NULL){
				PlateOCRGetFlag = 0;
				continue;
			}

			rockx_image_t input_image;
			input_image.data = (uint8_t *)RK_MPI_MB_GetPtr(PlateOCRBuffer);
			input_image.size = session->OutVideoWidth * session->OutVideoHeight * 3;
			input_image.width = session->OutVideoWidth;
			input_image.height = session->OutVideoHeight;
			input_image.pixel_format = ROCKX_PIXEL_FORMAT_RGB888;
			cmd_count = "count=";
			cmd_num = "num=";
			cmd_color = "color=";
			cmd_type = "type=";
			cmd_count.append(std::to_string(plate_ocr_count));
			cmd_count.append("&");
			for(int j = 0; j < plate_ocr_count; j++) {
				// create rockx_carplate_align_result_t for store result
				rockx_carplate_align_result_t result;
				memset(&result, 0, sizeof(rockx_carplate_align_result_t));

				// carplate_fmapping_
				ret = rockx_carplate_align(carplate_align_handle, &input_image, &carplate_ocr_array.object[j].box, &result);
				if (ret != ROCKX_RET_SUCCESS) {
					printf("rockx_carplate_align error %d\n", ret);
					return NULL;
				}

				// recognize carplate number
				rockx_carplate_recog_result_t recog_result;
				ret = rockx_carplate_recognize(carplate_recog_handle, &(result.aligned_image), &recog_result);

				// remember release aligned image
				rockx_image_release(&(result.aligned_image));

				if (ret != ROCKX_RET_SUCCESS) {
					printf("rockx_carplate_recognize error %d\n", ret);
					return NULL;
				}

				// process result
				char platename[32];
				memset(platename, 0, 32);
				for(int n = 0; n < recog_result.length; n++) {
					strcat(platename, ROCKX_CARPLATE_RECOG_CODE[recog_result.namecode[n]]);
				}
				//printf("carplate: %s\n", platename);

				if(strlen(platename) != 0) {
					cmd_num.append(platename);
				} else {
					cmd_num.append("unknow");
				}

				cmd_type.append("unknow");
				cmd_color.append("unknow");
				if( j != plate_ocr_count - 1) {
					cmd_num.append(",");
					cmd_color.append(",");
					cmd_type.append(",");
				} else {
					cmd_num.append("&");
					cmd_color.append("&");
				}
			}
		
			if(plate_ocr_count == 0){
				cmd_num.append("null&");		
				cmd_color.append("null&");
				cmd_type.append("null");
			}

			cmd_count.append(cmd_num);
			cmd_count.append(cmd_color);
			cmd_count.append(cmd_type);
			//std::cout<<cmd_count<<std::endl; 打印发送的字符串信息
			if(posixsm_share->http_data_update == 0) {
				strcpy(posixsm_share->http_data, cmd_count.c_str());
				posixsm_share->http_data_update = 1;
			}
			//httpSend(cmd_count.c_str());
			//delete img;

			RK_MPI_MB_ReleaseBuffer(PlateOCRBuffer);
			plate_ocr_count = 0;
			plate_ocr_flag = 1;
			PlateOCRGetFlag = 0;
		}
	}
	rockx_destroy(carplate_align_handle);
	rockx_destroy(carplate_recog_handle);
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

	posixsm_share->http_data_update = 0;
	posixsm_share->plate_data_update = 0;

	pthread_t plate_det_thread;
	pthread_create(&plate_det_thread, NULL, PlateDetectStream, session_cfg);//车牌检测

	pthread_t plate_ocr_thread;
	pthread_create(&plate_ocr_thread, NULL, PlateOCRStream, session_cfg);//车牌识别

	while (!quit) {
		usleep(100000);
	}
	munmap(posixsm_share, sizeof(struct share_data)); //取消内存映射
	SAMPLE_COMMON_UnBind(RK_ID_VI, 1, RK_ID_RGA, 0);
	SAMPLE_COMMON_UnBind(RK_ID_VI, 1, RK_ID_RGA, 1);
	RK_MPI_RGA_DestroyChn(0);
	RK_MPI_RGA_DestroyChn(1);
	RK_MPI_VI_DisableChn(s32CamId, 1);
	pthread_join(plate_det_thread, NULL);
	pthread_join(plate_ocr_thread, NULL);
	return 0;
}  
