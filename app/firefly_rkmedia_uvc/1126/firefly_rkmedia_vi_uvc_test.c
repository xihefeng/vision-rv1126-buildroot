// Copyright 2020 Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>

#include <rga/im2d.h>
#include <rga/rga.h>

#include "rkmedia_api.h"
#include "rkmedia_api.h"
#include "rkmedia_venc.h"
#include "rknn/rknn_api.h"

#include <assert.h>
#include <fcntl.h>
#include <time.h>

#include <linux/usb/video.h>
#include "uvc/uvc_control.h"
#include "uvc/uvc_video.h"
#include "common/sample_common.h"
#include "common/tcp_comm.h"
#include "rkmedia_api.h"


#ifdef RKAIQ
#include "common/sample_common.h"
#endif

#define MAX_SESSION_NUM 2
#define DRAW_INDEX 0
#define RK_NN_INDEX 1
#define MAX_RKNN_LIST_NUM 10
#define UPALIGNTO(value, align) ((value + align - 1) & (~(align - 1)))
#define UPALIGNTO16(value) UPALIGNTO(value, 16)

static pthread_t th;
static pthread_t read_thread;
static pthread_t main_stream_thread;
static pthread_t tcp_thread;
static int start;
static RK_CHAR *pDeviceName = "rkispp_scale0";
RK_CHAR *pIqfilesPath = NULL;
RK_S32 s32CamId = 0;
RK_BOOL bMultictx = RK_FALSE;

static bool quit = false;
static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  quit = true;
  start = 0;
}
struct rv1126_data {
	rknn_input_output_num io_num;
	rknn_context ctx;
	int camera_input_width;
	int camera_input_height;
	int number;
};
struct rv1126_data nndata;
unsigned char *model;

struct label_and_location {
    char c_label[30][30];
    float x1[30];
    float y1[30];
    float x2[30];
    float y2[30];
    float prop[30];
    int topclass[30];
    int val;
    int camera_input_width;
    int camera_input_height;
};
struct label_and_location tcp_send_data;

struct Session {
  char path[64];
  CODEC_TYPE_E video_type;
  RK_U32 u32Width;
  RK_U32 u32Height;
  IMAGE_TYPE_E enImageType;
  char videopath[120];

  MPP_CHN_S stViChn;
  MPP_CHN_S stRgaChn;
};

struct demo_cfg {
  int session_count;
  struct Session session_cfg[MAX_SESSION_NUM];
};

struct demo_cfg cfg;
#ifdef RKAIQ
void camera_control(unsigned char cs, unsigned int val) {
  switch (cs) {
  case UVC_PU_BRIGHTNESS_CONTROL:
    SAMPLE_COMM_ISP_SET_Brightness(s32CamId, val);
    break;
  case UVC_PU_CONTRAST_CONTROL:
    SAMPLE_COMM_ISP_SET_Contrast(s32CamId, val);
    break;
  case UVC_PU_HUE_CONTROL:
    break;
  case UVC_PU_SATURATION_CONTROL:
    SAMPLE_COMM_ISP_SET_Saturation(s32CamId, val);
    break;
  case UVC_PU_SHARPNESS_CONTROL:
    SAMPLE_COMM_ISP_SET_Sharpness(s32CamId, val);
    break;
  case UVC_PU_GAMMA_CONTROL:
    break;
  case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
    break;
  case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
    break;
  case UVC_PU_GAIN_CONTROL:
    break;
  case UVC_PU_HUE_AUTO_CONTROL:
    break;
  case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
    break;
  case UVC_PU_DIGITAL_MULTIPLIER_CONTROL:
    break;
  default:
    break;
  }
}
#endif

// SSD
static char *g_box_priors = "/oem/usr/share/rknn_model/box_priors.txt";
static char *g_labels_list = "/oem/usr/share/rknn_model/coco_labels_list.txt";
static char *g_ssd_path =
    "/oem/usr/share/rknn_model/ssd_inception_v2_rv1109_rv1126.rknn";

#define OBJ_NAME_MAX_SIZE 16
#define OBJ_NUMB_MAX_SIZE 64

typedef struct _BOX_RECT {
  int left;
  int right;
  int top;
  int bottom;
} BOX_RECT;

typedef struct __detect_result_t {
  char name[OBJ_NAME_MAX_SIZE];
  BOX_RECT box;
  float prop;
} detect_result_t;

typedef struct _detect_result_group_t {
  int id;
  int count;
  detect_result_t results[OBJ_NUMB_MAX_SIZE];
} detect_result_group_t;

#define IMG_CHANNEL 3
#define MODEL_INPUT_SIZE 300
#define NUM_RESULTS 1917
#define NUM_CLASS 91

#define Y_SCALE 10.0f
#define X_SCALE 10.0f
#define H_SCALE 5.0f
#define W_SCALE 5.0f

static void *UvcGetMediaBuffer(void *arg) {
  MEDIA_BUFFER mb = NULL;
  (void)arg;
  while (1) {
    mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, cfg.session_cfg[DRAW_INDEX].stRgaChn.s32ChnId, -1);
    if (!mb) {
      printf("RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      break;
    }

    MPP_ENC_INFO_DEF info = {0};
    info.fd = RK_MPI_MB_GetFD(mb);
    info.size = RK_MPI_MB_GetSize(mb);
    uvc_read_camera_buffer(RK_MPI_MB_GetPtr(mb), &info, NULL, 0);

    RK_MPI_MB_ReleaseBuffer(mb);
  }

  return NULL;
}

float MIN_SCORE = 0.4f;
float NMS_THRESHOLD = 0.45f;

static char *labels[NUM_CLASS];
static float box_priors[4][NUM_RESULTS];

char *readLine(FILE *fp, char *buffer, int *len) {
  int ch;
  int i = 0;
  size_t buff_len = 0;

  buffer = (char *)malloc(buff_len + 1);
  if (!buffer)
    return NULL; // Out of memory

  while ((ch = fgetc(fp)) != '\n' && ch != EOF) {
    buff_len++;
    void *tmp = realloc(buffer, buff_len + 1);
    if (tmp == NULL) {
      free(buffer);
      return NULL; // Out of memory
    }
    buffer = (char *)tmp;

    buffer[i] = (char)ch;
    i++;
  }
  buffer[i] = '\0';

  *len = buff_len;

  // Detect end
  if (ch == EOF && (i == 0 || ferror(fp))) {
    free(buffer);
    return NULL;
  }
  return buffer;
}

