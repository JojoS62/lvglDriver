/* lvglDriver for Mbed
 * Copyright (c) 2019 Johannes Stratmann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "LVGLTouchDriver_FK743.h"


/*********************
 *      DEFINES
 *********************/

LVGLTouchDriverFK743::LVGLTouchDriverFK743(LVGLDispDriver *lvglDispDriver) :
    LVGLInputDriver(lvglDispDriver),
//    _touchDriver(PG_7, PG_3, PI_11, PI_10)  // FK743M5_XIH6
    _touchDriver(PI_8, PI_11, PG_3, PH_4)      // FK743M2_IIT6
{
    lv_indev_set_type(_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(_indev, read_cb);
    lv_indev_set_user_data(_indev, this);
}

void LVGLTouchDriverFK743::read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{

    // if (TS_State.touchDetected) {
    //     data->state = LV_INDEV_STATE_PR;
    // } else {
    //     data->state = LV_INDEV_STATE_REL;
    // }

    void* touchDriver = (void*)lv_indev_get_user_data(indev);
    ((LVGLTouchDriverFK743*)touchDriver)->_touchDriver.read(indev, data);
}

MBED_WEAK LVGLInputDriver *LVGLInputDriver::get_target_default_instance_touchdrv(LVGLDispDriver *disp)
{
    static LVGLTouchDriverFK743 lvglTouch(disp);
    return &lvglTouch;
}
