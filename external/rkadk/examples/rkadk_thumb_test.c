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

#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rkadk_common.h"
#include "rkadk_log.h"
#include "rkadk_photo.h"
#include "rkadk_thumb.h"

#define THUMB_TEST_SAVE_FILE
//#define THUMB_ONLY_TEST_JPG

extern int optind;
extern char *optarg;

static bool is_quit = false;
static RKADK_CHAR optstr[] = "i:t:T:f:W:H:h";

static void print_usage(const RKADK_CHAR *name) {
  printf("usage example:\n");
  printf("\t%s [-i /tmp/xxx.mp4] [-t 0]\n", name);
  printf("\t-i: test file\n");
  printf("\t-T: thumbnail type, default JPG, options: NV12, JPG, RGB565, "
         "RBG888\n");
  printf("\t-t: JPG thumbnail type, default DCF, options: DCF, MPF1, MPF2\n");
  printf("\t-f: file type, default mp4, options: mp4, jpg\n");
  printf("\t-W: thumbnail width, default obtained from ini\n");
  printf("\t-H: thumbnail height, default obtained from ini\n");
}

static void sigterm_handler(int sig) {
  fprintf(stderr, "signal %d\n", sig);
  is_quit = true;
}

int main(int argc, char *argv[]) {
  int c;
  unsigned int count = 1;
  RKADK_CHAR *pInuptPath = "/userdata/RecordTest_0.mp4";
  char filePath[RKADK_MAX_FILE_PATH_LEN];
  RKADK_JPG_THUMB_TYPE_E eJpgThumbType = RKADK_JPG_THUMB_TYPE_DCF;
  bool bIsMp4 = true;
  const char *postfix = "jpg";

#ifdef THUMB_ONLY_TEST_JPG
  int buf_size = 1024 * 1024;
  RKADK_U32 size = buf_size;
  RKADK_U8 buffer[size];
#else
  RKADK_THUMB_ATTR_S stThumbAttr;
  memset(&stThumbAttr, 0, sizeof(RKADK_THUMB_ATTR_S));
  stThumbAttr.enType = RKADK_THUMB_TYPE_JPEG;
#endif

  while ((c = getopt(argc, argv, optstr)) != -1) {
    switch (c) {
    case 'i':
      pInuptPath = optarg;
      break;
    case 'f':
      if (strstr(optarg, "jpg"))
        bIsMp4 = false;
      break;
#ifdef THUMB_ONLY_TEST_JPG
    case 'W':
    case 'H':
    case 'T':
      break;
#else
    case 'W':
      stThumbAttr.u32Width = atoi(optarg);
      break;
    case 'H':
      stThumbAttr.u32Height = atoi(optarg);
      break;
    case 'T':
      if (strstr(optarg, "NV12")) {
        stThumbAttr.enType = RKADK_THUMB_TYPE_NV12;
        postfix = "yuv";
      } else if (strstr(optarg, "RGB565")) {
        stThumbAttr.enType = RKADK_THUMB_TYPE_RGB565;
        postfix = "rgb565";
      } else if (strstr(optarg, "RGB888")) {
        stThumbAttr.enType = RKADK_THUMB_TYPE_RGB888;
        postfix = "rgb888";
      }
      break;
#endif
    case 't':
      if (strstr(optarg, "MPF1"))
        eJpgThumbType = RKADK_JPG_THUMB_TYPE_MFP1;
      else if (strstr(optarg, "MPF2"))
        eJpgThumbType = RKADK_JPG_THUMB_TYPE_MFP2;
      break;
    case 'h':
    default:
      print_usage(argv[0]);
      optind = 0;
      return 0;
    }
  }
  optind = 0;

  if (!pInuptPath) {
    RKADK_LOGE("Please input test file");
    return -1;
  }

  RKADK_LOGD("#get thm file: %s", pInuptPath);

  signal(SIGINT, sigterm_handler);

  while (!is_quit) {
#ifdef THUMB_ONLY_TEST_JPG
    size = buf_size;

    if (bIsMp4) {
#ifdef ROCKIT
      if (RKADK_GetThmInMp4(pInuptPath, buffer, &size)) {
        RKADK_LOGE("RKADK_GetThmInMp4 failed");
        return -1;
      }
#else
      RKADK_LOGI("Don't enable rkadk rockit");
#endif
    } else {
      RKADK_LOGD("eJpgThumbType: %d", eJpgThumbType);
      if (RKADK_PHOTO_GetThmInJpg(pInuptPath, eJpgThumbType, buffer, &size)) {
        RKADK_LOGE("RKADK_PHOTO_GetThmInJpg failed");
        return -1;
      }
    }
    RKADK_LOGD("%s: size: %d, count: %d", postfix, size, count);

#ifdef THUMB_TEST_SAVE_FILE
    if (size > 0) {
      FILE *file = NULL;
      sprintf(filePath, "/tmp/thm_test_%u.%s", count, "jpg");
      file = fopen(filePath, "w");
      if (!file) {
        RKADK_LOGE("Create file(%s) failed", filePath);
        return -1;
      }

      fwrite(buffer, 1, size, file);
      fclose(file);
      RKADK_LOGD("save %s done", filePath);
    }
#endif

#else
    if (bIsMp4) {
#ifdef ROCKIT
      if (RKADK_GetThmInMp4Ex(pInuptPath, &stThumbAttr)) {
        RKADK_LOGE("RKADK_GetThmInMp4Ex failed");
        return -1;
      }
#else
      RKADK_LOGI("Don't enable rkadk rockit");
#endif
    } else {
      RKADK_LOGD("eJpgThumbType: %d", eJpgThumbType);
      if (RKADK_PHOTO_GetThmInJpgEx(pInuptPath, eJpgThumbType, &stThumbAttr)) {
        RKADK_LOGE("RKADK_PHOTO_GetThmInJpg failed");
        return -1;
      }
    }
    RKADK_LOGD("%s[%d, %d]: size: %d, count: %d", postfix, stThumbAttr.u32Width,
               stThumbAttr.u32Height, stThumbAttr.u32BufSize, count);

#ifdef THUMB_TEST_SAVE_FILE
    if (stThumbAttr.u32BufSize > 0) {
      FILE *file = NULL;
      sprintf(filePath, "/tmp/thm_test_%u.%s", count, postfix);
      file = fopen(filePath, "w");
      if (!file) {
        RKADK_LOGE("Create file(%s) failed", filePath);
        return -1;
      }

      fwrite(stThumbAttr.pu8Buf, 1, stThumbAttr.u32BufSize, file);
      fclose(file);
      RKADK_LOGD("save %s done", filePath);
    }
#endif

    if (bIsMp4) {
#ifdef ROCKIT
      RKADK_ThmBufFree(&stThumbAttr);
#endif
    } else {
      RKADK_PHOTO_ThumbBufFree(&stThumbAttr);
    }
#endif
    count--;
    if (count <= 0)
      break;

    usleep(2 * 1000 * 1000);
  }

  return 0;
}
