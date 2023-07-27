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
#include "lv_freetype.h"
#include "player.h"

extern lv_ft_info_t ttf_info_32;
extern lv_ft_info_t ttf_info_28;
extern lv_ft_info_t ttf_info_24;

static lv_timer_t *timer = NULL;

static lv_img_dsc_t *index_bg = NULL;
static lv_img_dsc_t *icon_return_r = NULL;
static lv_img_dsc_t *icon_return_p = NULL;
static lv_img_dsc_t *img_icon_next = NULL;
static lv_img_dsc_t *img_icon_pre = NULL;
static lv_img_dsc_t *player_icon_play = NULL;
static lv_img_dsc_t *player_icon_pre = NULL;
static lv_img_dsc_t *player_icon_next = NULL;


static lv_obj_t *player_main_obj = NULL;
static lv_obj_t *player_main_bg_obj = NULL;
static lv_obj_t *cur_time_obj = NULL;
static lv_obj_t *total_time_obj = NULL;
static lv_obj_t *slider_obj = NULL;

static int cur_time = 0;
static int total_time = 60;

static void player_release_res(void)
{
    release_resource(&index_bg);
    release_resource(&icon_return_r);
    release_resource(&icon_return_p);
    release_resource(&img_icon_next);
    release_resource(&img_icon_pre);
    release_resource(&player_icon_play);
    release_resource(&player_icon_pre);
    release_resource(&player_icon_next);
}

static void player_load_res(void)
{
    player_release_res();

    index_bg = load_resource("/oem/cvr_res/ui_1280x720/index_bg.png");
    icon_return_r = load_resource("/oem/cvr_res/ui_1280x720/icon_return_01.png");
    icon_return_p = load_resource("/oem/cvr_res/ui_1280x720/icon_return_02.png");
    img_icon_next = load_resource("/oem/cvr_res/ui_1280x720/img_icon_next.png");
    img_icon_pre = load_resource("/oem/cvr_res/ui_1280x720/img_icon_pre.png");
    player_icon_play = load_resource("/oem/cvr_res/ui_1280x720/player_icon_play.png");
    player_icon_pre = load_resource("/oem/cvr_res/ui_1280x720/player_icon_pre.png");
    player_icon_next = load_resource("/oem/cvr_res/ui_1280x720/player_icon_next.png");
}

static void return_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        printf("%s Clicked\n", __func__);
        player_stop();
    }
}

static void update_time(void)
{
    char str_time[32];

    lv_slider_set_value(slider_obj, cur_time, LV_ANIM_OFF);
    sprintf(str_time, "%02d:%02d", cur_time / 60, cur_time % 60);
    lv_label_set_text(cur_time_obj, str_time);
    sprintf(str_time, "%02d:%02d", total_time / 60, total_time % 60);
    lv_label_set_text(total_time_obj, str_time);
}

static void time_handler(lv_timer_t *timer)
{
    cur_time++;
    if (cur_time > total_time)
        cur_time = 0;
    update_time();
}

