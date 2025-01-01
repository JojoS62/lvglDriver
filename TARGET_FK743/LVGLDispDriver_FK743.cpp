
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

#include "display_ltdc.h"
#include "LVGLDispDriver_FK743.h"


LVGLDispFK743::LVGLDispFK743(uint32_t resolutionX, uint32_t resolutionY) :
    LVGLDispDriver(resolutionX, resolutionY)
{
    // low level hardware init
    display.LCD_Init();

    // tft controller init
    init();
}

void LVGLDispFK743::init()
{
    size_t bufferSize = _horRes * _verRes * 4;

    // lv_display_set_flush_cb(_disp, disp_flush);
	_disp = lv_st_ltdc_create_direct((void*)LCD_MemoryAdd, (void*)(LCD_MemoryAdd + bufferSize), 0);
    // lv_display_set_buffers(_disp, (void*)LCD_MemoryAdd, (void*)(LCD_MemoryAdd + bufferSize), bufferSize, LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_user_data(_disp, this);
}

void LVGLDispFK743::disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LVGLDispFK743* instance = (LVGLDispFK743*)lv_display_get_user_data(disp);	

    instance->flush(area, (lv_color32_t*)px_map);
  	
	lv_display_flush_ready(disp);         /* Indicate you are ready with the flushing*/
 }

void LVGLDispFK743::flush(const lv_area_t *area, lv_color32_t *color_p)
{

    [[maybe_unused]] volatile int len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1) * 2; 	// in bytes
}

void LVGLDispFK743::flush_ready(int event_flags)
{
    if (event_flags & SPI_EVENT_COMPLETE) {
        lv_display_flush_ready(_disp);         /* Indicate you are ready with the flushing*/
    }
}

MBED_WEAK LVGLDispDriver *LVGLDispDriver::get_target_default_instance()
{
    static LVGLDispFK743 drv;
    return &drv;
}
