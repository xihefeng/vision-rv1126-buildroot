/**
 * @file lv_port_indev_templ.c
 *
 */

 /* Copy this file as "lv_port_indev.c" and set this value to "1" to enable conten */

#include "lv_port_indev.h"
#include "lvgl.h"

#include "hal/evdev.h"

static int rot_indev;
lv_indev_t * indev_touchpad;

void lv_port_indev_init(int rot)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    static lv_indev_drv_t indev_drv;
    rot_indev = rot;

    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    evdev_init(rot);

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv);
}
