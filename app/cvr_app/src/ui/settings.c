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
#include "settings.h"
#include "lv_freetype.h"

extern lv_ft_info_t ttf_info_32;
extern lv_ft_info_t ttf_info_28;
extern lv_ft_info_t ttf_info_24;

#define maxlabelsize 35
char *res_str[RES_STR_MAX] = {0};

int movie = 0;
int res = 0;
int code_val = 0;
int vol = 0;
int mic = 0;
int lang = 0;

static lv_obj_t *setting_main_obj = NULL;
static lv_obj_t *movie_obj = NULL;
static lv_obj_t *res_obj = NULL;
static lv_obj_t *code_obj = NULL;
static lv_obj_t *vol_obj = NULL;
static lv_obj_t *mic_obj = NULL;
static lv_obj_t *lang_set_obj = NULL;
static lv_obj_t *store_state_obj = NULL;
static lv_obj_t *format_obj = NULL;

static lv_img_dsc_t *icon_return_r = NULL;
static lv_img_dsc_t *icon_return_p = NULL;
static lv_img_dsc_t *set_boxbg_r = NULL;
static lv_img_dsc_t *set_boxbg_p = NULL;
static lv_img_dsc_t *set_icon_movie_1 = NULL;
static lv_img_dsc_t *set_icon_movie_3 = NULL;
static lv_img_dsc_t *set_icon_movie_5 = NULL;
static lv_img_dsc_t *set_icon_res_4k = NULL;
static lv_img_dsc_t *set_icon_res_2k = NULL;
static lv_img_dsc_t *set_icon_res_1k = NULL;
static lv_img_dsc_t *set_icon_code_264 = NULL;
static lv_img_dsc_t *set_icon_code_265 = NULL;
static lv_img_dsc_t *set_icon_vol_01 = NULL;
static lv_img_dsc_t *set_icon_vol_02 = NULL;
static lv_img_dsc_t *set_icon_vol_03 = NULL;
static lv_img_dsc_t *set_icon_vol_04 = NULL;
static lv_img_dsc_t *set_icon_mic_01 = NULL;
static lv_img_dsc_t *set_icon_mic_02 = NULL;
static lv_img_dsc_t *set_icon_lang_01 = NULL;
static lv_img_dsc_t *set_icon_store_01 = NULL;
static lv_img_dsc_t *set_icon_format_01 = NULL;

static lv_img_dsc_t *set_btn_l_01 = NULL;
static lv_img_dsc_t *set_btn_l_02 = NULL;
static lv_img_dsc_t *set_btn_s_01 = NULL;
static lv_img_dsc_t *set_btn_s_02 = NULL;

lv_ft_info_t ttf_info_24;
lv_ft_info_t ttf_info_28;
lv_ft_info_t ttf_info_32;

void loading_font(void)
{
    /*Init freetype library
     *Cache max 64 faces and 1 size*/
    lv_freetype_init(64, 1, 0);

    /*Create a font*/
    ttf_info_24.name = "/oem/cvr_res/font/msyh.ttf";
    ttf_info_24.weight = 24;
    ttf_info_24.style = FT_FONT_STYLE_NORMAL;
    lv_ft_font_init(&ttf_info_24);

    ttf_info_28.name = "/oem/cvr_res/font/msyh.ttf";
    ttf_info_28.weight = 28;
    ttf_info_28.style = FT_FONT_STYLE_NORMAL;
    lv_ft_font_init(&ttf_info_28);

    ttf_info_32.name = "/oem/cvr_res/font/msyh.ttf";
    ttf_info_32.weight = 32;
    ttf_info_32.style = FT_FONT_STYLE_NORMAL;
    lv_ft_font_init(&ttf_info_32);
}

