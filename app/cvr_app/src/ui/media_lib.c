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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "lvgl.h"
#include "resource/resource_manager.h"
#include "media_lib.h"
#include "lv_freetype.h"
#include "player.h"
#include "settings.h"

extern lv_ft_info_t ttf_info_24;
extern lv_ft_info_t ttf_info_28;
extern lv_ft_info_t ttf_info_32;

static lv_obj_t *media_lib_main_obj = NULL;
static lv_obj_t *media_lib_title_obj = NULL;
static lv_obj_t *media_lib_img_bg_obj = NULL;

static lv_img_dsc_t *icon_return_r = NULL;
static lv_img_dsc_t *icon_return_p = NULL;

static lv_img_dsc_t *icon_bg_r = NULL;
static lv_img_dsc_t *icon_bg_p = NULL;
static lv_img_dsc_t *icon_video_f_01 = NULL;
static lv_img_dsc_t *icon_video_p_01 = NULL;
static lv_img_dsc_t *icon_video_u_01 = NULL;
static lv_img_dsc_t *icon_photo_01 = NULL;
static lv_img_dsc_t *list_icon_play = NULL;

static lv_img_dsc_t *test_img = NULL;

static void media_lib_release_res(void)
{
    release_resource(&icon_return_r);
    release_resource(&icon_return_p);
    release_resource(&icon_bg_r);
    release_resource(&icon_bg_p);
    release_resource(&icon_video_f_01);
    release_resource(&icon_video_p_01);
    release_resource(&icon_video_u_01);
    release_resource(&icon_photo_01);
    release_resource(&test_img);
    release_resource(&list_icon_play);
}

static void media_lib_load_res(void)
{
    media_lib_release_res();

    icon_return_r = load_resource("/oem/cvr_res/ui_1280x720/icon_return_01.png");
    icon_return_p = load_resource("/oem/cvr_res/ui_1280x720/icon_return_02.png");
    icon_bg_r = load_resource("/oem/cvr_res/ui_1280x720/media_icon_bg_01.png");
    icon_bg_p = load_resource("/oem/cvr_res/ui_1280x720/media_icon_bg_02.png");
    icon_video_f_01 = load_resource("/oem/cvr_res/ui_1280x720/icon_video_f_01.png");
    icon_video_p_01 = load_resource("/oem/cvr_res/ui_1280x720/icon_video_p_01.png");
    icon_video_u_01 = load_resource("/oem/cvr_res/ui_1280x720/icon_video_u_01.png");
    icon_photo_01 = load_resource("/oem/cvr_res/ui_1280x720/icon_photo_01.png");
    list_icon_play = load_resource("/oem/cvr_res/ui_1280x720/list_icon_play.png");

    test_img = load_resource("/oem/cvr_res/ui_1280x720/test_img.png");
}

static void return_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        media_lib_stop();
    }
}

static void back_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        if (media_lib_title_obj) {
            lv_obj_del(media_lib_title_obj);
            media_lib_title_obj = NULL;
        }
        if (media_lib_img_bg_obj) {
            lv_obj_del(media_lib_img_bg_obj);
            media_lib_img_bg_obj = NULL;
        }
    }
}

static void video_player_event_handler(lv_event_t * e)
{
    static int mic = 0;
    lv_event_code_t code = lv_event_get_code(e);
    char *val = (char *)lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        player_start(val, 0);
    }
}

static void photo_player_event_handler(lv_event_t * e)
{
    static int mic = 0;
    lv_event_code_t code = lv_event_get_code(e);
    char *val = (char *)lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        player_start(val, 1);
    }
}

