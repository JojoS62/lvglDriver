
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

#include "LVGLDispDriverBase.h"

class LVGLDispGC9A01 : public LVGLDispDriver {
public:
    LVGLDispGC9A01(SPI &spi, PinName pinCS, PinName pinCMD, PinName pinRST, PinName pinBacklight, uint32_t nBufferRows = 20, uint32_t resolutionX = 240, uint32_t resolutionY = 240);

private:
    SPI &_spi;
    DigitalOut _cs;
    DigitalOut _cmd;
    DigitalOut _rst;
    DigitalOut _backlight;

    uint32_t _nBufferRows;
    void init();
    static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t *px_map);
    void flush(const lv_area_t * area, lv_color16_t * color_p);
    void flush_ready(int event_flags);

    void controllerInit();

    lv_color16_t *_buf1_1;     // display working buffer

    void GC9A01_command(uint8_t cmd);
    void GC9A01_data(uint8_t data);
    void GC9A01_hard_reset();
    void GC9A01_run_cfg_script();
    void GC9A01_setRotation(uint8_t m);
    int GC9A01_init();
    void GC9A01_set_addr_win(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);


};