static void setting_release_res(void)
{
    release_resource(&icon_return_r);
    release_resource(&icon_return_p);
    release_resource(&set_boxbg_r);
    release_resource(&set_boxbg_p);
    release_resource(&set_icon_movie_1);
    release_resource(&set_icon_movie_3);
    release_resource(&set_icon_movie_5);
    release_resource(&set_icon_res_4k);
    release_resource(&set_icon_res_2k);
    release_resource(&set_icon_res_1k);
    release_resource(&set_icon_code_264);
    release_resource(&set_icon_code_265);
    release_resource(&set_icon_vol_01);
    release_resource(&set_icon_vol_02);
    release_resource(&set_icon_vol_03);
    release_resource(&set_icon_vol_04);
    release_resource(&set_icon_mic_01);
    release_resource(&set_icon_mic_02);
    release_resource(&set_icon_lang_01);
    release_resource(&set_icon_store_01);
    release_resource(&set_icon_format_01);
    release_resource(&set_btn_l_01);
    release_resource(&set_btn_l_02);
    release_resource(&set_btn_s_01);
    release_resource(&set_btn_s_02);
}

static void setting_load_res(void)
{
    setting_release_res();

    icon_return_r = load_resource("/oem/cvr_res/ui_1280x720/icon_return_01.png");
    icon_return_p = load_resource("/oem/cvr_res/ui_1280x720/icon_return_02.png");
    set_boxbg_r = load_resource("/oem/cvr_res/ui_1280x720/set_boxbg_01.png");
    set_boxbg_p = load_resource("/oem/cvr_res/ui_1280x720/set_boxbg_02.png");
    set_icon_movie_1 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_movie_01.png");
    set_icon_movie_3 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_movie_02.png");
    set_icon_movie_5 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_movie_03.png");
    set_icon_res_4k = load_resource("/oem/cvr_res/ui_1280x720/set_icon_res_01.png");
    set_icon_res_2k = load_resource("/oem/cvr_res/ui_1280x720/set_icon_res_02.png");
    set_icon_res_1k = load_resource("/oem/cvr_res/ui_1280x720/set_icon_res_03.png");
    set_icon_code_264 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_code_01.png");
    set_icon_code_265 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_code_02.png");
    set_icon_vol_01 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_vol_01.png");
    set_icon_vol_02 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_vol_02.png");
    set_icon_vol_03 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_vol_03.png");
    set_icon_vol_04 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_vol_04.png");
    set_icon_mic_01 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_mic_01.png");
    set_icon_mic_02 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_mic_02.png");
    set_icon_lang_01 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_lang_01.png");
    set_icon_store_01 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_store_01.png");
    set_icon_format_01 = load_resource("/oem/cvr_res/ui_1280x720/set_icon_format_01.png");
    set_btn_l_01 = load_resource("/oem/cvr_res/ui_1280x720/set_btn_l_01.png");
    set_btn_l_02 = load_resource("/oem/cvr_res/ui_1280x720/set_btn_l_02.png");
    set_btn_s_01 = load_resource("/oem/cvr_res/ui_1280x720/set_btn_s_01.png");
    set_btn_s_02 = load_resource("/oem/cvr_res/ui_1280x720/set_btn_s_02.png");
}

static void return_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        settings_stop();
    }
}

static void movie_update(void)
{
    if (movie == 0)
        lv_img_set_src(movie_obj, set_icon_movie_1);
    else if (movie == 1)
        lv_img_set_src(movie_obj, set_icon_movie_3);
    else if (movie == 2)
        lv_img_set_src(movie_obj, set_icon_movie_5);
}

static void res_update(void)
{
    if (res == 0)
        lv_img_set_src(res_obj, set_icon_res_4k);
    else if (res == 1)
        lv_img_set_src(res_obj, set_icon_res_2k);
    else if (res == 2)
        lv_img_set_src(res_obj, set_icon_res_1k);
}

static void code_update(void)
{
    if (code_val)
       lv_img_set_src(code_obj, set_icon_code_265);
    else
        lv_img_set_src(code_obj, set_icon_code_264);
}

static void vol_update(void)
{
    if (vol == 0)
        lv_img_set_src(vol_obj, set_icon_vol_01);
    else if (vol == 1)
        lv_img_set_src(vol_obj, set_icon_vol_02);
    else if (vol == 2)
        lv_img_set_src(vol_obj, set_icon_vol_03);
    else if (vol == 3)
        lv_img_set_src(vol_obj, set_icon_vol_04);
}

static void mic_update(void)
{
    if (mic)
        lv_img_set_src(mic_obj, set_icon_mic_02);
    else
        lv_img_set_src(mic_obj, set_icon_mic_01);
}

static void movie_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        movie++;
        if (movie > 2)
            movie = 0;
        movie_update();
    }
}

