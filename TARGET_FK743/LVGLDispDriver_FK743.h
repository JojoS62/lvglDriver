
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

#pragma once

#include "display_ltdc.h"
#include "LVGLDispDriverBase.h"

class LVGLDispFK743 : public LVGLDispDriver {
public:
    LVGLDispFK743(uint32_t resolutionX = LCD_Width, uint32_t resolutionY = LCD_Height);

private:
    DisplayLTDC display;

    void init();
    static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t *px_map);
    void flush(const lv_area_t * area, lv_color32_t * color_p);
    void flush_ready(int event_flags);
};
