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

#include "rkadk_photo.h"
#include "rkadk_log.h"
#include "rkadk_media_comm.h"
#include "rkadk_param.h"
#include "rkadk_param_inner.h"
#include "rkmedia_api.h"
#include <malloc.h>
#include <rga/RgaApi.h>
#include <rga/rga.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define JPG_THM_FIND_NUM_MAX 50
#define JPG_EXIF_FLAG_LEN 6
#define JPG_DIRECTORY_ENTRY_LEN 12
#define JPG_DE_TYPE_COUNT 12
#define JPG_MP_FLAG_LEN 4
#define JPG_MP_ENTRY_LEN 16
#define JPG_THUMB_TAG_LEN 4

typedef enum {
  RKADK_JPG_LITTLE_ENDIAN, // II
  RKADK_JPG_BIG_ENDIAN,    // MM
  RKADK_JPG_BYTE_ORDER_BUTT
} RKADK_JPG_BYTE_ORDER_E;

typedef struct {
  RKADK_U16 u16Type;
  RKADK_U16 u16TypeByte;
} RKADK_JPG_DE_TYPE_S;

typedef struct {
  bool init;
  RKADK_PHOTO_DATA_RECV_FN_PTR pDataRecvFn;
} PHOTO_INFO_S;

static RKADK_JPG_DE_TYPE_S g_stJpgDEType[JPG_DE_TYPE_COUNT] = {
    {1, 1}, {2, 1}, {3, 3}, {4, 4},  {5, 8},  {6, 1},
    {7, 1}, {8, 2}, {9, 3}, {10, 8}, {11, 4}, {12, 8}};

static PHOTO_INFO_S g_stPhotoInfo[RKADK_MAX_SENSOR_CNT] = {0};

static void RKADK_PHOTO_VencOutCb0(MEDIA_BUFFER mb) {
  if (!g_stPhotoInfo[0].pDataRecvFn) {
    RKADK_LOGW("RKADK_PHOTO_PACKET_RECV_FN_PTR unregistered");
    RK_MPI_MB_ReleaseBuffer(mb);
    return;
  }

  g_stPhotoInfo[0].pDataRecvFn((RKADK_U8 *)RK_MPI_MB_GetPtr(mb),
                               RK_MPI_MB_GetSize(mb));
  RK_MPI_MB_ReleaseBuffer(mb);
}

static void RKADK_PHOTO_VencOutCb1(MEDIA_BUFFER mb) {
  if (!g_stPhotoInfo[1].pDataRecvFn) {
    RKADK_LOGW("RKADK_PHOTO_PACKET_RECV_FN_PTR unregistered");
    RK_MPI_MB_ReleaseBuffer(mb);
    return;
  }

  g_stPhotoInfo[1].pDataRecvFn((RKADK_U8 *)RK_MPI_MB_GetPtr(mb),
                               RK_MPI_MB_GetSize(mb));
  RK_MPI_MB_ReleaseBuffer(mb);
}

static void RKADK_PHOTO_SetVencAttr(RKADK_PHOTO_THUMB_ATTR_S stThumbAttr,
                                    RKADK_PARAM_PHOTO_CFG_S *pstPhotoCfg,
                                    VENC_CHN_ATTR_S *pstVencAttr) {
  VENC_ATTR_JPEG_S *pstAttrJpege = &(pstVencAttr->stVencAttr.stAttrJpege);

  memset(pstVencAttr, 0, sizeof(VENC_CHN_ATTR_S));
  pstVencAttr->stVencAttr.enType = RK_CODEC_TYPE_JPEG;
  pstVencAttr->stVencAttr.imageType = pstPhotoCfg->vi_attr.stChnAttr.enPixFmt;
  pstVencAttr->stVencAttr.u32PicWidth = pstPhotoCfg->image_width;
  pstVencAttr->stVencAttr.u32PicHeight = pstPhotoCfg->image_height;
  pstVencAttr->stVencAttr.u32VirWidth = pstPhotoCfg->image_width;
  pstVencAttr->stVencAttr.u32VirHeight = pstPhotoCfg->image_height;

  pstAttrJpege->bSupportDCF = (RK_BOOL)stThumbAttr.bSupportDCF;
  pstAttrJpege->stMPFCfg.u8LargeThumbNailNum =
      stThumbAttr.stMPFAttr.sCfg.u8LargeThumbNum;
  if (pstAttrJpege->stMPFCfg.u8LargeThumbNailNum >
      RKADK_MPF_LARGE_THUMB_NUM_MAX)
    pstAttrJpege->stMPFCfg.u8LargeThumbNailNum = RKADK_MPF_LARGE_THUMB_NUM_MAX;

  switch (stThumbAttr.stMPFAttr.eMode) {
  case RKADK_PHOTO_MPF_SINGLE:
    pstAttrJpege->enReceiveMode = VENC_PIC_RECEIVE_SINGLE;
    pstAttrJpege->stMPFCfg.astLargeThumbNailSize[0].u32Width =
        UPALIGNTO(stThumbAttr.stMPFAttr.sCfg.astLargeThumbSize[0].u32Width, 4);
    pstAttrJpege->stMPFCfg.astLargeThumbNailSize[0].u32Height =
        UPALIGNTO(stThumbAttr.stMPFAttr.sCfg.astLargeThumbSize[0].u32Height, 2);
    break;
  case RKADK_PHOTO_MPF_MULTI:
    pstAttrJpege->enReceiveMode = VENC_PIC_RECEIVE_MULTI;
    for (int i = 0; i < pstAttrJpege->stMPFCfg.u8LargeThumbNailNum; i++) {
      pstAttrJpege->stMPFCfg.astLargeThumbNailSize[i].u32Width = UPALIGNTO(
          stThumbAttr.stMPFAttr.sCfg.astLargeThumbSize[i].u32Width, 4);
      pstAttrJpege->stMPFCfg.astLargeThumbNailSize[i].u32Height = UPALIGNTO(
          stThumbAttr.stMPFAttr.sCfg.astLargeThumbSize[i].u32Height, 2);
    }
    break;
  default:
    pstAttrJpege->enReceiveMode = VENC_PIC_RECEIVE_BUTT;
    break;
  }
}

static void RKADK_PHOTO_SetChn(RKADK_U32 u32CamID,
                               RKADK_PARAM_PHOTO_CFG_S *pstPhotoCfg,
                               MPP_CHN_S *pstViChn, MPP_CHN_S *pstVencChn) {
  pstViChn->enModId = RK_ID_VI;
  pstViChn->s32DevId = 0;
  pstViChn->s32ChnId = pstPhotoCfg->vi_attr.u32ViChn;

  pstVencChn->enModId = RK_ID_VENC;
  pstVencChn->s32DevId = 0;
  pstVencChn->s32ChnId = pstPhotoCfg->venc_chn;
}