int readLines(const char *fileName, char *lines[], int max_line) {
  FILE *file = fopen(fileName, "r");
  if (file == NULL)
    printf("open %s fail\n", fileName);
  char *s;
  int i = 0;
  int n = 0;
  while ((s = readLine(file, s, &n)) != NULL) {
    lines[i++] = s;
    if (i >= max_line)
      break;
  }
  return i;
}

int loadLabelName(const char *locationFilename, char *label[]) {
  printf("ssd - loadLabelName %s\n", locationFilename);
  readLines(locationFilename, label, NUM_CLASS);
  return 0;
}

int loadBoxPriors(const char *locationFilename,
                  float (*boxPriors)[NUM_RESULTS]) {
  const char *d = " ";
  char *lines[4];
  // int count = readLines(locationFilename, lines, 4);
  readLines(locationFilename, lines, 4);
  // printf("line count %d\n", count);
  // for (int i = 0; i < count; i++) {
  // printf("%s\n", lines[i]);
  // }
  for (int i = 0; i < 4; i++) {
    char *line_str = lines[i];
    char *p;
    p = strtok(line_str, d);
    int priorIndex = 0;
    while (p) {
      float number = (float)(atof(p));
      boxPriors[i][priorIndex++] = number;
      p = strtok(NULL, d);
    }
    if (priorIndex != NUM_RESULTS) {
      printf("error\n");
      return -1;
    }
  }
  return 0;
}

float CalculateOverlap(float xmin0, float ymin0, float xmax0, float ymax0,
                       float xmin1, float ymin1, float xmax1, float ymax1) {
  float w = fmax(0.f, fmin(xmax0, xmax1) - fmax(xmin0, xmin1));
  float h = fmax(0.f, fmin(ymax0, ymax1) - fmax(ymin0, ymin1));
  float i = w * h;
  float u =
      (xmax0 - xmin0) * (ymax0 - ymin0) + (xmax1 - xmin1) * (ymax1 - ymin1) - i;
  return u <= 0.f ? 0.f : (i / u);
}

float unexpit(float y) { return -1.0 * logf((1.0 / y) - 1.0); }

float expit(float x) { return (float)(1.0 / (1.0 + expf(-x))); }

void decodeCenterSizeBoxes(float *predictions,
                           float (*boxPriors)[NUM_RESULTS]) {

  for (int i = 0; i < NUM_RESULTS; ++i) {
    float ycenter =
        predictions[i * 4 + 0] / Y_SCALE * boxPriors[2][i] + boxPriors[0][i];
    float xcenter =
        predictions[i * 4 + 1] / X_SCALE * boxPriors[3][i] + boxPriors[1][i];
    float h = (float)expf(predictions[i * 4 + 2] / H_SCALE) * boxPriors[2][i];
    float w = (float)expf(predictions[i * 4 + 3] / W_SCALE) * boxPriors[3][i];

    float ymin = ycenter - h / 2.0f;
    float xmin = xcenter - w / 2.0f;
    float ymax = ycenter + h / 2.0f;
    float xmax = xcenter + w / 2.0f;

    predictions[i * 4 + 0] = ymin;
    predictions[i * 4 + 1] = xmin;
    predictions[i * 4 + 2] = ymax;
    predictions[i * 4 + 3] = xmax;
  }
}

int filterValidResult(float *outputClasses, int (*output)[NUM_RESULTS],
                      int numClasses, float *props) {
  int validCount = 0;
  float min_score = unexpit(MIN_SCORE);

  // Scale them back to the input size.
  for (int i = 0; i < NUM_RESULTS; ++i) {
    float topClassScore = (float)(-1000.0);
    int topClassScoreIndex = -1;

    // Skip the first catch-all class.
    for (int j = 1; j < numClasses; ++j) {
      // x and expit(x) has same monotonicity
      // so compare x and comare expit(x) is same
      // float score = expit(outputClasses[i*numClasses+j]);
      float score = outputClasses[i * numClasses + j];

      if (score > topClassScore) {
        topClassScoreIndex = j;
        topClassScore = score;
      }
    }

    if (topClassScore >= min_score) {
      output[0][validCount] = i;
      output[1][validCount] = topClassScoreIndex;
      props[validCount] =
          expit(outputClasses[i * numClasses + topClassScoreIndex]);
      ++validCount;
    }
  }

  return validCount;
}

int nms(int validCount, float *outputLocations, int (*output)[NUM_RESULTS]) {
  for (int i = 0; i < validCount; ++i) {
    if (output[0][i] == -1) {
      continue;
    }
    int n = output[0][i];
    for (int j = i + 1; j < validCount; ++j) {
      int m = output[0][j];
      if (m == -1) {
        continue;
      }
      float xmin0 = outputLocations[n * 4 + 1];
      float ymin0 = outputLocations[n * 4 + 0];
      float xmax0 = outputLocations[n * 4 + 3];
      float ymax0 = outputLocations[n * 4 + 2];

      float xmin1 = outputLocations[m * 4 + 1];
      float ymin1 = outputLocations[m * 4 + 0];
      float xmax1 = outputLocations[m * 4 + 3];
      float ymax1 = outputLocations[m * 4 + 2];

      float iou = CalculateOverlap(xmin0, ymin0, xmax0, ymax0, xmin1, ymin1,
                                   xmax1, ymax1);

      if (iou >= NMS_THRESHOLD) {
        output[0][j] = -1;
      }
    }
  }
  return 0;
}

void sort(int output[][1917], float *props, int sz) {
  int i = 0;
  int j = 0;

  if (sz < 2) {
    return;
  }

  for (i = 0; i < sz - 1; i++) {

    int top = i;
    for (j = i + 1; j < sz; j++) {
      if (props[top] < props[j]) {
        top = j;
      }
    }

    if (i != top) {
      int tmp1 = output[0][i];
      int tmp2 = output[1][i];
      float prop = props[i];
      output[0][i] = output[0][top];
      output[1][i] = output[1][top];
      props[i] = props[top];
      output[0][top] = tmp1;
      output[1][top] = tmp2;
      props[top] = prop;
    }
  }
}

