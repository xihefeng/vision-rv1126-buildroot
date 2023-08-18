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
#include "xhlpr_api.h"
#include "xhlpr_type.h"

#ifdef RKAIQ
#include "sample_common.h"
#endif
#include "rkmedia_api.h"
#include "sample_common_firefly_rkmedia.h"

#include <sys/stat.h>
#include <sys/mman.h>

PlateInfo *plate_ocr = NULL;

struct share_data *posixsm_share;
int count;
volatile int plate_ocr_count = 0;
volatile char plate_ocr_flag = 1; //车牌号识别数据拷贝标志位

MEDIA_BUFFER PlateDetectBuffer;
MEDIA_BUFFER PlateOCRBuffer;

volatile int PlateDetectGetFlag = 0;
volatile int PlateOCRGetFlag = 0;

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
	struct Session * det_session = (Session *)malloc(sizeof(Session));
	int ret;

	PlateInfo *plate_det = NULL;
	plate_ocr = (PlateInfo *)malloc(sizeof(PlateInfo)*9);

	memcpy(det_session, session, sizeof(Session));
	det_session->OutVideoWidth = 320;
	det_session->OutVideoHeight = 320;

	ret = SAMPLE_COMMON_RGA_Start(det_session, 0);
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
	//新建车牌检测会话
	XHLPR_SESS sess_pd;
	ret = PDCreate(&sess_pd);
	if (ret != LPR_OK) {
		std::cout << "PDCreate ERROR:" << ret << std::endl;
		return NULL;
	}
	while (!quit) {
		if(PlateDetectGetFlag == 1) {
			if (RK_MPI_MB_GetPtr(PlateDetectBuffer) == NULL){
				PlateDetectGetFlag = 0;
				continue; 
			}

			plate_det = NULL;
			//调用车牌检测接口
			ret = PlateDetect(sess_pd, (unsigned char *)RK_MPI_MB_GetPtr(PlateDetectBuffer), 320, 320, &plate_det, &count);
			if (ret != LPR_OK)
			{
				std::cout << "PlateDetect Error: " << ret << std::endl;
				return NULL;
			}
			//打印车牌检测运行时间与车牌数量
			//std::cout<<"Number of license plate_det:"<<count<<std::endl<<std::endl;
			if (plate_ocr_flag == 1) {
				plate_ocr_flag = 0;
				if(plate_det == NULL) {
					RK_MPI_MB_ReleaseBuffer(PlateDetectBuffer);
					plate_ocr_flag = 1;
					PlateDetectGetFlag = 0;
					continue;
				}
				plate_ocr_count = count;
				for(int j = 0; j < count; j++) {
					memcpy(plate_ocr + j, plate_det + j, (sizeof(PlateInfo)));
					for(int p = 0; p < 4; p++) {
						plate_ocr[j].points[p].x = (float)((float)plate_det[j].points[p].x * (float)((float)det_session->InVideoWidth / det_session->OutVideoWidth));
						plate_ocr[j].points[p].y = (float)((float)plate_det[j].points[p].y * (float)((float)det_session->InVideoHeight / det_session->OutVideoHeight));
					}
				}
			}
			if (posixsm_share->plate_data_update == 0) {
				posixsm_share->count = count;
				for(int j = 0; j < count; j++) {
					for(int p = 0; p < 4; p++) {
						posixsm_share->tplate[j].points[p].x = (float)((float)plate_det[j].points[p].x * (float)((float)det_session->EnVideoWidth / det_session->OutVideoWidth));
						posixsm_share->tplate[j].points[p].y = (float)((float)plate_det[j].points[p].y * (float)((float)det_session->EnVideoHeight / det_session->OutVideoHeight));
					}
					posixsm_share->tplate[j].points_score = plate_det[j].points_score;
#if 0
				std::cout << "----------------------------------------------------" << std::endl;
				std::cout<<"License Plate local:("
					<<posixsm_share->tplate[j].points[0].x<<","<<posixsm_share->tplate[j].points[0].y<<")("
					<<posixsm_share->tplate[j].points[1].x<<","<<posixsm_share->tplate[j].points[1].y<<")("
					<<posixsm_share->tplate[j].points[2].x<<","<<posixsm_share->tplate[j].points[2].y<<")("
					<<posixsm_share->tplate[j].points[3].x<<","<<posixsm_share->tplate[j].points[3].y<<")"<<std::endl;
#endif
				}
				posixsm_share->plate_data_update = 1;
			}
			RK_MPI_MB_ReleaseBuffer(PlateDetectBuffer);
			PlateDetectGetFlag = 0;
		} else {
			usleep(1000);
		}
	}

	//调用车牌检测会话销毁接口
	ret = PDDestroy(&sess_pd);
	if (ret != LPR_OK) {
		std::cout << "PDDestroy ERROR:" << ret << std::endl;
		return NULL;
	}

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

	//新建车牌识别会话
	XHLPR_SESS sess_pr;
	ret = PRCreate(&sess_pr);
	if (ret != LPR_OK) {
		std::cout << "PRCreate ERROR:" << ret << std::endl;
		return NULL;
	}

	while (!quit) {
		if(PlateOCRGetFlag == 1) {
			if (RK_MPI_MB_GetPtr(PlateOCRBuffer) == NULL){
				PlateOCRGetFlag = 0;
				continue;
			}

			cmd_count = "count=";
			cmd_num = "num=";
			cmd_color = "color=";
			cmd_type = "type=";
			cmd_count.append(std::to_string(plate_ocr_count));
			cmd_count.append("&");
			for(int j = 0; j < plate_ocr_count; j++) {
				//运行车牌识别接口
				ret = PlateOCR(sess_pr, (unsigned char *)RK_MPI_MB_GetPtr(PlateOCRBuffer), session->OutVideoWidth, session->OutVideoHeight, plate_ocr + j);
				if (ret != LPR_OK)
				{
					std::cout << "PlateOCR Error: " << ret << std::endl;
					return NULL;
				}
#if 0
				if(plate_ocr[j].number_score > 0){
					std::cout<<"PlateOcr Number:"<<plate_ocr[j].plateNumber<<"  score:"<<plate_ocr[j].number_score<<std::endl;
					std::cout<<"type:"<<plate_ocr[j].type<<"  color:"<<plate_ocr[j].color<<std::endl;
				} else {
					std::cout<<"License plateOcr recognition failure!!!"<<std::endl;
				}
				std::cout << "----------------------------------------------------\n" << std::endl;
#endif
				if((strlen(plate_ocr[j].plateNumber)) != 0) {
					cmd_num.append(plate_ocr[j].plateNumber);
					//printf("plate_ocr[%d].plateNumber = %s\n", j, plate_ocr[j].plateNumber);
				} else {
					cmd_num.append("unknow");
				}
				if(plate_ocr[j].type != 0) {
					if(plate_ocr[j].type >= 6000 && plate_ocr[j].type <= 6018) {
						cmd_type.append(plate_type[plate_ocr[j].type % 100]);
					} else {
						cmd_type.append("unknow");
					}
				} else {
					cmd_type.append("unknow");
				}
				if(plate_ocr[j].number_score > 0.8){
					//printf("plate_ocr[j].color = %d\n", plate_ocr[j].color);
					if(plate_ocr[j].color >= 5000 && plate_ocr[j].color <= 5005) {
						cmd_color.append(plate_color[plate_ocr[j].color % 100]);
					} else {
						cmd_color.append("unknow");
					}
				} else {
					cmd_color.append("unknow");
				}
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
			//std::cout<<cmd_count<<std::endl;
			if(posixsm_share->http_data_update == 0 && plate_ocr_count > 0 ) {
				strcpy(posixsm_share->http_data, cmd_count.c_str());
				posixsm_share->http_data_update = 1;
			}

			RK_MPI_MB_ReleaseBuffer(PlateOCRBuffer);
			plate_ocr_count = 0;
			plate_ocr_flag = 1;
			PlateOCRGetFlag = 0;
		} else {
			usleep(1000);
		}
	}

	//调用车牌识别会话销毁接口
	ret = PRDestroy(&sess_pr);
	if (ret != LPR_OK) {
		std::cout << "PRDestroy ERROR:" << ret << std::endl;
		return NULL;
	}
	return NULL;
}

int XH_Init(char *conf, char *licSever)
{
	int ret;
	//读取激活码，并打印激活码
	std::ifstream activation_file;
	//activation_file.open("../licSever/activation.conf");
	activation_file.open(conf);
	if (!activation_file)
	{
		std::cout << "activation.conf not found" << std::endl;
		return -1;
	}
	std::string activation_code;
	getline(activation_file, activation_code);
	activation_file.close();
	std::cout << "activation_code: " << (char*)activation_code.data() << std::endl;

	//程序开始调用初始化接口初始化激活
	//ret = XHLPRInit("../licSever", activation_code.c_str());
	ret = XHLPRInit(licSever, activation_code.c_str());
	if (ret != LPR_OK) {
		std::cout << "XHLPRInit ERROR:" << ret << std::endl;
		return ret;
	}
	return 0;
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

	ret = XH_Init(session_cfg->activation_conf_path, session_cfg->licSever_path);
	if(ret) {
		return -1;
	}

	RK_MPI_SYS_Init();

	ret = SAMPLE_COMMON_VI_Start(session_cfg, session_cfg->video_node, 1);
	if(ret) {
		return -1;
	}

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

	//程序结束，调用逆初始化接口
	XHLPRFinal();
	return 0;
}  
