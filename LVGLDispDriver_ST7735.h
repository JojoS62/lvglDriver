
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

class LVGLDispST7735 : public LVGLDispDriver {
public:
    LVGLDispST7735(SPI &spi, PinName pinCS, PinName pinCMD, PinName pinRST, uint32_t nBufferRows = 40, uint32_t resolutionX = 160, uint32_t resolutionY = 128);

private:
    // SPI &_spi;
    // DigitalOut _cs;
    // DigitalOut _cmd;
    // DigitalOut _rst;
    // DigitalOut _backlight;
    SPI &_spi;               // does SPI MOSI, MISO and SCK
    DigitalOut _cs;         // does SPI CE
    DigitalOut _cmd;         // register/date select
    DigitalOut _rst;        // does 3310 LCD_RST
    uint8_t     colstart;
    uint8_t     rowstart; // some displays need this changed

    uint32_t _nBufferRows;
    void init();
    static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t *px_map);
    void flush(const lv_area_t * area, lv_color16_t * color_p);
    void flush_ready(int event_flags);

    void controllerInit();

    lv_color16_t *_buf1_1;     // display working buffer

    void command(uint8_t cmd);
    void data(uint8_t data);
    void hard_reset();
    void setRotation(uint8_t m);
    int  ST7735_init();
    void set_addr_win(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

    void commandList(const uint8_t *addr);
    void commonInit(const uint8_t *cmdList);
    void initB(void);
    void initS(void);
    void initR(uint8_t options);
};