static void media_lib_preview(int type)
{
    lv_obj_t *obj;
    lv_obj_t *obj_b;
    lv_color_t bg_color;
    lv_color_t text_color;
    int num = 80;
    int i;
    bg_color.full = 0xFF04171D;
    text_color.full = 0xffffffff;

    media_lib_title_obj = obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, LV_HOR_RES , 130);
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(obj, bg_color, 0);
    lv_obj_set_style_border_color(obj, bg_color, 0);
    lv_obj_set_style_radius(obj, 0, 0);

    media_lib_img_bg_obj = obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, LV_HOR_RES , LV_VER_RES - 130);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(obj, bg_color, 0);
    lv_obj_set_style_border_color(obj, bg_color, 0);
    lv_obj_set_style_radius(obj, 0, 0);

    obj = lv_imgbtn_create(media_lib_title_obj);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, icon_return_r, NULL);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, icon_return_p, NULL);
    lv_obj_add_event_cb(obj, back_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj, 60, 36);
    lv_obj_set_size(obj, 48 , 48);

    obj = lv_label_create(media_lib_title_obj);
    lv_obj_set_style_text_font(obj, ttf_info_32.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 38);
    if (type == 0)
        lv_label_set_text(obj, res_str[RES_STR_MEDIA_LIB_VIDEO_F]);
    else if (type == 1)
        lv_label_set_text(obj, res_str[RES_STR_MEDIA_LIB_VIDEO_P]);
    else if (type == 2)
        lv_label_set_text(obj, res_str[RES_STR_MEDIA_LIB_PHOTO]);
    else if (type == 3)
        lv_label_set_text(obj, res_str[RES_STR_MEDIA_LIB_VIDEO_U]);
    if (type == 2) {
        for (i = 0; i < num; i++) {
            char *file = "2021-09-03 11:20:50";
            int img_x = 40 + (i % 4) * 300;
            int img_y = 10 + (i / 4) * 260;
            int text_x = 52 + (i % 4) * 300;
            int text_y = 200 + (i / 4) * 260;
            lv_obj_t *obj = lv_imgbtn_create(media_lib_img_bg_obj);
            lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, test_img, NULL);
            lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, test_img, NULL);
            lv_obj_add_event_cb(obj, photo_player_event_handler, LV_EVENT_CLICKED, file);
            lv_obj_set_pos(obj, img_x, img_y);
            lv_obj_set_size(obj, 260, 180);

            obj = lv_label_create(media_lib_img_bg_obj);
            lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
            //text_color.full = 0xff313131;
            lv_obj_set_style_text_color(obj, text_color, 0);
            //lv_obj_align(obj, LV_ALIGN_CENTER, 0, 100);
            lv_obj_set_pos(obj, text_x, text_y);
            lv_label_set_text(obj, file);
        }
    } else {
        for (i = 0; i < num; i++) {
            char *file = "2021-09-03 11:20:50";
            int img_x = 40 + (i % 4) * 300;
            int img_y = 10 + (i / 4) * 260;
            int text_x = 52 + (i % 4) * 300;
            int text_y = 200 + (i / 4) * 260;
            lv_obj_t *obj = lv_img_create(media_lib_img_bg_obj);
            lv_img_set_src(obj, test_img);
            //lv_obj_align(obj, LV_ALIGN_CENTER, 0, -20);
            lv_obj_set_pos(obj, img_x, img_y);
            lv_obj_set_size(obj, 260, 180);
            obj = lv_imgbtn_create(obj);
            lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, list_icon_play, NULL);
            lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, list_icon_play, NULL);
            lv_obj_add_event_cb(obj, video_player_event_handler, LV_EVENT_CLICKED, file);
            lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_size(obj, 72, 72);

            obj = lv_label_create(media_lib_img_bg_obj);
            lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
            //text_color.full = 0xff313131;
            lv_obj_set_style_text_color(obj, text_color, 0);
            //lv_obj_align(obj, LV_ALIGN_CENTER, 0, 100);
            lv_obj_set_pos(obj, text_x, text_y);
            lv_label_set_text(obj, file);
        }
    }
}

static void meida_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int userdata = (int)lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked, userdata = %d\n", __func__, userdata);
        media_lib_preview(userdata);
    }
}