RKADK_S32 RKADK_PHOTO_Init(RKADK_PHOTO_ATTR_S *pstPhotoAttr) {
  int ret;
  MPP_CHN_S stViChn;
  MPP_CHN_S stVencChn;
  VENC_CHN_ATTR_S stVencAttr;

  RKADK_CHECK_POINTER(pstPhotoAttr, RKADK_FAILURE);
  RKADK_CHECK_CAMERAID(pstPhotoAttr->u32CamID, RKADK_FAILURE);

  RKADK_LOGI("Photo[%d, %d] Init...", pstPhotoAttr->u32CamID,
             pstPhotoAttr->enPhotoType);

  PHOTO_INFO_S *pstPhotoInfo = &g_stPhotoInfo[pstPhotoAttr->u32CamID];
  if (pstPhotoInfo->init) {
    RKADK_LOGI("photo: camera[%d] has been init", pstPhotoAttr->u32CamID);
    return 0;
  }

  pstPhotoInfo->pDataRecvFn = pstPhotoAttr->pfnPhotoDataProc;

  RKADK_PARAM_Init(NULL, NULL);
  RKADK_PARAM_PHOTO_CFG_S *pstPhotoCfg =
      RKADK_PARAM_GetPhotoCfg(pstPhotoAttr->u32CamID);
  if (!pstPhotoCfg) {
    RKADK_LOGE("RKADK_PARAM_GetPhotoCfg failed");
    return -1;
  }

  RK_MPI_SYS_Init();

  RKADK_PHOTO_SetChn(pstPhotoAttr->u32CamID, pstPhotoCfg, &stViChn, &stVencChn);

  // Create VI
  ret = RKADK_MPI_VI_Init(pstPhotoAttr->u32CamID, stViChn.s32ChnId,
                          &(pstPhotoCfg->vi_attr.stChnAttr));
  if (ret) {
    RKADK_LOGE("RKADK_MPI_VI_Init failed, ret = %d", ret);
    return ret;
  }

  // Create VENC
  RKADK_PHOTO_SetVencAttr(pstPhotoAttr->stThumbAttr, pstPhotoCfg, &stVencAttr);
  ret = RK_MPI_VENC_CreateChn(stVencChn.s32ChnId, &stVencAttr);
  if (ret) {
    RKADK_LOGE("Create Venc failed! ret=%d", ret);
    goto failed;
  }

  if (pstPhotoAttr->u32CamID == 0)
    ret = RK_MPI_SYS_RegisterOutCb(&stVencChn, RKADK_PHOTO_VencOutCb0);
  else
    ret = RK_MPI_SYS_RegisterOutCb(&stVencChn, RKADK_PHOTO_VencOutCb1);
  if (ret) {
    RKADK_LOGE("Register Output callback failed! ret=%d", ret);
    goto failed;
  }

  // The encoder defaults to continuously receiving frames from the previous
  // stage. Before performing the bind operation, set s32RecvPicNum to 0 to
  // make the encoding enter the pause state.
  VENC_RECV_PIC_PARAM_S stRecvParam;
  stRecvParam.s32RecvPicNum = 0;
  ret = RK_MPI_VENC_StartRecvFrame(stVencChn.s32ChnId, &stRecvParam);
  if (ret) {
    RKADK_LOGE("RK_MPI_VENC_StartRecvFrame failed = %d", ret);
    goto failed;
  }

  // Bind
  ret = RK_MPI_SYS_Bind(&stViChn, &stVencChn);
  if (ret) {
    RKADK_LOGE("Bind VI[%d] to VENC[%d]::JPEG failed! ret=%d", stViChn.s32ChnId,
               stVencChn.s32ChnId, ret);
    goto failed;
  }

  pstPhotoInfo->init = true;
  RKADK_LOGI("Photo[%d, %d] Init End...", pstPhotoAttr->u32CamID,
             pstPhotoAttr->enPhotoType);
  return 0;

failed:
  RKADK_LOGE("failed");
  RK_MPI_VENC_DestroyChn(stVencChn.s32ChnId);
  RKADK_MPI_VI_DeInit(pstPhotoAttr->u32CamID, stViChn.s32ChnId);
  return ret;
}

RKADK_S32 RKADK_PHOTO_DeInit(RKADK_U32 u32CamID) {
  int ret;
  MPP_CHN_S stViChn;
  MPP_CHN_S stVencChn;

  RKADK_CHECK_CAMERAID(u32CamID, RKADK_FAILURE);

  RKADK_LOGI("Photo[%d] DeInit...", u32CamID);

  PHOTO_INFO_S *pstPhotoInfo = &g_stPhotoInfo[u32CamID];
  if (!pstPhotoInfo->init) {
    RKADK_LOGI("photo: camera[%d] has been deinit", u32CamID);
    return 0;
  }

  RKADK_PARAM_PHOTO_CFG_S *pstPhotoCfg = RKADK_PARAM_GetPhotoCfg(u32CamID);
  if (!pstPhotoCfg) {
    RKADK_LOGE("RKADK_PARAM_GetPhotoCfg failed");
    return -1;
  }

  RKADK_PHOTO_SetChn(u32CamID, pstPhotoCfg, &stViChn, &stVencChn);

  // UnBind
  ret = RK_MPI_SYS_UnBind(&stViChn, &stVencChn);
  if (ret) {
    RKADK_LOGE("UnBind VI[%d] to VENC[%d]::JPEG failed! ret=%d",
               stViChn.s32ChnId, stVencChn.s32ChnId, ret);
    return -1;
  }

  // Destory VENC
  ret = RK_MPI_VENC_DestroyChn(stVencChn.s32ChnId);
  if (ret) {
    RKADK_LOGE("Destory VENC[%d] failed, ret=%d", stVencChn.s32ChnId, ret);
    return -1;
  }

  // Destory VI
  ret = RKADK_MPI_VI_DeInit(u32CamID, stViChn.s32ChnId);
  if (ret) {
    RKADK_LOGE("RKADK_MPI_VI_DeInit failed, ret=%d", ret);
    return -1;
  }

  pstPhotoInfo->pDataRecvFn = NULL;
  pstPhotoInfo->init = false;
  RKADK_LOGI("Photo[%d] DeInit End...", u32CamID);
  return 0;
}

RKADK_S32 RKADK_PHOTO_TakePhoto(RKADK_PHOTO_ATTR_S *pstPhotoAttr) {
  VENC_RECV_PIC_PARAM_S stRecvParam;

  RKADK_CHECK_POINTER(pstPhotoAttr, RKADK_FAILURE);
  RKADK_CHECK_CAMERAID(pstPhotoAttr->u32CamID, RKADK_FAILURE);

  RKADK_PARAM_PHOTO_CFG_S *pstPhotoCfg =
      RKADK_PARAM_GetPhotoCfg(pstPhotoAttr->u32CamID);
  if (!pstPhotoCfg) {
    RKADK_LOGE("RKADK_PARAM_GetPhotoCfg failed");
    return -1;
  }

  if (pstPhotoAttr->enPhotoType == RKADK_PHOTO_TYPE_LAPSE) {
    // TODO
    RKADK_LOGI("nonsupport photo type = %d", pstPhotoAttr->enPhotoType);
    return -1;
  }

  PHOTO_INFO_S *pstPhotoInfo = &g_stPhotoInfo[pstPhotoAttr->u32CamID];
  if (!pstPhotoInfo->init) {
    RKADK_LOGI("photo: camera[%d] isn't init", pstPhotoAttr->u32CamID);
    return -1;
  }

  if (pstPhotoAttr->enPhotoType == RKADK_PHOTO_TYPE_SINGLE)
    stRecvParam.s32RecvPicNum = 1;
  else
    stRecvParam.s32RecvPicNum =
        pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count;

  return RK_MPI_VENC_StartRecvFrame(pstPhotoCfg->venc_chn, &stRecvParam);
}