static void res_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
         res++;
        if (res > 2)
            res = 0;
        res_update();
    }
}

static void code_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        code_val = !code_val;
        code_update();
    }
}

static void vol_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        vol--;
        if (vol < 0)
            vol = 3;
        vol_update();
    }
}

static void mic_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        mic = !mic;
        mic_update();
    }
}

static void lang_sel_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int val = (int)lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        if (lang_set_obj) {
            lv_obj_del(lang_set_obj);
            lang_set_obj = NULL;
        }
        //set lang val
        lang = val;
        unloading_string_res();
        loading_string_res();
        settings_stop();
        settings_start();
    }
}

static void lang_close_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        if (lang_set_obj) {
            lv_obj_del(lang_set_obj);
            lang_set_obj = NULL;
        }
    }
}

static void lang_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        lv_obj_t *obj;
        lv_obj_t *obj_bg;
        lv_color_t color;
        lv_color_t text_color;
        text_color.full = 0xffffffff;
	      color.full = 0xFF000000;

        lang_set_obj = obj = lv_obj_create(lv_scr_act());
        lv_obj_set_size(obj, LV_HOR_RES , LV_VER_RES);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(obj, color, 0);
        lv_obj_set_style_border_color(obj, color, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);
        lv_obj_add_event_cb(obj, lang_close_event_handler, LV_EVENT_CLICKED, NULL);

        color.full = 0xFF020D10;
        obj_bg = obj = lv_obj_create(obj);
        lv_obj_set_size(obj, 500 , 540);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(obj, color, 0);
        lv_obj_set_style_border_color(obj, color, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_opa(obj, LV_OPA_80, 0);

        obj = lv_label_create(obj_bg);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 10);
        lv_label_set_text(obj, res_str[RES_STR_SETTINGS_BOX_LANG_TITLE]);

        text_color.full = 0xff4C5457;
        obj = lv_label_create(lang_set_obj);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 140);
        lv_label_set_text(obj, "-----------------------------------------");

        obj = lv_imgbtn_create(obj_bg);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, set_btn_l_01, NULL);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, set_btn_l_02, NULL);
        lv_obj_add_event_cb(obj, lang_sel_event_handler, LV_EVENT_CLICKED, (void *)0);
        lv_obj_set_size(obj, 368, 88);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, -100);
        obj = lv_label_create(obj);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        text_color.full = 0xff313131;
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(obj, res_str[RES_STR_SETTINGS_BOX_LANG_SN]);

        obj = lv_imgbtn_create(obj_bg);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, set_btn_l_01, NULL);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, set_btn_l_02, NULL);
        lv_obj_add_event_cb(obj, lang_sel_event_handler, LV_EVENT_CLICKED, (void *)1);
        lv_obj_set_size(obj, 368, 88);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        obj = lv_label_create(obj);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        text_color.full = 0xff313131;
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(obj, res_str[RES_STR_SETTINGS_BOX_LANG_TN]);

        obj = lv_imgbtn_create(obj_bg);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, set_btn_l_01, NULL);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, set_btn_l_02, NULL);
        lv_obj_add_event_cb(obj, lang_sel_event_handler, LV_EVENT_CLICKED, (void *)2);
        lv_obj_set_size(obj, 368, 88);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 100);
        obj = lv_label_create(obj);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        text_color.full = 0xff313131;
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(obj, res_str[RES_STR_SETTINGS_BOX_LANG_EN]);

        obj = lv_imgbtn_create(obj_bg);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, set_btn_l_01, NULL);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, set_btn_l_02, NULL);
        lv_obj_add_event_cb(obj, lang_sel_event_handler, LV_EVENT_CLICKED, (void *)3);
        lv_obj_set_size(obj, 368, 88);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 200);
        obj = lv_label_create(obj);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        text_color.full = 0xff313131;
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(obj, res_str[RES_STR_SETTINGS_BOX_LANG_JA]);
    }
}

static void store_close_event_handler(lv_event_t * e)
{
    static int mic = 0;
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        if (store_state_obj) {
            lv_obj_del(store_state_obj);
            store_state_obj = NULL;
        }
    }
}

