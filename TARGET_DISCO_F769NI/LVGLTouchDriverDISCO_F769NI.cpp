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

#include "LVGLTouchDriverDISCO_F769NI.h"

#include "TS_DISCO_F769NI.h"

TS_DISCO_F769NI ts;

/*********************
 *      DEFINES
 *********************/

LVGLTouchDriverDISCO_F769NI::LVGLTouchDriverDISCO_F769NI(LVGLDispDriver *lvglDispDriver) :
    LVGLInputDriver(lvglDispDriver)
{
    ts.Init(800, 472);

    lv_indev_set_type(_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(_indev, read_cb);
    lv_indev_set_user_data(_indev, this);

}

void LVGLTouchDriverDISCO_F769NI::read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
    TS_StateTypeDef TS_State;

    ts.GetState(&TS_State);

    if (TS_State.touchDetected) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = TS_State.touchX[0];
        data->point.y = TS_State.touchY[0];
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

}

MBED_WEAK LVGLInputDriver *LVGLInputDriver::get_target_default_instance_touchdrv(LVGLDispDriver *disp)
{
    static LVGLTouchDriverDISCO_F769NI lvglTouch(disp);
    return &lvglTouch;
}