static RKADK_U16 RKADK_JPG_ReadU16(FILE *fd,
                                   RKADK_JPG_BYTE_ORDER_E eByteOrder) {
  RKADK_U16 u16Data;

  if (fread((char *)&u16Data, sizeof(RKADK_U16), 1, fd) != 1) {
    RKADK_LOGE("read failed");
    return -1;
  }

  if (eByteOrder == RKADK_JPG_BIG_ENDIAN)
    u16Data = RKADK_SWAP16(u16Data);

  return u16Data;
}

static RKADK_U32 RKADK_JPG_ReadU32(FILE *fd,
                                   RKADK_JPG_BYTE_ORDER_E eByteOrder) {
  RKADK_U32 u32Data;

  if (fread((char *)&u32Data, sizeof(RKADK_U32), 1, fd) != 1) {
    RKADK_LOGE("read failed");
    return -1;
  }

  if (eByteOrder == RKADK_JPG_BIG_ENDIAN)
    u32Data = RKADK_SWAP32(u32Data);

  return u32Data;
}

static RKADK_S16 RKADK_JPG_GetEntryCount(FILE *fd,
                                         RKADK_JPG_BYTE_ORDER_E eByteOrder,
                                         RKADK_U64 u64TiffHeaderOffset) {
  RKADK_U32 u32IFDOffset;
  RKADK_U16 u16EntryCount;

  // read IFD offset
  u32IFDOffset = RKADK_JPG_ReadU32(fd, eByteOrder);
  if (u32IFDOffset < 0) {
    RKADK_LOGD("read IFD offset failed");
    return -1;
  }

  // seek to IFD
  if (fseek(fd, u32IFDOffset + u64TiffHeaderOffset, SEEK_SET)) {
    RKADK_LOGD("seek to IFD failed");
    return -1;
  }

  // read IFD entry count
  u16EntryCount = RKADK_JPG_ReadU16(fd, eByteOrder);
  return u16EntryCount;
}

static RKADK_S16 RKADK_JPG_GetDETypeByte(FILE *fd,
                                         RKADK_JPG_BYTE_ORDER_E eByteOrder) {
  RKADK_U16 u16Type;

  // read DE type
  u16Type = RKADK_JPG_ReadU16(fd, eByteOrder);
  if (u16Type < 0) {
    RKADK_LOGE("read DE type failed");
    return -1;
  }

  // match type byte
  for (int i = 0; i < JPG_DE_TYPE_COUNT; i++) {
    if (u16Type == g_stJpgDEType[i].u16Type)
      return g_stJpgDEType[i].u16TypeByte;
  }

  RKADK_LOGE("not match type[%d] byte", u16Type);
  return -1;
}

static RKADK_S32 RKADK_JPG_CheckExif(FILE *fd) {
  char exifFlag[JPG_EXIF_FLAG_LEN];

  if (fread(exifFlag, JPG_EXIF_FLAG_LEN, 1, fd) != 1) {
    RKADK_LOGE("read exif flag failed");
    return -1;
  }

  if (strncmp(exifFlag, "Exif", 4)) {
    RKADK_LOGE("invaild exif flag: %s", exifFlag);
    return -1;
  }

  return 0;
}

static RKADK_S32 RKADK_JPG_CheckMPF(FILE *fd) {
  char MPFFlag[JPG_MP_FLAG_LEN];

  if (fread(MPFFlag, JPG_MP_FLAG_LEN, 1, fd) != 1) {
    RKADK_LOGE("read MPF flag failed");
    return -1;
  }

  if (strncmp(MPFFlag, "MPF", 3)) {
    RKADK_LOGE("invaild MPF flag: %s", MPFFlag);
    return -1;
  }

  return 0;
}

static RKADK_JPG_BYTE_ORDER_E RKADK_JPG_GetByteOrder(FILE *fd) {
  RKADK_JPG_BYTE_ORDER_E eByteOrder = RKADK_JPG_BYTE_ORDER_BUTT;
  RKADK_U16 u16ByteOrder = 0;

  u16ByteOrder = RKADK_JPG_ReadU16(fd, RKADK_JPG_BIG_ENDIAN);
  if (u16ByteOrder == 0x4949)
    eByteOrder = RKADK_JPG_LITTLE_ENDIAN;
  else if (u16ByteOrder == 0x4d4d)
    eByteOrder = RKADK_JPG_BIG_ENDIAN;
  else
    RKADK_LOGE("invaild byte order: 0x%4x", u16ByteOrder);

  return eByteOrder;
}

static RKADK_S32 RKADK_JPG_GetImageNum(FILE *fd,
                                       RKADK_JPG_BYTE_ORDER_E eByteOrder) {
  RKADK_U32 u32DECount;
  RKADK_U32 u32DESize;
  RKADK_U32 u32ImageNum;
  RKADK_U16 u16TypeByte;

  u16TypeByte = RKADK_JPG_GetDETypeByte(fd, eByteOrder);
  if (u16TypeByte <= 0)
    return -1;

  u32DECount = RKADK_JPG_ReadU32(fd, eByteOrder);
  if (u32DECount < 0) {
    RKADK_LOGE("read image number DE count failed");
    return -1;
  }

  u32DESize = u16TypeByte * u32DECount;
  if (u32DESize > 4) {
    RKADK_LOGE("image number size byte(0x%x) > 4 byte, unreasonable",
               u32DESize);
    return -1;
  }

  // read MP image number tag
  u32ImageNum = RKADK_JPG_ReadU32(fd, eByteOrder);
  return u32ImageNum;
}