static void store_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        char info[32];
        int use = 10;
        int total = 64;
        lv_obj_t *obj;
        lv_obj_t *obj_bg;
        lv_color_t color;
        lv_color_t text_color;
        text_color.full = 0xffffffff;
	      color.full = 0xFF000000;

        store_state_obj = obj = lv_obj_create(lv_scr_act());
        lv_obj_set_size(obj, LV_HOR_RES , LV_VER_RES);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(obj, color, 0);
        lv_obj_set_style_border_color(obj, color, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);
        lv_obj_add_event_cb(obj, store_close_event_handler, LV_EVENT_CLICKED, NULL);

        color.full = 0xFF020D10;
        obj_bg = obj = lv_obj_create(obj);
        lv_obj_set_size(obj, 600 , 400);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(obj, color, 0);
        lv_obj_set_style_border_color(obj, color, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_opa(obj, LV_OPA_80, 0);
        lv_obj_add_event_cb(obj, store_close_event_handler, LV_EVENT_CLICKED, NULL);

        obj = lv_label_create(obj_bg);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 10);
        lv_label_set_text(obj, res_str[RES_STR_SETTINGS_BOX_STORE_TITLE]);

        text_color.full = 0xff4C5457;
        obj = lv_label_create(store_state_obj);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 210);
        lv_label_set_text(obj, "-------------------------------------------------");

        sprintf(info, "%s%dGB/%dGB", res_str[RES_STR_SETTINGS_BOX_STORE_INFO], use, total);
        text_color.full = 0xffffffff;
        obj = lv_label_create(obj_bg);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_RIGHT_MID, -10, -20);
        lv_label_set_text(obj, info);

        obj = lv_bar_create(obj_bg);
        lv_obj_set_size(obj, 540, 20);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 40);
        lv_bar_set_value(obj, 70, LV_ANIM_OFF);
    }
}

static void format_close_event_handler(lv_event_t * e)
{
    static int mic = 0;
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        if (format_obj) {
            lv_obj_del(format_obj);
            format_obj = NULL;
        }
    }
}

static void auto_del(lv_obj_t * obj, uint32_t delay)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_time(&a, 0);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_ready_cb(&a, lv_obj_del_anim_ready_cb);
    lv_anim_start(&a);
}

static void format_yes_event_handler(lv_event_t * e)
{
    static int mic = 0;
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        lv_obj_t *obj;
        lv_color_t color;
	      color.full = 0xFF000000;
        printf("%s Clicked\n", __func__);
        if (format_obj) {
            lv_obj_del(format_obj);
            format_obj = NULL;
        }

        format_obj = obj = lv_obj_create(lv_scr_act());
        lv_obj_set_size(obj, LV_HOR_RES , LV_VER_RES);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(obj, color, 0);
        lv_obj_set_style_border_color(obj, color, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);

        obj = lv_spinner_create(obj, 1000, 60);
        lv_obj_set_size(obj, 100, 100);
        lv_obj_center(obj);

        auto_del(format_obj, 5000);
        //call format api
    }
}

static void format_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        lv_obj_t *obj;
        lv_obj_t *obj_bg;
        lv_color_t color;
        lv_color_t text_color;
        text_color.full = 0xffffffff;
	      color.full = 0xFF000000;

        format_obj = obj = lv_obj_create(lv_scr_act());
        lv_obj_set_size(obj, LV_HOR_RES , LV_VER_RES);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(obj, color, 0);
        lv_obj_set_style_border_color(obj, color, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);
        lv_obj_add_event_cb(obj, format_close_event_handler, LV_EVENT_CLICKED, NULL);

        color.full = 0xFF020D10;
        obj_bg = obj = lv_obj_create(obj);
        lv_obj_set_size(obj, 600 , 400);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(obj, color, 0);
        lv_obj_set_style_border_color(obj, color, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_opa(obj, LV_OPA_80, 0);

        obj = lv_label_create(obj_bg);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 10);
        lv_label_set_text(obj, res_str[RES_STR_SETTINGS_BOX_FORMAT_TITLE]);

        text_color.full = 0xff4C5457;
        obj = lv_label_create(format_obj);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 210);
        lv_label_set_text(obj, "-------------------------------------------------");

        text_color.full = 0xffffffff;
        obj = lv_label_create(obj_bg);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, -20);
        lv_label_set_text(obj, res_str[RES_STR_SETTINGS_BOX_FORMAT_INFO]);

        obj = lv_imgbtn_create(obj_bg);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, set_btn_s_01, NULL);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, set_btn_s_02, NULL);
        lv_obj_add_event_cb(obj, format_yes_event_handler, LV_EVENT_CLICKED, (void *)0);
        lv_obj_set_size(obj, 246, 88);
        lv_obj_align(obj, LV_ALIGN_LEFT_MID, 10, 100);
        obj = lv_label_create(obj);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        text_color.full = 0xff313131;
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(obj, res_str[RES_STR_SETTINGS_BOX_FORMAT_OK]);

        obj = lv_imgbtn_create(obj_bg);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, set_btn_s_01, NULL);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, set_btn_s_02, NULL);
        lv_obj_add_event_cb(obj, format_close_event_handler, LV_EVENT_CLICKED, (void *)0);
        lv_obj_set_size(obj, 246, 88);
        lv_obj_align(obj, LV_ALIGN_RIGHT_MID, -10, 100);
        obj = lv_label_create(obj);
        lv_obj_set_style_text_font(obj, ttf_info_28.font, 0);
        text_color.full = 0xff313131;
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(obj, res_str[RES_STR_SETTINGS_BOX_FORMAT_CANCEL]);
    }
}