int postProcessSSD(float *predictions, float *output_classes, int width,
                   int heigh, detect_result_group_t *group) {
  static int init = -1;
  char send_buf[1024 * 2];  
  int ret = 0;
  if (init == -1) {
    int ret = 0;
    printf("loadLabelName\n");
    ret = loadLabelName(g_labels_list, labels);
    if (ret < 0) {
      return -1;
    }
    // for (int i = 0; i < 91; i++) {
    // printf("%s\n", labels[i]);
    // }

    printf("loadBoxPriors\n");
    ret = loadBoxPriors(g_box_priors, box_priors);
    if (ret < 0) {
      return -1;
    }
    // for (int i = 0; i < 4; i++) {
    // for (int j = 0; j < 1917; j++) {
    // printf("%f ", box_priors[i][j]);
    // }
    // printf("\n");
    // }
    init = 0;
  }

  int output[2][NUM_RESULTS];
  float props[NUM_RESULTS];
  memset(output, 0, 2 * NUM_RESULTS);
  memset(props, 0, sizeof(float) * NUM_RESULTS);

  decodeCenterSizeBoxes(predictions, box_priors);

  int validCount = filterValidResult(output_classes, output, NUM_CLASS, props);

  if (validCount > OBJ_NUMB_MAX_SIZE) {
    printf("validCount too much !!\n");
    tcp_send_data.val = 0;
    memset(send_buf, 0x00, sizeof(send_buf)); 
    memcpy(send_buf, &tcp_send_data, sizeof(struct label_and_location));  
    ret = send_data(send_buf, sizeof(struct label_and_location));//发送send_buf 字节label_and_location大小
    if (ret != sizeof(struct label_and_location)) {
            printf("send len error: %d; expect len: %d\n", ret, sizeof(struct label_and_location));
            return -1;
    }
    memset(&tcp_send_data, 0x00, sizeof(struct label_and_location));//置零                
    return -1;
  }

  sort(output, props, validCount);

  /* detect nest box */
  nms(validCount, predictions, output);

  int last_count = 0;
  group->count = 0;
  /* box valid detect target */
  for (int i = 0; i < validCount; ++i) {
    if (output[0][i] == -1) {
      continue;
    }
    int n = output[0][i];
    int topClassScoreIndex = output[1][i];

    int x1 = (int)(predictions[n * 4 + 1] * width);
    int y1 = (int)(predictions[n * 4 + 0] * heigh);
    int x2 = (int)(predictions[n * 4 + 3] * width);
    int y2 = (int)(predictions[n * 4 + 2] * heigh);


    // There are a bug show toothbrush always
    if (x1 == 0 && x2 == 0 && y1 == 0 && y2 == 0)
      continue;
    char *label = labels[topClassScoreIndex];
    tcp_send_data.x1[i] =
        (predictions[n * 4 + 1]);
    tcp_send_data.y1[i] =
        (predictions[n * 4 + 0]);
    tcp_send_data.x2[i] =
        (predictions[n * 4 + 3]);
    tcp_send_data.y2[i] =
        (predictions[n * 4 + 2]);
    tcp_send_data.prop[i] = props[i];
    strcpy(tcp_send_data.c_label[i], label);
    tcp_send_data.topclass[i] = topClassScoreIndex;

    group->results[last_count].box.left = x1;
    group->results[last_count].box.top = y1;
    group->results[last_count].box.right = x2;
    group->results[last_count].box.bottom = y2;
    group->results[last_count].prop = props[i];
    memcpy(group->results[last_count].name, label, OBJ_NAME_MAX_SIZE);

    // printf("ssd result %2d: (%4d, %4d, %4d, %4d), %4.2f, %s\n", i, x1, y1,
    // x2, y2, props[i], label);
    last_count++;
  }
  //打包发送数据
  tcp_send_data.val = validCount;
  tcp_send_data.camera_input_width = nndata.camera_input_width;
  tcp_send_data.camera_input_height = nndata.camera_input_height;
  memset(send_buf, 0x00, sizeof(send_buf)); 
  memcpy(send_buf, &tcp_send_data, sizeof(struct label_and_location));  

  ret = send_data(send_buf, sizeof(struct label_and_location));//发送send_buf 字节label_and_location大小
  if (ret != sizeof(struct label_and_location)) {
      printf("send len error: %d; expect len: %d\n", ret, sizeof(struct label_and_location));
      return -1;
  }
  memset(&tcp_send_data, 0x00, sizeof(struct label_and_location));//置零 
  group->count = last_count;
  return 0;
}

// rknn list to draw boxs asynchronously
typedef struct node {
  long timeval;
  detect_result_group_t detect_result_group;
  struct node *next;
} Node;

typedef struct my_stack {
  int size;
  Node *top;
} rknn_list;

void create_rknn_list(rknn_list **s) {
  if (*s != NULL)
    return;
  *s = (rknn_list *)malloc(sizeof(rknn_list));
  (*s)->top = NULL;
  (*s)->size = 0;
  printf("create rknn_list success\n");
}

void destory_rknn_list(rknn_list **s) {
  Node *t = NULL;
  if (*s == NULL)
    return;
  while ((*s)->top) {
    t = (*s)->top;
    (*s)->top = t->next;
    free(t);
  }
  free(*s);
  *s = NULL;
}

void rknn_list_push(rknn_list *s, long timeval,
                    detect_result_group_t detect_result_group) {
  Node *t = NULL;
  t = (Node *)malloc(sizeof(Node));
  t->timeval = timeval;
  t->detect_result_group = detect_result_group;
  if (s->top == NULL) {
    s->top = t;
    t->next = NULL;
  } else {
    t->next = s->top;
    s->top = t;
  }
  s->size++;
}

void rknn_list_pop(rknn_list *s, long *timeval,
                   detect_result_group_t *detect_result_group) {
  Node *t = NULL;
  if (s == NULL || s->top == NULL)
    return;
  t = s->top;
  *timeval = t->timeval;
  *detect_result_group = t->detect_result_group;
  s->top = t->next;
  free(t);
  s->size--;
}

void rknn_list_drop(rknn_list *s) {
  Node *t = NULL;
  if (s == NULL || s->top == NULL)
    return;
  t = s->top;
  s->top = t->next;
  free(t);
  s->size--;
}

int rknn_list_size(rknn_list *s) {
  if (s == NULL)
    return -1;
  return s->size;
}

rknn_list *rknn_list_;