static RKADK_S32 RKADK_JPG_ReadMPFData(FILE *fd,
                                       RKADK_JPG_THUMB_TYPE_E eThmType,
                                       RKADK_JPG_BYTE_ORDER_E eByteOrder,
                                       RKADK_U32 u32ImageNum,
                                       RKADK_U64 u64TiffHeaderOffset,
                                       RKADK_U8 *pu8Buf, RKADK_U32 *pu32Size) {
  RKADK_U32 u32ImageAttr;
  RKADK_U32 u32MPTypeCode;
  RKADK_U32 u32ImageSize;
  RKADK_U32 u32ImageOffset;
  RKADK_U16 u16MPIndex = 0;

  for (int i = 0; i < (int)u32ImageNum; i++) {
    u32ImageAttr = RKADK_JPG_ReadU32(fd, eByteOrder);
    if (u32ImageAttr < 0) {
      RKADK_LOGE("read image attr failed");
      return -1;
    }

    u32MPTypeCode = u32ImageAttr & 0x00FFFFFF;
    if (u32MPTypeCode == 0x010001 || u32MPTypeCode == 0x010002) {
      u16MPIndex++;

      if ((eThmType == RKADK_JPG_THUMB_TYPE_MFP2) && (u16MPIndex != 2))
        goto seek_next;

      // read image size
      u32ImageSize = RKADK_JPG_ReadU32(fd, eByteOrder);
      if (u32ImageSize < 0) {
        RKADK_LOGE("read image size failed");
        return -1;
      }

      // read image data offset
      u32ImageOffset = RKADK_JPG_ReadU32(fd, eByteOrder);
      if (u32ImageOffset < 0) {
        RKADK_LOGE("read image data offset failed");
        return -1;
      }

      // seek to image offset
      if (fseek(fd, u32ImageOffset + u64TiffHeaderOffset, SEEK_SET)) {
        RKADK_LOGD("seek to image offset failed");
        return -1;
      }

      if (*pu32Size < u32ImageSize)
        RKADK_LOGW("pu32Size(%d) < u32ImageSize(%d)", *pu32Size, u32ImageSize);
      else
        *pu32Size = u32ImageSize;

      // read MPF data
      if (fread(pu8Buf, *pu32Size, 1, fd) != 1) {
        RKADK_LOGE("read MPF data failed");
        return -1;
      }

      return 0;
    } else {
    seek_next:
      // seek to next MP Entry
      if (fseek(fd, JPG_MP_ENTRY_LEN - sizeof(RKADK_U32), SEEK_CUR)) {
        RKADK_LOGD("seek to next MP Entry failed");
        return -1;
      }
    }
  }

  RKADK_LOGE("not find MPF data");
  return -1;
}

static RKADK_S32 RKADK_JPG_GetMPEntryOffset(FILE *fd,
                                            RKADK_JPG_BYTE_ORDER_E eByteOrder,
                                            RKADK_U32 u32ImageNum) {
  RKADK_U32 u32DECount;
  RKADK_U32 u32DESize;
  RKADK_U16 u16TypeByte;
  RKADK_U32 u32MPEntryOffset;

  u16TypeByte = RKADK_JPG_GetDETypeByte(fd, eByteOrder);
  if (u16TypeByte <= 0)
    return -1;

  u32DECount = RKADK_JPG_ReadU32(fd, eByteOrder);
  if (u32DECount < 0) {
    RKADK_LOGE("read MP Entry DE count failed");
    return -1;
  }

  u32DESize = u16TypeByte * u32DECount;
  if (u32DESize != (JPG_MP_ENTRY_LEN * u32ImageNum)) {
    RKADK_LOGE("MP Entry total len[0x%x] != 16 * NumberOfImages[%d]",
               u32DECount, JPG_MP_ENTRY_LEN * u32ImageNum);
    return -1;
  }

  // read MP Entry Offset
  u32MPEntryOffset = RKADK_JPG_ReadU32(fd, eByteOrder);
  return u32MPEntryOffset;
}

static RKADK_S32 RKADK_JPG_ReadDCF(FILE *fd, RKADK_JPG_BYTE_ORDER_E eByteOrder,
                                   RKADK_U64 u64TiffHeaderOffset,
                                   RKADK_U8 *pu8Buf, RKADK_U32 *pu32Size) {
  RKADK_U16 u16Flag;
  RKADK_U16 u16EntryCount;
  RKADK_U16 u16DETag;
  RKADK_U32 u32DCFOffset = 0;
  RKADK_U32 u32DCFLen = 0;

  // read version: 0x002A
  u16Flag = RKADK_JPG_ReadU16(fd, eByteOrder);
  if (u16Flag != 0x002A) {
    RKADK_LOGE("invalid TIFF flag[0x%x] failed", u16Flag);
    return -1;
  }

  // get IFD0 entry count
  u16EntryCount = RKADK_JPG_GetEntryCount(fd, eByteOrder, u64TiffHeaderOffset);
  if (u16EntryCount <= 0)
    return -1;

  // seek to IFD1 offset
  if (fseek(fd, u16EntryCount * JPG_DIRECTORY_ENTRY_LEN, SEEK_CUR)) {
    RKADK_LOGD("seek to IFD1 offset failed");
    return -1;
  }

  // get IFD1 entry count
  u16EntryCount = RKADK_JPG_GetEntryCount(fd, eByteOrder, u64TiffHeaderOffset);
  if (u16EntryCount <= 0)
    return -1;

  for (int i = 0; i < u16EntryCount; i++) {
    // read IFD1 DE tag
    u16DETag = RKADK_JPG_ReadU16(fd, eByteOrder);
    if (u16DETag < 0) {
      RKADK_LOGE("read IFD1 DE tag failed");
      return -1;
    } else if (u16DETag == 0x0201 || u16DETag == 0x0202) {
      if (fseek(fd, 6, SEEK_CUR)) {
        RKADK_LOGD("seek to IFD1 DE[%d, 0x%4x] value failed", i, u16DETag);
        return -1;
      }

      if (u16DETag == 0x0202)
        u32DCFLen = RKADK_JPG_ReadU32(fd, eByteOrder);
      else if (u16DETag == 0x0201)
        u32DCFOffset = RKADK_JPG_ReadU32(fd, eByteOrder);
    } else {
      if (fseek(fd, JPG_DIRECTORY_ENTRY_LEN - sizeof(RKADK_U16), SEEK_CUR)) {
        RKADK_LOGD("seek to IFD1 next DE failed");
        return -1;
      }
    }
  }

  if (u32DCFOffset <= 0 || u32DCFLen <= 0) {
    RKADK_LOGE("invaild u32DCFOffset[%d] or u32DCFLen[%d]", u32DCFOffset,
               u32DCFLen);
    return -1;
  }

  if (fseek(fd, u32DCFOffset + u64TiffHeaderOffset, SEEK_SET)) {
    RKADK_LOGD("seek to DCF failed");
    return -1;
  }

  if (*pu32Size < u32DCFLen)
    RKADK_LOGW("pu32Size(%d) < u32DCFLen(%d)", *pu32Size, u32DCFLen);
  else
    *pu32Size = u32DCFLen;

  // read DCF
  if (fread(pu8Buf, *pu32Size, 1, fd) != 1) {
    RKADK_LOGE("read DCF failed");
    return -1;
  }

  return 0;
}