static int get_string(char **lineptr, ssize_t *n, FILE *stream, int lang)
{
    int count = 0;
    int buf;
    int table_cnt = 0;

    if (*lineptr == NULL) {
        *n = maxlabelsize;
        *lineptr = (char *)malloc(*n);
        memset(*lineptr, 0, *n);
    }

    if ((buf = fgetc(stream)) == EOF) {
        return -1;
    }

    do {
        if (buf == '\n') {
            count += 1;
            break;
        }
        if (buf == '\t') {
            table_cnt++;
        } else if (table_cnt == lang) {
            count++;

            *(*lineptr + count - 1) = buf;
            *(*lineptr + count) = '\0';

            if (*n <= count)
                *lineptr = (char *)realloc(*lineptr, count * 2);
        }
        buf = fgetc(stream);
    } while (buf != EOF);

    return count;
}

int loading_string_res(void)
{
    FILE *fp;
    ssize_t len = 0;
    int pos = 0;
    const char *langFile = "/oem/cvr_res/string/string.txt";

    fp = fopen(langFile, "r");
    if (fp == NULL)
    {
        printf("open file %s\n", langFile);
        return -1;
    }

    fgetc(fp);
    fgetc(fp);
    fgetc(fp);

    while ((get_string(&res_str[pos], &len, fp, lang + 1)) != -1) {
        //printf("load line Label %d------%s\n", pos, res_str[pos]);
        pos++;
        if (pos >= RES_STR_MAX)
            break;
    }

    fclose(fp);
    //updatesysfont(logfont);
}

void unloading_string_res(void)
{
    int i;

    for (i = 0; i < RES_STR_MAX; i++) {
        if (res_str[i]) {
            free(res_str[i]);
            res_str[i] = 0;
        }
    }
}

