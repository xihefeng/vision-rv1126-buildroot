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

#include "recorder.h"

#include <stdint.h>
#include <string.h>

#include <rkadk/rkadk_record.h>

typedef struct sRecorder {
    RecorderState state;
    RKADK_MW_PTR adk_rec;
} Recorder;

RKADK_MW_PTR g_recorder = NULL;

static int cvr_recorder_make_filename(RKADK_MW_PTR pRecorder, RKADK_U32 u32FileCnt,
                  RKADK_CHAR (*paszFilename)[RKADK_MAX_FILE_PATH_LEN]) {
  static uint32_t index = 0;

  RKADK_LOGD("u32FileCnt:%d, pRecorder:%p", u32FileCnt, pRecorder);

  if (index >= 10)
    index = 0;

  for (uint32_t i = 0; i < u32FileCnt; i++) {
    sprintf(paszFilename[i], "/tmp/record_test_%u.mp4", index);
    index++;
  }

  return 0;
}

int cvr_recorder_start(int cam_id)
{
    RKADK_RECORD_ATTR_S rec_attr;

    memset(&rec_attr, 0, sizeof(rec_attr));

    // set default value
    rec_attr.s32CamID = cam_id;
    rec_attr.enRecType = RKADK_REC_TYPE_NORMAL;
    rec_attr.pfnRequestFileNames = cvr_recorder_make_filename;

    RKADK_RECORD_Create(&rec_attr, &g_recorder);
    RKADK_RECORD_Start(g_recorder);

    return 0;
}

int cvr_recorder_stop(void)
{
    RKADK_RECORD_Destroy(g_recorder);
    g_recorder = NULL;
    return 0;
}