static long getCurrentTimeMsec() {
  long msec = 0;
  char str[20] = {0};
  struct timeval stuCurrentTime;

  gettimeofday(&stuCurrentTime, NULL);
  sprintf(str, "%ld%03ld", stuCurrentTime.tv_sec,
          (stuCurrentTime.tv_usec) / 1000);
  for (size_t i = 0; i < strlen(str); i++) {
    msec = msec * 10 + (str[i] - '0');
  }

  return msec;
}

static long int crv_tab[256];
static long int cbu_tab[256];
static long int cgu_tab[256];
static long int cgv_tab[256];
static long int tab_76309[256];
static unsigned char clp[1024]; // for clip in CCIR601

void init_yuv420p_table() {
  long int crv, cbu, cgu, cgv;
  int i, ind;
  static int init = 0;

  if (init == 1)
    return;

  crv = 104597;
  cbu = 132201; /* fra matrise i global.h */
  cgu = 25675;
  cgv = 53279;

  for (i = 0; i < 256; i++) {
    crv_tab[i] = (i - 128) * crv;
    cbu_tab[i] = (i - 128) * cbu;
    cgu_tab[i] = (i - 128) * cgu;
    cgv_tab[i] = (i - 128) * cgv;
    tab_76309[i] = 76309 * (i - 16);
  }

  for (i = 0; i < 384; i++)
    clp[i] = 0;
  ind = 384;
  for (i = 0; i < 256; i++)
    clp[ind++] = i;
  ind = 640;
  for (i = 0; i < 384; i++)
    clp[ind++] = 255;

  init = 1;
}

int rgb24_resize(unsigned char *input_rgb, unsigned char *output_rgb, int width,
                 int height, int outwidth, int outheight) {
  rga_buffer_t src =
      wrapbuffer_virtualaddr(input_rgb, width, height, RK_FORMAT_RGB_888);
  rga_buffer_t dst = wrapbuffer_virtualaddr(output_rgb, outwidth, outheight,
                                            RK_FORMAT_RGB_888);
  rga_buffer_t pat = {0};
  im_rect src_rect = {0, 0, width, height};
  im_rect dst_rect = {0, 0, outwidth, outheight};
  im_rect pat_rect = {0};
  IM_STATUS STATUS = improcess(src, dst, pat, src_rect, dst_rect, pat_rect, 0);
  if (STATUS != IM_STATUS_SUCCESS) {
    printf("imcrop failed: %s\n", imStrError(STATUS));
    start = 0;
    return -1;
  }
  return 0;
}

void nv12_to_rgb24(unsigned char *yuvbuffer, unsigned char *rga_buffer,
                   int width, int height) {
  int y1, y2, u, v;
  unsigned char *py1, *py2;
  int i, j, c1, c2, c3, c4;
  unsigned char *d1, *d2;
  unsigned char *src_u;

  src_u = yuvbuffer + width * height; // u

  py1 = yuvbuffer; // y
  py2 = py1 + width;
  d1 = rga_buffer;
  d2 = d1 + 3 * width;

  init_yuv420p_table();

  for (j = 0; j < height; j += 2) {
    for (i = 0; i < width; i += 2) {
      u = *src_u++;
      v = *src_u++; // v immediately follows u, in the next position of u

      c4 = crv_tab[v];
      c2 = cgu_tab[u];
      c3 = cgv_tab[v];
      c1 = cbu_tab[u];

      // up-left
      y1 = tab_76309[*py1++];
      *d1++ = clp[384 + ((y1 + c1) >> 16)];
      *d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
      *d1++ = clp[384 + ((y1 + c4) >> 16)];

      // down-left
      y2 = tab_76309[*py2++];
      *d2++ = clp[384 + ((y2 + c1) >> 16)];
      *d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
      *d2++ = clp[384 + ((y2 + c4) >> 16)];

      // up-right
      y1 = tab_76309[*py1++];
      *d1++ = clp[384 + ((y1 + c1) >> 16)];
      *d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
      *d1++ = clp[384 + ((y1 + c4) >> 16)];

      // down-right
      y2 = tab_76309[*py2++];
      *d2++ = clp[384 + ((y2 + c1) >> 16)];
      *d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
      *d2++ = clp[384 + ((y2 + c4) >> 16)];
    }
    d1 += 3 * width;
    d2 += 3 * width;
    py1 += width;
    py2 += width;
  }

  // save bmp
  // int filesize = 54 + 3 * width * height;
  // FILE *f;
  // unsigned char bmpfileheader[14] = {'B', 'M', 0, 0,  0, 0, 0,
  //                                    0,   0,   0, 54, 0, 0, 0};
  // unsigned char bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0,  0,
  //                                    0,  0, 0, 0, 1, 0, 24, 0};
  // unsigned char bmppad[3] = {0, 0, 0};

  // bmpfileheader[2] = (unsigned char)(filesize);
  // bmpfileheader[3] = (unsigned char)(filesize >> 8);
  // bmpfileheader[4] = (unsigned char)(filesize >> 16);
  // bmpfileheader[5] = (unsigned char)(filesize >> 24);

  // bmpinfoheader[4] = (unsigned char)(width);
  // bmpinfoheader[5] = (unsigned char)(width >> 8);
  // bmpinfoheader[6] = (unsigned char)(width >> 16);
  // bmpinfoheader[7] = (unsigned char)(width >> 24);
  // bmpinfoheader[8] = (unsigned char)(height);
  // bmpinfoheader[9] = (unsigned char)(height >> 8);
  // bmpinfoheader[10] = (unsigned char)(height >> 16);
  // bmpinfoheader[11] = (unsigned char)(height >> 24);

  // f = fopen("/tmp/tmp.bmp", "wb");
  // fwrite(bmpfileheader, 1, 14, f);
  // fwrite(bmpinfoheader, 1, 40, f);
  // for (int k = 0; k < height; k++) {
  //   fwrite(rga_buffer + (width * (height - k - 1) * 3), 3, width, f);
  //   fwrite(bmppad, 1, (4 - (width * 3) % 4) % 4, f);
  // }
  // fclose(f);
}