void settings_start(void)
{
    lv_obj_t *obj;
    lv_obj_t *obj_b;
    lv_color_t bg_color;
    lv_color_t text_color;
    bg_color.full = 0xFF04171D;
    text_color.full = 0xffffffff;
    setting_load_res();

    setting_main_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(setting_main_obj, LV_HOR_RES , LV_VER_RES);
    lv_obj_align(setting_main_obj, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(setting_main_obj, bg_color, 0);
    lv_obj_set_style_border_color(setting_main_obj, bg_color, 0);
    lv_obj_set_style_radius(setting_main_obj, 0, 0);

    obj = lv_imgbtn_create(setting_main_obj);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, icon_return_r, NULL);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, icon_return_p, NULL);
    lv_obj_add_event_cb(obj, return_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj, 60, 36);
    lv_obj_set_size(obj, 48 , 48);

    obj = lv_label_create(setting_main_obj);
    lv_obj_set_style_text_font(obj, ttf_info_32.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 38);
    lv_label_set_text(obj, res_str[RES_STR_SETTINGS_SET]);

    obj_b = lv_imgbtn_create(setting_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, set_boxbg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, set_boxbg_p, NULL);
    lv_obj_add_event_cb(obj_b, movie_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj_b, 40, 150);
    lv_obj_set_size(obj_b, 290 , 210);
    movie_obj = obj = lv_img_create(obj_b);
    //lv_img_set_src(obj, set_icon_movie_1);
    movie_update();
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 30);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 140);
    lv_label_set_text(obj, res_str[RES_STR_SETTINGS_MOVIE]);

    obj_b = lv_imgbtn_create(setting_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, set_boxbg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, set_boxbg_p, NULL);
    lv_obj_add_event_cb(obj_b, res_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj_b, 331, 150);
    lv_obj_set_size(obj_b, 290 , 210);
    res_obj = obj = lv_img_create(obj_b);
    //lv_img_set_src(obj, set_icon_res_4k);
    res_update();
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 30);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 140);
    lv_label_set_text(obj, res_str[RES_STR_SETTINGS_RES]);

    obj_b = lv_imgbtn_create(setting_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, set_boxbg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, set_boxbg_p, NULL);
    lv_obj_add_event_cb(obj_b, code_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj_b, 622, 150);
    lv_obj_set_size(obj_b, 290 , 210);
    code_obj = obj = lv_img_create(obj_b);
    //lv_img_set_src(obj, set_icon_code_264);
    code_update();
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 30);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 140);
    lv_label_set_text(obj, res_str[RES_STR_SETTINGS_CODE]);

    obj_b = lv_imgbtn_create(setting_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, set_boxbg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, set_boxbg_p, NULL);
    lv_obj_add_event_cb(obj_b, vol_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj_b, 913, 150);
    lv_obj_set_size(obj_b, 290 , 210);
    vol_obj = obj = lv_img_create(obj_b);
    //lv_img_set_src(obj, set_icon_vol_01);
    vol_update();
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 30);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 140);
    lv_label_set_text(obj, res_str[RES_STR_SETTINGS_VOL]);

    obj_b = lv_imgbtn_create(setting_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, set_boxbg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, set_boxbg_p, NULL);
    lv_obj_add_event_cb(obj_b, mic_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj_b, 40, 361);
    lv_obj_set_size(obj_b, 290 , 210);
    mic_obj = obj = lv_img_create(obj_b);
    //lv_img_set_src(obj, set_icon_mic_01);
    mic_update();
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 30);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 140);
    lv_label_set_text(obj, res_str[RES_STR_SETTINGS_MIC]);

    obj_b = lv_imgbtn_create(setting_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, set_boxbg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, set_boxbg_p, NULL);
    lv_obj_add_event_cb(obj_b, lang_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj_b, 331, 361);
    lv_obj_set_size(obj_b, 290 , 210);
    obj = lv_img_create(obj_b);
    lv_img_set_src(obj, set_icon_lang_01);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 30);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 140);
    lv_label_set_text(obj, res_str[RES_STR_SETTINGS_LANG]);

    obj_b = lv_imgbtn_create(setting_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, set_boxbg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, set_boxbg_p, NULL);
    lv_obj_add_event_cb(obj_b, store_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj_b, 622, 361);
    lv_obj_set_size(obj_b, 290 , 210);
    obj = lv_img_create(obj_b);
    lv_img_set_src(obj, set_icon_store_01);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 30);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 140);
    lv_label_set_text(obj, res_str[RES_STR_SETTINGS_STORE]);

    obj_b = lv_imgbtn_create(setting_main_obj);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_RELEASED, NULL, set_boxbg_r, NULL);
    lv_imgbtn_set_src(obj_b, LV_IMGBTN_STATE_PRESSED, NULL, set_boxbg_p, NULL);
    lv_obj_add_event_cb(obj_b, format_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj_b, 913, 361);
    lv_obj_set_size(obj_b, 290 , 210);
    obj = lv_img_create(obj_b);
    lv_img_set_src(obj, set_icon_format_01);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 30);
    obj = lv_label_create(obj_b);
    lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 140);
    lv_label_set_text(obj, res_str[RES_STR_SETTINGS_FORMAT]);
}

void settings_stop(void)
{
    if (setting_main_obj) {
        lv_obj_del(setting_main_obj);
        setting_main_obj = NULL;
    }
    setting_load_res();
}