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
#include "main_view.h"
#include "media_lib.h"
#include "settings.h"

static lv_img_dsc_t *index_bg = NULL;
static lv_img_dsc_t *index_icon_photo_p = NULL;
static lv_img_dsc_t *index_icon_photo_r = NULL;
static lv_img_dsc_t *index_icon_sw_p = NULL;
static lv_img_dsc_t *index_icon_sw_r = NULL;
static lv_img_dsc_t *index_icon_set_p = NULL;
static lv_img_dsc_t *index_icon_set_r = NULL;
static lv_img_dsc_t *index_icon_media_p = NULL;
static lv_img_dsc_t *index_icon_media_r = NULL;
static lv_img_dsc_t *index_icon_sd_nor = NULL;
static lv_img_dsc_t *index_icon_sd = NULL;
static lv_img_dsc_t *index_icon_record_nor = NULL;
static lv_img_dsc_t *index_icon_record = NULL;
static lv_img_dsc_t *index_icon_movie_p_nor = NULL;
static lv_img_dsc_t *index_icon_movie_f_nor = NULL;
static lv_img_dsc_t *index_icon_mic_nor = NULL;
static lv_img_dsc_t *index_icon_mic = NULL;

static lv_obj_t *main_view_obj = NULL;
static lv_obj_t *main_view_bg_obj = NULL;
static lv_obj_t *sd_obj = NULL;
static lv_obj_t *rec_obj = NULL;
static lv_obj_t *mic_obj = NULL;
static lv_obj_t *time_obj = NULL;
static lv_timer_t *timer = NULL;
static lv_color_t bg_color;
static lv_color_t text_color;

static void main_view_release_res(void)
{
    release_resource(&index_bg);
    release_resource(&index_icon_photo_p);
    release_resource(&index_icon_photo_r);
    release_resource(&index_icon_sw_p);
    release_resource(&index_icon_sw_r);
    release_resource(&index_icon_set_p);
    release_resource(&index_icon_set_r);
    release_resource(&index_icon_media_p);
    release_resource(&index_icon_media_r);
    release_resource(&index_icon_sd_nor);
    release_resource(&index_icon_sd);
    release_resource(&index_icon_record_nor);
    release_resource(&index_icon_record);
    release_resource(&index_icon_movie_p_nor);
    release_resource(&index_icon_movie_f_nor);
    release_resource(&index_icon_mic_nor);
    release_resource(&index_icon_mic);
}

static void main_view_load_res(void)
{
    main_view_release_res();

    index_bg = load_resource("/oem/cvr_res/ui_1280x720/index_bg.png");
    index_icon_photo_p = load_resource("/oem/cvr_res/ui_1280x720/index_icon_photo_p.png");
    index_icon_photo_r = load_resource("/oem/cvr_res/ui_1280x720/index_icon_photo_r.png");
    index_icon_sw_p = load_resource("/oem/cvr_res/ui_1280x720/index_icon_sw_p.png");
    index_icon_sw_r = load_resource("/oem/cvr_res/ui_1280x720/index_icon_sw_r.png");
    index_icon_set_p = load_resource("/oem/cvr_res/ui_1280x720/index_icon_set_p.png");
    index_icon_set_r = load_resource("/oem/cvr_res/ui_1280x720/index_icon_set_r.png");
    index_icon_media_p = load_resource("/oem/cvr_res/ui_1280x720/index_icon_media_p.png");
    index_icon_media_r = load_resource("/oem/cvr_res/ui_1280x720/index_icon_media_r.png");
    index_icon_sd_nor = load_resource("/oem/cvr_res/ui_1280x720/index_icon_sd_nor.png");
    index_icon_sd = load_resource("/oem/cvr_res/ui_1280x720/index_icon_sd.png");
    index_icon_record_nor = load_resource("/oem/cvr_res/ui_1280x720/index_icon_record_nor.png");
    index_icon_record = load_resource("/oem/cvr_res/ui_1280x720/index_icon_record.png");
    index_icon_movie_p_nor = load_resource("/oem/cvr_res/ui_1280x720/index_icon_movie_p_nor.png");
    index_icon_movie_f_nor = load_resource("/oem/cvr_res/ui_1280x720/index_icon_movie_f_nor.png");
    index_icon_mic_nor = load_resource("/oem/cvr_res/ui_1280x720/index_icon_mic_nor.png");
    index_icon_mic = load_resource("/oem/cvr_res/ui_1280x720/index_icon_mic.png");
}

static void sw_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
    }
}

static void photo_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
    }
}

static void media_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        media_lib_start();
    }
}

static void set_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        settings_start();
    }
}