int nv12_border(char *pic, int pic_w, int pic_h, int rect_x, int rect_y,
                int rect_w, int rect_h, int R, int G, int B) {
  /* Set up the rectangle border size */
  const int border = 5;

  /* RGB convert YUV */
  int Y, U, V;
  Y = 0.299 * R + 0.587 * G + 0.114 * B;
  U = -0.1687 * R + 0.3313 * G + 0.5 * B + 128;
  V = 0.5 * R - 0.4187 * G - 0.0813 * B + 128;
  /* Locking the scope of rectangle border range */
  int j, k;
  for (j = rect_y; j < rect_y + rect_h; j++) {
    for (k = rect_x; k < rect_x + rect_w; k++) {
      if (k < (rect_x + border) || k > (rect_x + rect_w - border) ||
          j < (rect_y + border) || j > (rect_y + rect_h - border)) {
        /* Components of YUV's storage address index */
        int y_index = j * pic_w + k;
        int u_index =
            (y_index / 2 - pic_w / 2 * ((j + 1) / 2)) * 2 + pic_w * pic_h;
        int v_index = u_index + 1;
        /* set up YUV's conponents value of rectangle border */
        pic[y_index] = Y;
        pic[u_index] = U;
        pic[v_index] = V;
      }
    }
  }

  return 0;
}

static void printRKNNTensor(rknn_tensor_attr *attr) {
  printf("index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d "
         "fmt=%d type=%d qnt_type=%d fl=%d zp=%d scale=%f\n",
         attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2],
         attr->dims[1], attr->dims[0], attr->n_elems, attr->size, 0, attr->type,
         attr->qnt_type, attr->fl, attr->zp, attr->scale);
}

static unsigned char *load_model(const char *filename, int *model_size) {
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    printf("fopen %s fail!\n", filename);
    return NULL;
  }
  fseek(fp, 0, SEEK_END);
  unsigned int model_len = ftell(fp);
  unsigned char *model = (unsigned char *)malloc(model_len);
  fseek(fp, 0, SEEK_SET);
  if (model_len != fread(model, 1, model_len, fp)) {
    printf("fread %s fail!\n", filename);
    free(model);
    return NULL;
  }
  *model_size = model_len;
  if (fp) {
    fclose(fp);
  }
  return model;
}

static void *GetMediaBuffer(void *arg) {
  printf("#Start %s thread, arg:%p\n", __func__, arg);
  int ret;
  MEDIA_BUFFER buffer = NULL;
  while (start) {
    buffer = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, cfg.session_cfg[RK_NN_INDEX].stRgaChn.s32ChnId, -1);
    if (!buffer) {
      //printf("RK_ID_RGA NN RK_MPI_SYS_GetMediaBuffer is NULL\n");
      continue;
    }
    // Set Input Data
    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].size = MODEL_INPUT_SIZE * MODEL_INPUT_SIZE * 3;
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].buf = (unsigned char*)RK_MPI_MB_GetPtr(buffer);

    ret = rknn_inputs_set(nndata.ctx, nndata.io_num.n_input, inputs);
    if (ret < 0) {
      printf("rknn_input_set fail! ret=%d\n", ret);
      return NULL;
    }

    // Run
    printf("rknn_run\n");
    ret = rknn_run(nndata.ctx, NULL);
    if (ret < 0) {
      printf("rknn_run fail! ret=%d\n", ret);
      return NULL;
    }

    // Get Output
    rknn_output outputs[2];
    memset(outputs, 0, sizeof(outputs));
    outputs[0].want_float = 1;
    outputs[1].want_float = 1;
    ret = rknn_outputs_get(nndata.ctx, nndata.io_num.n_output, outputs, NULL);
    if (ret < 0) {
      printf("rknn_outputs_get fail! ret=%d\n", ret);
      return NULL;
    }

    // Post Process
    detect_result_group_t detect_result_group;
    postProcessSSD((float *)(outputs[0].buf), (float *)(outputs[1].buf),
                   MODEL_INPUT_SIZE, MODEL_INPUT_SIZE, &detect_result_group);
    // Release rknn_outputs
    rknn_outputs_release(nndata.ctx, 2, outputs);

    // Dump Objects
     for (int i = 0; i < detect_result_group.count; i++) {
       detect_result_t *det_result = &(detect_result_group.results[i]);
       printf("%s @ (%d %d %d %d) %f\n", det_result->name,
       det_result->box.left,
              det_result->box.top, det_result->box.right,
              det_result->box.bottom,
              det_result->prop);
     }

    if (detect_result_group.count > 0) {
      rknn_list_push(rknn_list_, getCurrentTimeMsec(), detect_result_group);
      int size = rknn_list_size(rknn_list_);
      if (size >= MAX_RKNN_LIST_NUM)
        rknn_list_drop(rknn_list_);
      // printf("size is %d\n", size);
    }
    RK_MPI_MB_ReleaseBuffer(buffer);
    
  }
  //不能卸载模型，不然下次回调函数调用就会没有模型
  /*
  if (nndata.ctx)
    rknn_destroy(nndata.ctx);
  if (model)
    free(model);
    */
  return NULL;
}

static void *MainStream() {
  MEDIA_BUFFER buffer;
  // float x_rate = (float)cfg.session_cfg[DRAW_INDEX].u32Width /
  //                (float)cfg.session_cfg[RK_NN_INDEX].u32Width;
  // float y_rate = (float)cfg.session_cfg[DRAW_INDEX].u32Height /
  //                (float)cfg.session_cfg[RK_NN_INDEX].u32Height;
  float x_rate = (float)cfg.session_cfg[DRAW_INDEX].u32Width / MODEL_INPUT_SIZE;
  float y_rate =
      (float)cfg.session_cfg[DRAW_INDEX].u32Height / MODEL_INPUT_SIZE;
  printf("x_rate is %f, y_rate is %f\n", x_rate, y_rate);

  while (start) {
    buffer = RK_MPI_SYS_GetMediaBuffer(
        RK_ID_RGA, cfg.session_cfg[DRAW_INDEX].stRgaChn.s32ChnId, -1);
    if (!buffer) {
      printf("RK_MPI_SYS_GetMediaBuffer DRAW_INDEX is NULL\n");
      continue;
    }
    // draw
    if (rknn_list_size(rknn_list_)) {
      long time_before;
      detect_result_group_t detect_result_group;
      memset(&detect_result_group, 0, sizeof(detect_result_group));
      rknn_list_pop(rknn_list_, &time_before, &detect_result_group);
      // printf("time interval is %ld\n", getCurrentTimeMsec() - time_before);

      for (int j = 0; j < detect_result_group.count; j++) {

        if (detect_result_group.results[j].prop < 0.5)
          continue;
        int x = detect_result_group.results[j].box.left * x_rate;
        int y = detect_result_group.results[j].box.top * y_rate;
        int w = (detect_result_group.results[j].box.right -
                 detect_result_group.results[j].box.left) *
                x_rate;
        int h = (detect_result_group.results[j].box.bottom -
                 detect_result_group.results[j].box.top) *
                y_rate;
        if (x < 0)
          x = 0;
        if (y < 0)
          y = 0;
        while ((uint32_t)(x + w) >= cfg.session_cfg[DRAW_INDEX].u32Width) {
          w -= 16;
        }
        while ((uint32_t)(y + h) >= cfg.session_cfg[DRAW_INDEX].u32Height) {
          h -= 16;
        }
        printf("border=(%d %d %d %d)\n", x, y, w, h);
        nv12_border((char *)RK_MPI_MB_GetPtr(buffer),
                    cfg.session_cfg[DRAW_INDEX].u32Width,
                    cfg.session_cfg[DRAW_INDEX].u32Height, x, y, w, h, 0, 0,
                    255);
      }
    }
    // send from VI to VENC
    //RK_MPI_SYS_SendMediaBuffer(
    //    RK_ID_VENC, cfg.session_cfg[DRAW_INDEX].stVenChn.s32ChnId, buffer);
    RK_MPI_MB_ReleaseBuffer(buffer);
  }

  return NULL;
}