static RKADK_S32 RKADK_JPG_ReadMPF(FILE *fd, RKADK_JPG_THUMB_TYPE_E eThmType,
                                   RKADK_JPG_BYTE_ORDER_E eByteOrder,
                                   RKADK_U64 u64TiffHeaderOffset,
                                   RKADK_U8 *pu8Buf, RKADK_U32 *pu32Size) {
  RKADK_U16 u16Flag;
  RKADK_U16 u16EntryCount;
  RKADK_U16 u16DETag;
  RKADK_U32 u32ImageNum = 0;
  RKADK_U32 u32MPEntryOffset;

  // read version: 0x002A
  u16Flag = RKADK_JPG_ReadU16(fd, eByteOrder);
  if (u16Flag != 0x002A) {
    RKADK_LOGE("invalid TIFF flag[0x%x] failed", u16Flag);
    return -1;
  }

  // get IFD entry count
  u16EntryCount = RKADK_JPG_GetEntryCount(fd, eByteOrder, u64TiffHeaderOffset);
  if (u16EntryCount <= 0)
    return -1;

  for (int i = 0; i < u16EntryCount; i++) {
    // read MP IFD tag
    u16DETag = RKADK_JPG_ReadU16(fd, eByteOrder);
    if (u16DETag < 0) {
      RKADK_LOGE("read MP IFD tag failed");
      return -1;
    }

    if (u16DETag == 0xB001) {
      // Number of Images
      u32ImageNum = RKADK_JPG_GetImageNum(fd, eByteOrder);
      if (u32ImageNum <= 1) {
        RKADK_LOGE("not contain MP thumbnail, u32ImageNum: %d", u32ImageNum);
        return -1;
      } else if (u32ImageNum <= 2 && eThmType == RKADK_JPG_THUMB_TYPE_MFP2) {
        RKADK_LOGE("not contain MP2 thumbnail, u32ImageNum: %d", u32ImageNum);
        return -1;
      }
    } else if (u16DETag == 0xB002) {
      // MP Entry
      u32MPEntryOffset =
          RKADK_JPG_GetMPEntryOffset(fd, eByteOrder, u32ImageNum);
      if (u32MPEntryOffset <= 0)
        return -1;

      // seek to MP Entry
      if (fseek(fd, u32MPEntryOffset + u64TiffHeaderOffset, SEEK_SET)) {
        RKADK_LOGD("seek to MP Entry failed");
        return -1;
      }

      return RKADK_JPG_ReadMPFData(fd, eThmType, eByteOrder, u32ImageNum,
                                   u64TiffHeaderOffset, pu8Buf, pu32Size);
    } else {
      if (fseek(fd, JPG_DIRECTORY_ENTRY_LEN - sizeof(RKADK_U16), SEEK_CUR)) {
        RKADK_LOGD("seek to IFD1 next DE failed");
        return -1;
      }
    }
  }

  return -1;
}

static RKADK_S32 RKADK_JPG_GetDCF(FILE *fd, RKADK_U8 *pu8Buf,
                                  RKADK_U32 *pu32Size) {
  RKADK_U16 u16FindNum = 0;
  RKADK_U16 u16Marker;
  RKADK_U16 u16MarkerLen;
  RKADK_JPG_BYTE_ORDER_E eByteOrder;
  RKADK_S64 u64TiffHeaderOffset;

  while (!feof(fd)) {
    // read marker
    u16Marker = RKADK_JPG_ReadU16(fd, RKADK_JPG_BIG_ENDIAN);
    u16MarkerLen = RKADK_JPG_ReadU16(fd, RKADK_JPG_BIG_ENDIAN);

    if (u16Marker < 0 || u16MarkerLen < 0) {
      RKADK_LOGE("invalid u16Marker[%d] or u16MarkerLen[%d]", u16Marker,
                 u16MarkerLen);
      return -1;
    }

    // find APP1 EXIT
    if (u16Marker != 0xFFE1) {
      if (fseek(fd, u16MarkerLen - sizeof(RKADK_U16), SEEK_CUR)) {
        RKADK_LOGD("seek to next marker failed, current marker(0x%4x, 0x%2x)",
                   u16Marker, u16MarkerLen);
        break;
      }

      u16FindNum++;
      if (u16FindNum == JPG_THM_FIND_NUM_MAX) {
        RKADK_LOGE("not MPF was found in the first %d markers", u16FindNum);
        break;
      }

      continue;
    }

    if (RKADK_JPG_CheckExif(fd))
      return -1;

    if ((u64TiffHeaderOffset = ftell(fd)) == -1) {
      RKADK_LOGE("get TIFF Header offset failed");
      return -1;
    }

    if ((eByteOrder = RKADK_JPG_GetByteOrder(fd)) == RKADK_JPG_BYTE_ORDER_BUTT)
      return -1;

    return RKADK_JPG_ReadDCF(fd, eByteOrder, u64TiffHeaderOffset, pu8Buf,
                             pu32Size);
  }

  return -1;
}

static RKADK_S32 RKADK_JPG_GetMPF(FILE *fd, RKADK_JPG_THUMB_TYPE_E eThmType,
                                  RKADK_U8 *pu8Buf, RKADK_U32 *pu32Size) {
  RKADK_U16 u16FindNum = 0;
  RKADK_U16 u16Marker;
  RKADK_U16 u16MarkerLen;
  RKADK_JPG_BYTE_ORDER_E eByteOrder;
  RKADK_S64 u64TiffHeaderOffset;

  while (!feof(fd)) {
    u16Marker = RKADK_JPG_ReadU16(fd, RKADK_JPG_BIG_ENDIAN);
    u16MarkerLen = RKADK_JPG_ReadU16(fd, RKADK_JPG_BIG_ENDIAN);

    if (u16Marker < 0 || u16MarkerLen < 0) {
      RKADK_LOGE("invalid u16Marker[%d] or u16MarkerLen[%d]", u16Marker,
                 u16MarkerLen);
      return -1;
    }

    // find APP1 EXIT
    if (u16Marker != 0xFFE2) {
      if (fseek(fd, u16MarkerLen - sizeof(RKADK_U16), SEEK_CUR)) {
        RKADK_LOGD("seek to next marker failed, current marker(0x%4x, 0x%2x)",
                   u16Marker, u16MarkerLen);
        break;
      }

      u16FindNum++;
      if (u16FindNum == JPG_THM_FIND_NUM_MAX) {
        RKADK_LOGE("not MPF was found in the first %d markers", u16FindNum);
        break;
      }

      continue;
    }

    if (RKADK_JPG_CheckMPF(fd))
      return -1;

    if ((u64TiffHeaderOffset = ftell(fd)) == -1) {
      RKADK_LOGE("get TIFF Header offset failed");
      return -1;
    }

    if ((eByteOrder = RKADK_JPG_GetByteOrder(fd)) == RKADK_JPG_BYTE_ORDER_BUTT)
      return -1;

    return RKADK_JPG_ReadMPF(fd, eThmType, eByteOrder, u64TiffHeaderOffset,
                             pu8Buf, pu32Size);
  }

  return -1;
}