static void update_status(void)
{
    static int sd_status = 0;
    static int rec_status = 0;
    static int mic_status = 0;

    char str_time[32];
    time_t timep;
    struct tm *p;
    time(&timep);
    p = gmtime(&timep);
    sprintf(str_time, "%4d-%02d-%02d %02d:%02d:%02d", 1900 + p->tm_year,
           1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    lv_label_set_text(time_obj, str_time);

    sd_status = !sd_status;
    if (sd_status)
        lv_img_set_src(sd_obj, index_icon_sd);
    else
        lv_img_set_src(sd_obj, index_icon_sd_nor);

    rec_status = !rec_status;
    if (rec_status)
        lv_img_set_src(rec_obj, index_icon_record);
    else
        lv_img_set_src(rec_obj, index_icon_record_nor);

    mic_status = !mic_status;
    if (mic_status)
        lv_img_set_src(mic_obj, index_icon_mic);
    else
        lv_img_set_src(mic_obj, index_icon_mic_nor);
}

static void time_handler(lv_timer_t *timer)
{
    update_status();
}

void main_view_start(void)
{
    lv_obj_t *obj;

    main_view_load_res();

	  bg_color.full = 0x00000000;
	  text_color.full = 0xffffffff;

    main_view_bg_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_view_bg_obj, LV_HOR_RES , LV_VER_RES);
    lv_obj_align(main_view_bg_obj, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(main_view_bg_obj, bg_color, 0);
    lv_obj_set_style_border_color(main_view_bg_obj, bg_color, 0);
    lv_obj_set_style_radius(main_view_bg_obj, 0, 0);

    main_view_obj = lv_img_create(lv_scr_act());
    lv_img_set_src(main_view_obj, index_bg);

    obj = lv_img_create(main_view_obj);
    lv_img_set_src(obj, index_icon_movie_f_nor);
    lv_obj_set_pos(obj, LV_HOR_RES - 300, 32);

    obj = lv_img_create(main_view_obj);
    lv_img_set_src(obj, index_icon_movie_p_nor);
    lv_obj_set_pos(obj, LV_HOR_RES - 240, 32);

    mic_obj = lv_img_create(main_view_obj);
    lv_img_set_src(mic_obj, index_icon_mic);
    lv_obj_set_pos(mic_obj, LV_HOR_RES - 180, 32);

    sd_obj = lv_img_create(main_view_obj);
    lv_img_set_src(sd_obj, index_icon_sd);
    lv_obj_set_pos(sd_obj, LV_HOR_RES - 120, 32);

    rec_obj = lv_img_create(main_view_obj);
    lv_img_set_src(rec_obj, index_icon_record);
    lv_obj_set_pos(rec_obj, LV_HOR_RES - 66, 40);

    obj = lv_imgbtn_create(main_view_obj);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, index_icon_sw_r, NULL);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, index_icon_sw_p, NULL);
    lv_obj_add_event_cb(obj, sw_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj, LV_HOR_RES - 800, LV_VER_RES - 148);
    lv_obj_set_size(obj, 96 , 96);

    obj = lv_imgbtn_create(main_view_obj);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, index_icon_photo_r, NULL);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, index_icon_photo_p, NULL);
    lv_obj_add_event_cb(obj, photo_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj, LV_HOR_RES - 676, LV_VER_RES - 164);
    lv_obj_set_size(obj, 128 , 128);

    obj = lv_imgbtn_create(main_view_obj);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, index_icon_media_r, NULL);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, index_icon_media_p, NULL);
    lv_obj_add_event_cb(obj, media_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj, LV_HOR_RES - 520, LV_VER_RES - 148);
    lv_obj_set_size(obj, 96 , 96);

    obj = lv_imgbtn_create(main_view_obj);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, index_icon_set_r, NULL);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, index_icon_set_p, NULL);
    lv_obj_add_event_cb(obj, set_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj, LV_HOR_RES - 380, LV_VER_RES - 148);
    lv_obj_set_size(obj, 96 , 96);

    time_obj = lv_label_create(main_view_obj);
    lv_obj_set_pos(time_obj, LV_HOR_RES - 260, LV_VER_RES - 116);
    //lv_label_set_recolor(time_obj, true);
    lv_obj_set_style_text_font(time_obj, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(time_obj, text_color, 0);

    update_status();

    timer = lv_timer_create(time_handler, 200, NULL);
}

void main_view_stop(void)
{
    if (timer)
        lv_timer_del(timer);
    timer = NULL;
    if (main_view_obj) {
        lv_obj_del(main_view_obj);
        main_view_obj = NULL;
    }
    if (main_view_bg_obj) {
        lv_obj_del(main_view_bg_obj);
        main_view_bg_obj = NULL;
    }

    main_view_release_res();
}