void player_start(char *filepath, int type)
{
    printf("%s %s\n", __func__, filepath);
    lv_obj_t *obj;
    lv_color_t bg_color;
    lv_color_t text_color;
    bg_color.full = 0x00000000;
    text_color.full = 0xffffffff;

    player_load_res();

    player_main_obj = obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, LV_HOR_RES , LV_VER_RES);
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(obj, bg_color, 0);
    lv_obj_set_style_border_color(obj, bg_color, 0);
    lv_obj_set_style_radius(obj, 0, 0);

    player_main_bg_obj = obj = lv_img_create(lv_scr_act());
    lv_img_set_src(obj, index_bg);

    obj = lv_imgbtn_create(player_main_obj);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, icon_return_r, NULL);
    lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, icon_return_p, NULL);
    lv_obj_add_event_cb(obj, return_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_pos(obj, 60, 36);
    lv_obj_set_size(obj, 48, 48);

    obj = lv_label_create(player_main_obj);
    lv_obj_set_style_text_font(obj, ttf_info_32.font, 0);
    lv_obj_set_style_text_color(obj, text_color, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 38);
    lv_label_set_text(obj, filepath);
    if (type == 0) {
        obj = lv_imgbtn_create(player_main_bg_obj);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, player_icon_play, NULL);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, player_icon_play, NULL);
        //lv_obj_add_event_cb(obj, return_event_handler, LV_EVENT_CLICKED, NULL);
        lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_size(obj, 128, 128);
        obj = lv_imgbtn_create(player_main_bg_obj);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, player_icon_pre, NULL);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, player_icon_pre, NULL);
        //lv_obj_add_event_cb(obj, return_event_handler, LV_EVENT_CLICKED, NULL);
        lv_obj_align(obj, LV_ALIGN_CENTER, -200, 0);
        lv_obj_set_size(obj, 72, 72);
        obj = lv_imgbtn_create(player_main_bg_obj);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, player_icon_next, NULL);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, player_icon_next, NULL);
        //lv_obj_add_event_cb(obj, return_event_handler, LV_EVENT_CLICKED, NULL);
        lv_obj_align(obj, LV_ALIGN_CENTER, 200, 0);
        lv_obj_set_size(obj, 72, 72);

        cur_time_obj = obj = lv_label_create(player_main_bg_obj);
        lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_BOTTOM_LEFT, 16, -30);
        //lv_label_set_text(obj, "00:00");

        total_time_obj = obj = lv_label_create(player_main_bg_obj);
        lv_obj_set_style_text_font(obj, ttf_info_24.font, 0);
        lv_obj_set_style_text_color(obj, text_color, 0);
        lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, -16, -30);
        //lv_label_set_text(obj, "01:00");

        /* Create a transition */
        static const lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, 0};
        static lv_style_transition_dsc_t transition_dsc;
        lv_style_transition_dsc_init(&transition_dsc, props, lv_anim_path_linear, 300, 0, NULL);

        static lv_style_t style_main;
        static lv_style_t style_indicator;
        static lv_style_t style_knob;
        static lv_style_t style_pressed_color;
        lv_style_init(&style_main);
        lv_style_set_bg_opa(&style_main, LV_OPA_50);
        lv_style_set_bg_color(&style_main, lv_color_hex(0x4c5457));
        lv_style_set_radius(&style_main, LV_RADIUS_CIRCLE);
        lv_style_set_pad_ver(&style_main, -2); /*Makes the indicator larger*/

        lv_style_init(&style_indicator);
        lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);
        lv_style_set_bg_color(&style_indicator, lv_color_hex(0x00eaeb));
        lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);
        lv_style_set_transition(&style_indicator, &transition_dsc);

        lv_style_init(&style_knob);
        lv_style_set_bg_opa(&style_knob, LV_OPA_COVER);
        lv_style_set_bg_color(&style_knob, lv_color_hex(0xffffff));
        lv_style_set_border_color(&style_knob, lv_color_hex(0xffffff));
        lv_style_set_border_width(&style_knob, 10);
        lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);
        lv_style_set_pad_all(&style_knob, 6); /*Makes the knob larger*/
        lv_style_set_transition(&style_knob, &transition_dsc);

        lv_style_init(&style_pressed_color);
        lv_style_set_bg_color(&style_pressed_color, lv_color_hex(0x00eaeb));

        /* Create a slider and add the style */
        slider_obj = lv_slider_create(player_main_bg_obj);
        lv_slider_set_range(slider_obj, 0, total_time);

        lv_obj_remove_style_all(slider_obj);        /*Remove the styles coming from the theme*/

        lv_obj_add_style(slider_obj, &style_main, LV_PART_MAIN);
        lv_obj_add_style(slider_obj, &style_indicator, LV_PART_INDICATOR);
        lv_obj_add_style(slider_obj, &style_pressed_color, LV_PART_INDICATOR | LV_STATE_PRESSED);
        lv_obj_add_style(slider_obj, &style_knob, LV_PART_KNOB);
        lv_obj_add_style(slider_obj, &style_pressed_color, LV_PART_KNOB | LV_STATE_PRESSED);
        lv_obj_set_size(slider_obj, LV_HOR_RES - 200, 10);
        lv_obj_align(slider_obj, LV_ALIGN_BOTTOM_MID, 0, -40);

        update_time();
        timer = lv_timer_create(time_handler, 1000, NULL);
    } else {
        obj = lv_imgbtn_create(player_main_bg_obj);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, img_icon_pre, NULL);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, img_icon_pre, NULL);
        //lv_obj_add_event_cb(obj, return_event_handler, LV_EVENT_CLICKED, NULL);
        lv_obj_align(obj, LV_ALIGN_LEFT_MID, 50, 0);
        lv_obj_set_size(obj, 72, 72);
        obj = lv_imgbtn_create(player_main_bg_obj);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, NULL, img_icon_next, NULL);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_PRESSED, NULL, img_icon_next, NULL);
        //lv_obj_add_event_cb(obj, return_event_handler, LV_EVENT_CLICKED, NULL);
        lv_obj_align(obj, LV_ALIGN_RIGHT_MID, -50, 0);
        lv_obj_set_size(obj, 72, 72);
    }
}

void player_stop(void)
{
    if (timer)
        lv_timer_del(timer);
    timer = NULL;
    if (player_main_bg_obj) {
        lv_obj_del(player_main_bg_obj);
        player_main_bg_obj = NULL;
    }
    if (player_main_obj) {
        lv_obj_del(player_main_obj);
        player_main_obj = NULL;
    }
    player_release_res();
}