static int SAMPLE_COMMON_RGA_Start(struct Session *session, int width, int height, int number) {
  int ret = 0;
  RGA_ATTR_S stRgaAttr;

  if (number == RK_NN_INDEX) {
    cfg.session_cfg[RK_NN_INDEX].stRgaChn.enModId = RK_ID_RGA;
    cfg.session_cfg[RK_NN_INDEX].stRgaChn.s32ChnId = number;
    stRgaAttr.bEnBufPool = RK_TRUE;
    stRgaAttr.u16BufPoolCnt = 2;
    stRgaAttr.u16Rotaion = 0;
    stRgaAttr.stImgIn.u32X = 0;
    stRgaAttr.stImgIn.u32Y = 0;
    stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;
    stRgaAttr.stImgIn.u32Width = 720;
    stRgaAttr.stImgIn.u32Height = 576;
    stRgaAttr.stImgIn.u32HorStride = 720;
    stRgaAttr.stImgIn.u32VirStride = 576;
    stRgaAttr.stImgOut.u32X = 0;
    stRgaAttr.stImgOut.u32Y = 0;
    stRgaAttr.stImgOut.imgType = IMAGE_TYPE_RGB888;
    stRgaAttr.stImgOut.u32Width = MODEL_INPUT_SIZE;
    stRgaAttr.stImgOut.u32Height = MODEL_INPUT_SIZE;
    stRgaAttr.stImgOut.u32HorStride = MODEL_INPUT_SIZE;
    stRgaAttr.stImgOut.u32VirStride = MODEL_INPUT_SIZE;
    ret = RK_MPI_RGA_CreateChn(session->stRgaChn.s32ChnId, &stRgaAttr);
    if (ret) {
      printf("ERROR: Create stRgaChn[%d] falied! ret=%d\n", session->stRgaChn.s32ChnId, ret);
    }

  } else if (number == DRAW_INDEX) {
    cfg.session_cfg[DRAW_INDEX].stRgaChn.enModId = RK_ID_RGA;
    cfg.session_cfg[DRAW_INDEX].stRgaChn.s32ChnId = number;
    stRgaAttr.bEnBufPool = RK_TRUE;
    stRgaAttr.u16BufPoolCnt = 2;
    stRgaAttr.u16Rotaion = 0;
    stRgaAttr.stImgIn.u32X = 0;
    stRgaAttr.stImgIn.u32Y = 0;
    stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;
    stRgaAttr.stImgIn.u32Width = width;
    stRgaAttr.stImgIn.u32Height = height;
    stRgaAttr.stImgIn.u32HorStride = width;
    stRgaAttr.stImgIn.u32VirStride = height;
    stRgaAttr.stImgOut.u32X = 0;
    stRgaAttr.stImgOut.u32Y = 0;
    stRgaAttr.stImgOut.imgType = IMAGE_TYPE_NV12;
    stRgaAttr.stImgOut.u32Width = width;
    stRgaAttr.stImgOut.u32Height = height;
    stRgaAttr.stImgOut.u32HorStride = width;
    stRgaAttr.stImgOut.u32VirStride = height;//16对齐
    ret = RK_MPI_RGA_CreateChn(session->stRgaChn.s32ChnId, &stRgaAttr);
    if (ret) {
      printf("ERROR: Create stRgaChn[%d] falied! ret=%d\n", session->stRgaChn.s32ChnId, ret);
    }
  }
}

static void dump_cfg() {
  for (int i = 0; i < cfg.session_count; i++) {
    printf("rtsp path = %s.\n", cfg.session_cfg[i].path);
    printf("video_type = %d.\n", cfg.session_cfg[i].video_type);
    printf("width = %d.\n", cfg.session_cfg[i].u32Width);
    printf("height = %d.\n", cfg.session_cfg[i].u32Height);
    printf("video path =%s.\n", cfg.session_cfg[i].videopath);
    printf("image type = %u.\n", cfg.session_cfg[i].enImageType);
  }
}

static int load_cfg(const char *cfg_file) {
  // cfgline:
  // path=%s video_type=%d width=%u height=%u
  FILE *fp = fopen(cfg_file, "r");
  char line[1024];
  int count = 0;

  if (!fp) {
    fprintf(stderr, "open %s failed\n", cfg_file);
    return -1;
  }

  memset(&cfg, 0, sizeof(cfg));
  while (fgets(line, sizeof(line) - 1, fp)) {
    const char *p;
    // char codec_type[20];
    memset(&cfg.session_cfg[count], 0, sizeof(cfg.session_cfg[count]));

    if (line[0] == '#')
      continue;
    p = strstr(line, "path=");
    if (!p)
      continue;
    if (sscanf(p, "path=%s", cfg.session_cfg[count].path) != 1)
      continue;

    if ((p = strstr(line, "video_type="))) {
      if (sscanf(p,
                 "video_type=%d width=%u height=%u image_type=%u video_path=%s",
                 &cfg.session_cfg[count].video_type,
                 &cfg.session_cfg[count].u32Width,
                 &cfg.session_cfg[count].u32Height,
                 &cfg.session_cfg[count].enImageType,
                 cfg.session_cfg[count].videopath) == 0) {
        printf("parse video file failed %s.\n", p);
      }
    }
    if (cfg.session_cfg[count].video_type != RK_CODEC_TYPE_NONE) {
      count++;
    } else {
      printf("parse line %s failed\n", line);
    }
  }
  cfg.session_count = count;
  fclose(fp);
  dump_cfg();
  return count;
}