static RKADK_S32 RKADK_PHOTO_RgaProcess(void *pSrcPtr,
                                        MB_IMAGE_INFO_S stImageInfo,
                                        RKADK_THUMB_ATTR_S *pstThumbAttr,
                                        int dstFormat) {
  int ret;
  RKADK_U32 u32DstDataLen;
  rga_info_t srcInfo;
  rga_info_t dstInfo;

  if (dstFormat == RK_FORMAT_YCbCr_420_SP)
    u32DstDataLen = pstThumbAttr->u32Width * pstThumbAttr->u32Height * 3 / 2;
  else if (dstFormat == RK_FORMAT_RGB_565)
    u32DstDataLen = pstThumbAttr->u32Width * pstThumbAttr->u32Height * 2;
  else if (dstFormat == RK_FORMAT_RGB_888)
    u32DstDataLen = pstThumbAttr->u32Width * pstThumbAttr->u32Height * 3;
  else
    return -1;

  if (pstThumbAttr->u32BufSize < u32DstDataLen) {
    RKADK_LOGE("u32DstDataLen[%d] > buffer size[%d]", u32DstDataLen,
               pstThumbAttr->u32BufSize);
    return -1;
  }

  ret = c_RkRgaInit();
  if (ret < 0) {
    RKADK_LOGE("c_RkRgaInit failed(%d)", ret);
    return -1;
  }

  memset(&srcInfo, 0, sizeof(rga_info_t));
  srcInfo.fd = -1;
  srcInfo.virAddr = pSrcPtr;
  srcInfo.mmuFlag = 1;
  srcInfo.rotation = 0;
  rga_set_rect(&srcInfo.rect, 0, 0, stImageInfo.u32Width, stImageInfo.u32Height,
               stImageInfo.u32HorStride, stImageInfo.u32VerStride,
               RK_FORMAT_YCbCr_420_SP);

  memset(&dstInfo, 0, sizeof(rga_info_t));
  dstInfo.fd = -1;
  dstInfo.virAddr = pstThumbAttr->pu8Buf;
  dstInfo.mmuFlag = 1;
  rga_set_rect(&dstInfo.rect, 0, 0, pstThumbAttr->u32Width,
               pstThumbAttr->u32Height, pstThumbAttr->u32Width,
               pstThumbAttr->u32Height, dstFormat);

  ret = c_RkRgaBlit(&srcInfo, &dstInfo, NULL);
  if (ret)
    RKADK_LOGE("c_RkRgaBlit scale failed(%d)", ret);

  c_RkRgaDeInit();

  pstThumbAttr->u32BufSize = u32DstDataLen;
  RKADK_LOGD("done[%d]", ret);
  return ret;
}

static RKADK_S32 RKADK_PHOTO_JpgDecode(RKADK_U8 *pu8JpgBuf,
                                       RKADK_U32 u32JpgBufLen,
                                       RKADK_THUMB_ATTR_S *pstThumbAttr) {
  int ret = 0;
  RKADK_S32 s32VdecChnID = 0;
  VDEC_CHN_ATTR_S stVdecAttr;
  MEDIA_BUFFER jpg_mb = NULL;
  MEDIA_BUFFER mb = NULL;
  RKADK_U32 mbSize = 0;
  MB_IMAGE_INFO_S stImageInfo;
  int dstFormat;

  RKADK_PARAM_THUMB_CFG_S *ptsThumbCfg = RKADK_PARAM_GetThumbCfg();
  if (!ptsThumbCfg) {
    RKADK_LOGE("RKADK_PARAM_GetThumbCfg failed");
    return -1;
  }

  // Jpg Decode
  RK_MPI_SYS_Init();

  stVdecAttr.enCodecType = RK_CODEC_TYPE_JPEG;
  stVdecAttr.enMode = VIDEO_MODE_FRAME;
  stVdecAttr.enDecodecMode = VIDEO_DECODEC_HADRWARE;
  ret = RK_MPI_VDEC_CreateChn(s32VdecChnID, &stVdecAttr);
  if (ret) {
    printf("Create VDEC[0] failed[%d]!\n", ret);
    return ret;
  }

  jpg_mb = RK_MPI_MB_CreateBuffer(u32JpgBufLen, RK_FALSE, 0);
  if (!jpg_mb) {
    RKADK_LOGE("no space left");
    ret = -1;
    goto exit;
  }

  memcpy(RK_MPI_MB_GetPtr(jpg_mb), pu8JpgBuf, u32JpgBufLen);
  RK_MPI_MB_SetSize(jpg_mb, u32JpgBufLen);
  ret = RK_MPI_SYS_SendMediaBuffer(RK_ID_VDEC, s32VdecChnID, jpg_mb);
  if (ret) {
    RKADK_LOGE("RK_MPI_SYS_SendMediaBuffer failed[%d]", ret);
    goto exit;
  }

  mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VDEC, s32VdecChnID, -1);
  if (!mb) {
    RKADK_LOGE("RK_MPI_SYS_GetMediaBuffer failed");
    ret = -1;
    goto exit;
  }

  mbSize = RK_MPI_MB_GetSize(mb);
  ret = RK_MPI_MB_GetImageInfo(mb, &stImageInfo);
  if (ret) {
    RKADK_LOGE("RK_MPI_MB_GetImageInfo failed[%d]", ret);
    goto stop_get_mb;
  }
  RKADK_LOGD("mb[%d, %d, %d, %d], type = %d, size = %d", stImageInfo.u32Width,
             stImageInfo.u32Height, stImageInfo.u32HorStride,
             stImageInfo.u32VerStride, stImageInfo.enImgType, mbSize);

  if (stImageInfo.enImgType != IMAGE_TYPE_NV12)
    RKADK_LOGW("Jpg decode output format != NV12");

  if (pstThumbAttr->enType == RKADK_THUMB_TYPE_RGB565)
    dstFormat = RK_FORMAT_RGB_565;
  else if (pstThumbAttr->enType == RKADK_THUMB_TYPE_RGB888)
    dstFormat = RK_FORMAT_RGB_888;
  else
    dstFormat = RK_FORMAT_YCbCr_420_SP;

  if (RK_FORMAT_YCbCr_420_SP == dstFormat &&
      stImageInfo.u32HorStride == pstThumbAttr->u32Width &&
      stImageInfo.u32VerStride == pstThumbAttr->u32Height) {
    if (pstThumbAttr->u32BufSize < mbSize)
      RKADK_LOGW("buffer size[%d] < mbSize[%d]", pstThumbAttr->u32BufSize,
                 mbSize);
    else
      pstThumbAttr->u32BufSize = mbSize;
    memcpy(pstThumbAttr->pu8Buf, RK_MPI_MB_GetPtr(mb),
           pstThumbAttr->u32BufSize);
  } else {
    ret = RKADK_PHOTO_RgaProcess(RK_MPI_MB_GetPtr(mb), stImageInfo,
                                 pstThumbAttr, dstFormat);
  }

stop_get_mb:
  if (RK_MPI_SYS_StopGetMediaBuffer(RK_ID_VDEC, s32VdecChnID))
    RKADK_LOGW("RK_MPI_SYS_StopGetMediaBuffer faield");

exit:
  RK_MPI_VDEC_DestroyChn(s32VdecChnID);

  if (jpg_mb)
    RK_MPI_MB_ReleaseBuffer(jpg_mb);

  if (mb)
    RK_MPI_MB_ReleaseBuffer(mb);

  return ret;
}