void media_lib_start()
{
    lv_obj_t *obj;
    lv_obj_t *obj_b;
    lv_color_t bg_color;
    lv_color_t text_color;
    bg_color.full = 0xFF04171D;
    text_color.full = 0xffffffff;
    media_lib_load_res();

    media_lib_main_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(media_lib_main_obj, LV_HOR_RES , LV_VER_RES);
    lv_obj_align(media_lib_main_obj, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(media_lib_main_obj, bg_color, 0);
    lv_obj_set_style_border_color(media_lib_main_obj, bg_color, 0);
    lv_obj_set_style_radius(media_lib_main_obj, 0, 0);

    obj = lv_imgbtn_create(media_lib_main_obj);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, icon_return_r, NULL);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, icon_return_p, NULL);
    lv_obj_add_event_cb(obj, return_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj, 60, 36);
    lv_obj_set_size(obj, 48 , 48);

    obj_b = lv_imgbtn_create(media_lib_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, icon_bg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, icon_bg_p, NULL);
    lv_obj_add_event_cb(obj_b, meida_event_handler, LV_EVENT_CLICKED, (void *)0);
    lv_obj_set_pos(obj_b, 30, 150);
    lv_obj_set_size(obj_b, 276 , 366);
    obj = lv_img_create(obj_b);
    lv_img_set_src(obj, icon_video_f_01);
    lv_obj_set_pos(obj, 76, 90);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 250);
    lv_label_set_text(obj, res_str[RES_STR_MEDIA_LIB_VIDEO_F]);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 290);
    lv_label_set_text(obj, "10");

    obj_b = lv_imgbtn_create(media_lib_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, icon_bg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, icon_bg_p, NULL);
    lv_obj_add_event_cb(obj_b, meida_event_handler, LV_EVENT_CLICKED, (void *)1);
    lv_obj_set_pos(obj_b, 330, 150);
    lv_obj_set_size(obj_b, 276 , 366);
    obj = lv_img_create(obj_b);
    lv_img_set_src(obj, icon_video_p_01);
    lv_obj_set_pos(obj, 76, 90);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 250);
    lv_label_set_text(obj, res_str[RES_STR_MEDIA_LIB_VIDEO_P]);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 290);
    lv_label_set_text(obj, "10");

    obj_b = lv_imgbtn_create(media_lib_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, icon_bg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, icon_bg_p, NULL);
    lv_obj_add_event_cb(obj_b, meida_event_handler, LV_EVENT_CLICKED, (void *)2);
    lv_obj_set_pos(obj_b, 630, 150);
    lv_obj_set_size(obj_b, 276 , 366);
    obj = lv_img_create(obj_b);
    lv_img_set_src(obj, icon_photo_01);
    lv_obj_set_pos(obj, 76, 90);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 250);
    lv_label_set_text(obj, res_str[RES_STR_MEDIA_LIB_PHOTO]);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 290);
    lv_label_set_text(obj, "50");

    obj_b = lv_imgbtn_create(media_lib_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, icon_bg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, icon_bg_p, NULL);
    lv_obj_add_event_cb(obj_b, meida_event_handler, LV_EVENT_CLICKED, (void *)3);
    lv_obj_set_pos(obj_b, 930, 150);
    lv_obj_set_size(obj_b, 276 , 366);
    obj = lv_img_create(obj_b);
    lv_img_set_src(obj, icon_video_u_01);
    lv_obj_set_pos(obj, 76, 90);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 250);
    lv_label_set_text(obj, res_str[RES_STR_MEDIA_LIB_VIDEO_U]);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 290);
    lv_label_set_text(obj, "5");
}

void media_lib_stop(void)
{
    if (media_lib_main_obj) {
        lv_obj_del(media_lib_main_obj);
        media_lib_main_obj = NULL;
    }
    media_lib_release_res();
}