static void SAMPLE_COMMON_VI_Start(struct Session *session,
                                   VI_CHN_WORK_MODE mode, RK_S32 vi_pipe,
				   int width, int height, int number) {
  VI_CHN_ATTR_S vi_chn_attr;
  if(number == DRAW_INDEX) {
    cfg.session_cfg[DRAW_INDEX].stViChn.enModId = RK_ID_VI;
    cfg.session_cfg[DRAW_INDEX].stViChn.s32ChnId = DRAW_INDEX;
    vi_chn_attr.u32Width = width;
    vi_chn_attr.u32Height = height;
  }
  else if (number == RK_NN_INDEX) {
    cfg.session_cfg[RK_NN_INDEX].stViChn.enModId = RK_ID_VI;
    cfg.session_cfg[RK_NN_INDEX].stViChn.s32ChnId = RK_NN_INDEX;
    vi_chn_attr.u32Width = 720;
    vi_chn_attr.u32Height = 576;
  }
  vi_chn_attr.u32BufCnt = 3;
  vi_chn_attr.enWorkMode = mode;
  vi_chn_attr.pcVideoNode = session->videopath;
  vi_chn_attr.enPixFmt = session->enImageType;

  RK_MPI_VI_SetChnAttr(vi_pipe, session->stViChn.s32ChnId, &vi_chn_attr);
  RK_MPI_VI_EnableChn(vi_pipe, session->stViChn.s32ChnId);
}


static RK_CHAR optstr[] = "?::a::c:b:l:p:I:M:r:";
static const struct option long_options[] = {
    {"aiq", optional_argument, NULL, 'a'},
    {"cfg_path", required_argument, NULL, 'c'},
    {"box_priors", required_argument, NULL, 'b'},
    {"labels_list", required_argument, NULL, 'l'},
    {"ssd_path", required_argument, NULL, 'p'},
    {"camid", required_argument, NULL, 'I'},
    {"multictx", required_argument, NULL, 'M'},
    {"help", optional_argument, NULL, '?'},
    {NULL, 0, NULL, 0},
};

static void print_usage(const RK_CHAR *name) {
  printf("usage example:\n");
#ifdef RKAIQ
  printf("\t%s [-a [iqfiles_dir]] "
         "[-I 0] "
         "[-M 0] "
         "[-b box_priors.txt] "
         "[-l coco_labels_list.txt] "
         "[-p ssd_inception_v2_rv1109_rv1126.rknn] "
         "[-c rtsp-nn.cfg]\n",
         name);
  printf("\t-a | --aiq: enable aiq with dirpath provided, eg:-a "
         "/oem/etc/iqfiles/, "
         "set dirpath empty to using path by default, without this option aiq "
         "should run in other application\n");
  printf("\t-M | --multictx: switch of multictx in isp, set 0 to disable, set "
         "1 to enable. Default: 0\n");
#else
  printf("\t%s "
         "[-I 0] "
         "[-b box_priors.txt] "
         "[-l coco_labels_list.txt] "
         "[-p ssd_inception_v2_rv1109_rv1126.rknn] "
         "[-c rtsp-nn.cfg]\n",
         name);
#endif
  printf("\t-I | --camid: camera ctx id, Default 0\n");
  printf("\t-b | --box_priors: box_priors path, Default: "
         "\"/oem/usr/share/rknn_model/box_priors.txt\"\n");
  printf("\t-l | --labels_list: labels_list path, Default: "
         "\"/oem/usr/share/rknn_model/coco_labels_list.txt\"\n");
  printf("\t-p | --ssd_path: ssd mode path, Default: "
         "\"/oem/usr/share/rknn_model/ssd_inception_v2_rv1109_rv1126.rknn\"\n");
  printf("\t-c | --cfg_path: rtsp cfg path, Default: "
         "\"/oem/usr/share/rtsp-nn.cfg\"\n");
}

int camera_start(int id, int width, int height, int fps, int format, int eptz) {
  int ret = 0;
  (void)id;
  (void)fps;
  (void)format;
  (void)eptz;

  if (pIqfilesPath) {
#ifdef RKAIQ
    rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
    SAMPLE_COMM_ISP_Init(s32CamId, hdr_mode, bMultictx, pIqfilesPath);
    SAMPLE_COMM_ISP_Run(s32CamId);
    SAMPLE_COMM_ISP_SetFrameRate(s32CamId, fps);

    register_uvc_pu_control_callback(camera_control);
#endif
  }
  // create session
  for (int i = 0; i < cfg.session_count; i++) {
    // VI create
    printf("VI[%d] create\n", i);
    SAMPLE_COMMON_VI_Start(&cfg.session_cfg[i], VI_WORK_MODE_NORMAL, s32CamId, width, height, i);
    if (i == DRAW_INDEX)
      RK_MPI_VI_StartStream(s32CamId, cfg.session_cfg[DRAW_INDEX].stViChn.s32ChnId);
  }


  for (int i = 0; i < cfg.session_count; i++) {
    // Rga create
    printf("Rga[%d] create\n", i);
    SAMPLE_COMMON_RGA_Start(&cfg.session_cfg[i], width, height, i);
    if (i == RK_NN_INDEX) {
      //vi --> rga_nn
      ret = RK_MPI_SYS_Bind(&cfg.session_cfg[RK_NN_INDEX].stViChn, &cfg.session_cfg[RK_NN_INDEX].stRgaChn);
      if (ret) {
         printf("ERROR: Bind Vi[%d] and stRgaChn[%d] failed! ret=%d\n",
          cfg.session_cfg[RK_NN_INDEX].stViChn.s32ChnId, cfg.session_cfg[RK_NN_INDEX].stRgaChn.s32ChnId, ret);
         return -1;
      }
    } else if (i == DRAW_INDEX) {
      //vi --> rga_draw
      ret = RK_MPI_SYS_Bind(&cfg.session_cfg[DRAW_INDEX].stViChn, &cfg.session_cfg[DRAW_INDEX].stRgaChn);
      if (ret) {
         printf("ERROR: Bind Vi[%d] and stRgaChn[%d] failed! ret=%d\n",
          cfg.session_cfg[DRAW_INDEX].stViChn.s32ChnId, cfg.session_cfg[DRAW_INDEX].stRgaChn.s32ChnId, ret);
         return -1;
      }
    }
  }

  start = 1;

  if (pthread_create(&th, NULL, UvcGetMediaBuffer, NULL)) {
    printf("create UvcGetMediaBuffer thread failed!\n");
    start = 0;
    return -1;
  }
  // Get the sub-stream buffer for humanoid recognition
  nndata.camera_input_width = width;//记录摄像头的宽与高
  nndata.camera_input_height = height;
  pthread_create(&read_thread, NULL, GetMediaBuffer, NULL);

  // The mainstream draws a box asynchronously based on the recognition result
  //pthread_create(&main_stream_thread, NULL, MainStream, NULL);
  return 0;
}