static RKADK_S32 RKADK_PHOTO_BuildInThm(FILE *fd,
                                        RKADK_THUMB_ATTR_S *pstThumbAttr) {

  char tag[JPG_THUMB_TAG_LEN];
  RKADK_U32 u32Size;

  if (fseek(fd, 0, SEEK_END)) {
    RKADK_LOGE("seek file end failed");
    return -1;
  }

  if (fwrite(pstThumbAttr->pu8Buf, pstThumbAttr->u32BufSize, 1, fd) != 1) {
    RKADK_LOGE("write thm data failed");
    return -1;
  }

  if (fwrite((char *)&pstThumbAttr->u32Width, 4, 1, fd) != 1) {
    RKADK_LOGE("write thm width failed");
    return -1;
  }

  if (fwrite((char *)&pstThumbAttr->u32Height, 4, 1, fd) != 1) {
    RKADK_LOGE("write thm height failed");
    return -1;
  }

  // width: 4byte, height: 4byte
  u32Size = pstThumbAttr->u32BufSize + 8;
  if (fwrite((char *)&u32Size, sizeof(RKADK_U32), 1, fd) != 1) {
    RKADK_LOGE("write thm len failed");
    return -1;
  }

  tag[0] = 't';
  tag[1] = 'h';
  tag[2] = 'm';
  tag[3] = pstThumbAttr->enType;

  if (fwrite(tag, JPG_THUMB_TAG_LEN, 1, fd) != 1) {
    RKADK_LOGE("write thm tag failed");
    return -1;
  }

  RKADK_LOGD("done");
  return 0;
}

static RKADK_S32 RKADK_PHOTO_GetThmInFile(FILE *fd,
                                          RKADK_THUMB_ATTR_S *pstThumbAttr) {
  int ret = -1;
  char tag[JPG_THUMB_TAG_LEN];
  RKADK_U32 u32Size = 0;

  if (pstThumbAttr->enType == RKADK_THUMB_TYPE_JPEG)
    return -1;

  // seek to file end
  if (fseek(fd, 0, SEEK_END)) {
    RKADK_LOGE("seek file end failed");
    return -1;
  }

  while (1) {
    // offset: 4btye tag
    if (fseek(fd, -JPG_THUMB_TAG_LEN, SEEK_CUR)) {
      RKADK_LOGE("seek file end failed");
      break;
    }

    // read thm tag
    if (fread(tag, JPG_THUMB_TAG_LEN, 1, fd) != 1) {
      RKADK_LOGE("read jpg thumb tag failed");
      break;
    }

    if (tag[2] == 0xFF && tag[3] == 0xD9) {
      RKADK_LOGD("read jpg EOF tag(0xFFD9)");
      break;
    }

    if (tag[0] != 't' || tag[1] != 'h' || tag[2] != 'm') {
      RKADK_LOGD("can't read thm tag");
      break;
    }

    RKADK_LOGD("tag[0] = %d, %c", tag[0], tag[0]);
    RKADK_LOGD("tag[1] = %d, %c", tag[1], tag[1]);
    RKADK_LOGD("tag[2] = %d, %c", tag[2], tag[2]);
    RKADK_LOGD("tag[3] = %d", tag[3]);
    RKADK_LOGD("pstThumbAttr->enType = %d", pstThumbAttr->enType);

    // offset: 4btye tag + 4btye size
    if (fseek(fd, -(JPG_THUMB_TAG_LEN + 4), SEEK_CUR)) {
      RKADK_LOGE("seek file end failed");
      break;
    }

    // read thm size
    if (fread(&u32Size, 4, 1, fd) != 1) {
      RKADK_LOGE("read jpg thumb tag failed");
      break;
    }
    RKADK_LOGD("u32Size = %d", u32Size);

    if (tag[3] == pstThumbAttr->enType) {
      // 8: 4byte width + 4byte height
      RKADK_U32 u32DataLen = u32Size - 8;

      // offset: thm data size + 4byte thm size
      if (fseek(fd, -(u32Size + 4), SEEK_CUR)) {
        RKADK_LOGE("seek file end failed");
        break;
      }

      if (!pstThumbAttr->pu8Buf) {
        pstThumbAttr->pu8Buf = (RKADK_U8 *)malloc(u32DataLen);
        if (!pstThumbAttr->pu8Buf) {
          RKADK_LOGE("malloc thumbnail buffer[%d] failed", u32DataLen);
          break;
        }
        RKADK_LOGD("malloc thumbnail buffer[%p], u32DataLen[%d]",
                   pstThumbAttr->pu8Buf, u32DataLen);

        pstThumbAttr->u32BufSize = u32DataLen;
      } else {
        if (u32DataLen > pstThumbAttr->u32BufSize)
          RKADK_LOGW("buffer size[%d] < thumbnail data len[%d]",
                     pstThumbAttr->u32BufSize, u32DataLen);
        else
          pstThumbAttr->u32BufSize = u32DataLen;
      }

      // read thm data
      if (fread(pstThumbAttr->pu8Buf, pstThumbAttr->u32BufSize, 1, fd) != 1) {
        RKADK_LOGE("read jpg thumb data failed");
        break;
      }

      // seek the remain data
      if (u32DataLen > pstThumbAttr->u32BufSize) {
        if (fseek(fd, u32DataLen - pstThumbAttr->u32BufSize, SEEK_CUR)) {
          RKADK_LOGE("seek remain data failed");
          break;
        }
      }

      // read thm width
      if (fread(&(pstThumbAttr->u32Width), 4, 1, fd) != 1) {
        RKADK_LOGE("read jpg thumb width failed");
        break;
      }

      // read thm height
      if (fread(&(pstThumbAttr->u32Height), 4, 1, fd) != 1) {
        RKADK_LOGE("read jpg thumb height failed");
        break;
      }

      ret = 0;
      RKADK_LOGD("u32Width = %d, u32Height = %d", pstThumbAttr->u32Width,
                 pstThumbAttr->u32Height);
      RKADK_LOGD("done");
      break;
    } else {
      if (fseek(fd, -(u32Size + 4), SEEK_CUR)) {
        RKADK_LOGE("seek failed");
        break;
      }
    }
  }

  if (ret) {
    if (fseek(fd, 0, SEEK_SET)) {
      RKADK_LOGE("seek jpg file header failed");
      ret = 0;
    }

    RKADK_PHOTO_ThumbBufFree(pstThumbAttr);
  }

  return ret;
}

static RKADK_S32 RKADK_PHOTO_ThmBufMalloc(RKADK_THUMB_ATTR_S *pstThumbAttr) {
  if (pstThumbAttr->pu8Buf)
    return 0;

  switch (pstThumbAttr->enType) {
  case RKADK_THUMB_TYPE_NV12:
    pstThumbAttr->u32BufSize =
        pstThumbAttr->u32Width * pstThumbAttr->u32Height * 3 / 2;
    break;
  case RKADK_THUMB_TYPE_JPEG:
    pstThumbAttr->u32BufSize = pstThumbAttr->u32Width * pstThumbAttr->u32Height;
    break;
  case RKADK_THUMB_TYPE_RGB565:
    pstThumbAttr->u32BufSize =
        pstThumbAttr->u32Width * pstThumbAttr->u32Height * 2;
    break;
  case RKADK_THUMB_TYPE_RGB888:
    pstThumbAttr->u32BufSize =
        pstThumbAttr->u32Width * pstThumbAttr->u32Height * 3;
    break;
  default:
    RKADK_LOGE("Invalid enType[%d]", pstThumbAttr->enType);
    return -1;
  }

  pstThumbAttr->pu8Buf = (RKADK_U8 *)malloc(pstThumbAttr->u32BufSize);
  if (!pstThumbAttr->pu8Buf) {
    RKADK_LOGE("malloc pu8Buf failed, u32BufSize = %d",
               pstThumbAttr->u32BufSize);
    return -1;
  }

  RKADK_LOGD("malloc thumbnail buffer[%p], u32BufSize[%d]",
             pstThumbAttr->pu8Buf, pstThumbAttr->u32BufSize);
  return 0;
}