void camera_stop(void) {
  if (start) {
    start = 0;
    pthread_join(th, NULL);
    pthread_join(read_thread, NULL);
    pthread_join(tcp_thread, NULL);
    //pthread_join(main_stream_thread, NULL);
    for (int i = 0; i < cfg.session_count; i++) {
      RK_MPI_SYS_UnBind(&cfg.session_cfg[i].stViChn, &cfg.session_cfg[i].stRgaChn);
      RK_MPI_RGA_DestroyChn(cfg.session_cfg[i].stRgaChn.s32ChnId);
      RK_MPI_VI_DisableChn(s32CamId, cfg.session_cfg[i].stViChn.s32ChnId);
    }
#ifdef RKAIQ
    if (pIqfilesPath) {
      register_uvc_pu_control_callback(NULL);
      SAMPLE_COMM_ISP_Stop(s32CamId);
    }
#endif
  }
}

static void *TcpConnect(void *data) {
  int ready = 0;
  int ret;
  ret = tcps_accept();
  if (ret) {
    printf("accept error\n");
    exit(1);
  }
  ready = 1;
  ret = send_data(&ready, sizeof(int));//发送send_buf 字节label_and_location大小
  if (ret != sizeof(int)) {
    printf("camera not ready\n");
    return -1;
  }
}

int main(int argc, char **argv) {
  char *config = "uvc_config.sh rndis";
  RK_CHAR *pCfgPath = "/oem/usr/share/rtsp-nn.cfg";
  int ret, c;
  int model_len = 0;
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
        pIqfilesPath = "/oem/etc/iqfiles/";
      }
      break;
    case 'c':
      pCfgPath = optarg;
      break;
    case 'b':
      g_box_priors = optarg;
      break;
    case 'l':
      g_labels_list = optarg;
      break;
    case 'p':
      g_ssd_path = optarg;
      break;
    case 'I':
      s32CamId = atoi(optarg);
      break;
    case 'r':
      config = "uvc_config.sh rndis";
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

  printf("#####Device: %s\n", pDeviceName);
  printf("#####Aiq xml dirpath: %s\n\n", pIqfilesPath);
  printf("#CameraIdx: %d\n\n", s32CamId);
  printf("#bMultictx: %d\n\n", bMultictx);

  printf("cfg path is %s\n", pCfgPath);
  printf("BOX_PRIORS_TXT_PATH is %s\n", g_box_priors);
  printf("LABEL_NALE_TXT_PATH is %s\n", g_labels_list);
  printf("MODEL_PATH is %s\n", g_ssd_path);
  printf("#CameraIdx: %d\n\n", s32CamId);
  load_cfg(pCfgPath);


  // init mpi
  printf("init mpi\n");
  RK_MPI_SYS_Init();
  //init tcp
  tcp_comm();


  printf("Loading model ...\n");
  model = load_model(g_ssd_path, &model_len);
  ret = rknn_init(&nndata.ctx, model, model_len, 0);
  if (ret < 0) {
    printf("rknn_init fail! ret=%d\n", ret);
    return NULL;
  }

  // Get Model Input Output Info
  rknn_input_output_num io_num;
  ret = rknn_query(nndata.ctx, RKNN_QUERY_IN_OUT_NUM, &nndata.io_num, sizeof(nndata.io_num));
  if (ret != RKNN_SUCC) {
    printf("rknn_query fail! ret=%d\n", ret);
    return NULL;
  }
  printf("model input num: %d, output num: %d\n", nndata.io_num.n_input,
         nndata.io_num.n_output);

  printf("input tensors:\n");
  rknn_tensor_attr input_attrs[nndata.io_num.n_input];
  memset(input_attrs, 0, sizeof(input_attrs));
  for (unsigned int i = 0; i < nndata.io_num.n_input; i++) {
    input_attrs[i].index = i;
    ret = rknn_query(nndata.ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]),
                     sizeof(rknn_tensor_attr));
    if (ret != RKNN_SUCC) {
      printf("rknn_query fail! ret=%d\n", ret);
      return NULL;
    }
    printRKNNTensor(&(input_attrs[i]));
  }

  printf("output tensors:\n");
  rknn_tensor_attr output_attrs[nndata.io_num.n_output];
  memset(output_attrs, 0, sizeof(output_attrs));
  for (unsigned int i = 0; i < nndata.io_num.n_output; i++) {
    output_attrs[i].index = i;
    ret = rknn_query(nndata.ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]),
                     sizeof(rknn_tensor_attr));
    if (ret != RKNN_SUCC) {
      printf("rknn_query fail! ret=%d\n", ret);
      return NULL;
    }
    printRKNNTensor(&(output_attrs[i]));
  }
  create_rknn_list(&rknn_list_);

  //system(config);

  pthread_create(&tcp_thread, NULL, TcpConnect, NULL);

  printf("%s initial finish\n", __func__);
  signal(SIGINT, sigterm_handler);

  uvc_control_start_setcallback(camera_start);
  uvc_control_stop_setcallback(camera_stop);
  uvc_control_run(UVC_CONTROL_CAMERA);


  while (!quit) {
    if (0 == uvc_control_loop())
      break;
    usleep(500000);
  }

  camera_stop();
  uvc_control_join(UVC_CONTROL_CAMERA);

  printf("%s exit!\n", __func__);
  return 0;
}