static RKADK_S32 RKADK_PHOTO_GetThumb(RKADK_CHAR *pszFileName,
                                      RKADK_JPG_THUMB_TYPE_E eThmType,
                                      RKADK_THUMB_ATTR_S *pstThumbAttr) {
  FILE *fd = NULL;
  RKADK_S32 ret = -1;
  RKADK_U16 u16Marker;
  RKADK_U8 *pu8JpgBuf;
  RKADK_U32 u32JpgBufLen = 0;
  RKADK_U32 *pu32JpgBufLen;

  RKADK_PARAM_Init(NULL, NULL);
  RKADK_PARAM_THUMB_CFG_S *ptsThumbCfg = RKADK_PARAM_GetThumbCfg();
  if (!ptsThumbCfg) {
    RKADK_LOGE("RKADK_PARAM_GetThumbCfg failed");
    return -1;
  }

  if (!pstThumbAttr->u32Width || !pstThumbAttr->u32Height ||
      pstThumbAttr->enType == RKADK_THUMB_TYPE_JPEG) {
    pstThumbAttr->u32Width = UPALIGNTO(ptsThumbCfg->thumb_width, 4);
    pstThumbAttr->u32Height = UPALIGNTO(ptsThumbCfg->thumb_height, 2);
  }

  if (pstThumbAttr->u32Width != UPALIGNTO(pstThumbAttr->u32Width, 4)) {
    RKADK_LOGE("Width is not 4-aligned");
    return -1;
  }

  if (pstThumbAttr->u32Height != UPALIGNTO(pstThumbAttr->u32Height, 2)) {
    RKADK_LOGE("Height is not 2-aligned");
    return -1;
  }

  if (pstThumbAttr->enType < RKADK_THUMB_TYPE_NV12 ||
      pstThumbAttr->enType > RKADK_THUMB_TYPE_RGB888) {
    RKADK_LOGE("Invalid thumb type = %d", pstThumbAttr->enType);
    return -1;
  }

  fd = fopen(pszFileName, "r+");
  if (!fd) {
    RKADK_LOGE("open %s failed", pszFileName);
    return -1;
  }

  ret = RKADK_PHOTO_GetThmInFile(fd, pstThumbAttr);
  if (!ret)
    goto exit;

  if (RKADK_PHOTO_ThmBufMalloc(pstThumbAttr))
    goto exit;

  memset(pstThumbAttr->pu8Buf, 0, pstThumbAttr->u32BufSize);

  // check SOI
  u16Marker = RKADK_JPG_ReadU16(fd, RKADK_JPG_BIG_ENDIAN);
  if (u16Marker != 0xFFD8) {
    RKADK_LOGE("not find SOI marker");
    goto exit;
  }

  if (pstThumbAttr->enType != RKADK_THUMB_TYPE_JPEG) {
    u32JpgBufLen = ptsThumbCfg->thumb_width * ptsThumbCfg->thumb_height;
    pu8JpgBuf = (RKADK_U8 *)malloc(u32JpgBufLen);
    if (!pu8JpgBuf) {
      RKADK_LOGE("malloc pu8JpgBuf failed");
      goto exit;
    }
    RKADK_LOGD("malloc temp jpg buffer[%p]", pu8JpgBuf);
    pu32JpgBufLen = &u32JpgBufLen;
  } else {
    pu32JpgBufLen = &(pstThumbAttr->u32BufSize);
    pu8JpgBuf = pstThumbAttr->pu8Buf;
  }

  switch (eThmType) {
  case RKADK_JPG_THUMB_TYPE_DCF:
    ret = RKADK_JPG_GetDCF(fd, pu8JpgBuf, pu32JpgBufLen);
    break;

  case RKADK_JPG_THUMB_TYPE_MFP1:
  case RKADK_JPG_THUMB_TYPE_MFP2:
    ret = RKADK_JPG_GetMPF(fd, eThmType, pu8JpgBuf, pu32JpgBufLen);
    break;

  default:
    RKADK_LOGE("invalid type: %d", eThmType);
    break;
  }

  if (ret) {
    RKADK_LOGE("Get Jpg thumbnail failed");
    goto exit;
  }

  if (pstThumbAttr->enType == RKADK_THUMB_TYPE_JPEG)
    goto exit;

  ret = RKADK_PHOTO_JpgDecode(pu8JpgBuf, u32JpgBufLen, pstThumbAttr);
  if (!ret) {
    if (RKADK_PHOTO_BuildInThm(fd, pstThumbAttr))
      RKADK_LOGE("RKADK_PHOTO_BuildInThm failed");
  }

  if (pu8JpgBuf) {
    RKADK_LOGD("free temp jpg buffer[%p]", pu8JpgBuf);
    free(pu8JpgBuf);
  }

exit:
  if (fd)
    fclose(fd);

  return ret;
}

RKADK_S32 RKADK_PHOTO_GetThmInJpg(RKADK_CHAR *pszFileName,
                                  RKADK_JPG_THUMB_TYPE_E eThmType,
                                  RKADK_U8 *pu8Buf, RKADK_U32 *pu32Size) {
  int ret;
  RKADK_THUMB_ATTR_S stThumbAttr;

  RKADK_CHECK_POINTER(pszFileName, RKADK_FAILURE);
  RKADK_CHECK_POINTER(pu8Buf, RKADK_FAILURE);

  stThumbAttr.u32Width = 0;
  stThumbAttr.u32Height = 0;
  stThumbAttr.enType = RKADK_THUMB_TYPE_JPEG;
  stThumbAttr.pu8Buf = pu8Buf;
  stThumbAttr.u32BufSize = *pu32Size;

  ret = RKADK_PHOTO_GetThumb(pszFileName, eThmType, &stThumbAttr);
  *pu32Size = stThumbAttr.u32BufSize;

  return ret;
}

RKADK_S32 RKADK_PHOTO_GetThmInJpgEx(RKADK_CHAR *pszFileName,
                                    RKADK_JPG_THUMB_TYPE_E eThmType,
                                    RKADK_THUMB_ATTR_S *pstThumbAttr) {
  int ret;

  RKADK_CHECK_POINTER(pszFileName, RKADK_FAILURE);
  RKADK_CHECK_POINTER(pstThumbAttr, RKADK_FAILURE);

  ret = RKADK_PHOTO_GetThumb(pszFileName, eThmType, pstThumbAttr);
  if (ret)
    RKADK_PHOTO_ThumbBufFree(pstThumbAttr);

  return ret;
}

RKADK_S32 RKADK_PHOTO_ThumbBufFree(RKADK_THUMB_ATTR_S *pstThumbAttr) {
  RKADK_CHECK_POINTER(pstThumbAttr, RKADK_FAILURE);

  if (pstThumbAttr->pu8Buf) {
    RKADK_LOGD("free thumbnail buffer[%p]", pstThumbAttr->pu8Buf);
    free(pstThumbAttr->pu8Buf);
    pstThumbAttr->pu8Buf = NULL;
  }

  return 0;
}
