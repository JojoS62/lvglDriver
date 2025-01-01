
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

#include "LVGLDispDriver_GC9A01.h"

/*********************
 *      DEFINES
 *********************/
#define GC9A01_CMD_MODE     0
#define GC9A01_DATA_MODE    1

/* GC9A01 Commands that we know of.  Limited documentation */
#define GC9A01_INVOFF	0x20
#define GC9A01_INVON	0x21
#define GC9A01_DISPON	0x29
#define GC9A01_CASET	0x2A
#define GC9A01_RASET	0x2B
#define GC9A01_RAMWR	0x2C
#define GC9A01_COLMOD	0x3A
#define GC9A01_MADCTL	0x36
#define GC9A01_MADCTL_MY  0x80
#define GC9A01_MADCTL_MX  0x40
#define GC9A01_MADCTL_MV  0x20
#define GC9A01_MADCTL_RGB 0x08
#define GC9A01_DISFNCTRL	0xB6

/* Init script function */
struct GC9A01_function {
	uint16_t cmd;
	uint16_t data;
};

/* Init script commands */
enum GC9A01_cmd {
	GC9A01_START,
	GC9A01_END,
	GC9A01_CMD,
	GC9A01_DATA,
	GC9A01_DELAY
};

/**********************
 *  STATIC VARIABLES
 **********************/
// Documentation on op codes for GC9A01 are very hard to find.
// Will document should they be found.
static struct GC9A01_function GC9A01_cfg_table[] = {
	{ GC9A01_START, GC9A01_START},
	{ GC9A01_CMD, 0xEF},

	{ GC9A01_CMD, 0xEB},
	{ GC9A01_DATA, 0x14},

	{ GC9A01_CMD, 0xFE}, // Inter Register Enable1
	{ GC9A01_CMD, 0xEF}, // Inter Register Enable2

	{ GC9A01_CMD, 0xEB},
	{ GC9A01_DATA, 0x14},

	{ GC9A01_CMD, 0x84},
	{ GC9A01_DATA, 0x40},

	{ GC9A01_CMD, 0x85},
	{ GC9A01_DATA, 0xFF},

	{ GC9A01_CMD, 0x86},
	{ GC9A01_DATA, 0xFF},

	{ GC9A01_CMD, 0x87},
	{ GC9A01_DATA, 0xFF},

	{ GC9A01_CMD, 0x88},
	{ GC9A01_DATA, 0x0A},

	{ GC9A01_CMD, 0x89},
	{ GC9A01_DATA, 0x21},

	{ GC9A01_CMD, 0x8A},
	{ GC9A01_DATA, 0x00},

	{ GC9A01_CMD, 0x8B},
	{ GC9A01_DATA, 0x80},

	{ GC9A01_CMD, 0x8C},
	{ GC9A01_DATA, 0x01},

	{ GC9A01_CMD, 0x8D},
	{ GC9A01_DATA, 0x01},

	{ GC9A01_CMD, 0x8E},
	{ GC9A01_DATA, 0xFF},

	{ GC9A01_CMD, 0x8F},
	{ GC9A01_DATA, 0xFF},

	{ GC9A01_CMD, GC9A01_DISFNCTRL}, 	// Display Function Control
	{ GC9A01_DATA, 0x00},
	{ GC9A01_DATA, 0x00},

	{ GC9A01_CMD, GC9A01_COLMOD}, 		// COLMOD: Pixel Format Set
	{ GC9A01_DATA, 0x05}, 				// 16 Bits per pixel

	{ GC9A01_CMD, GC9A01_MADCTL }, 		// Memory Access Control
	{ GC9A01_DATA, GC9A01_MADCTL_MX | GC9A01_MADCTL_RGB }, 	// Set the display direction 0,1,2,3	four directions
//	{ GC9A01_DATA, GC9A01_MADCTL_MX }, 	// Set the display direction 0,1,2,3	four directions

	{ GC9A01_CMD, 0x90},
	{ GC9A01_DATA, 0x08},
	{ GC9A01_DATA, 0x08},
	{ GC9A01_DATA, 0x08},
	{ GC9A01_DATA, 0x08},

	{ GC9A01_CMD, 0xBD},
	{ GC9A01_DATA, 0x06},

	{ GC9A01_CMD, 0xBC},
	{ GC9A01_DATA, 0x00},

	{ GC9A01_CMD, 0xFF},
	{ GC9A01_DATA, 0x60},
	{ GC9A01_DATA, 0x01},
	{ GC9A01_DATA, 0x04},

	{ GC9A01_CMD, 0xC3}, // Power Control 2
	{ GC9A01_DATA, 0x13},
	{ GC9A01_CMD, 0xC4}, // Power Control 3
	{ GC9A01_DATA, 0x13},

	{ GC9A01_CMD, 0xC9}, // Power Control 4
	{ GC9A01_DATA, 0x22},

	{ GC9A01_CMD, 0xBE}, 
	{ GC9A01_DATA, 0x11},

	{ GC9A01_CMD, 0xE1}, 
	{ GC9A01_DATA, 0x10},
	{ GC9A01_DATA, 0x0E},

	{ GC9A01_CMD, 0xDF},
	{ GC9A01_DATA, 0x21},
	{ GC9A01_DATA, 0x0C},
	{ GC9A01_DATA, 0x02},

	{ GC9A01_CMD, 0xF0}, // SET_GAMMA1
	{ GC9A01_DATA, 0x45},
	{ GC9A01_DATA, 0x09},
	{ GC9A01_DATA, 0x08},
	{ GC9A01_DATA, 0x08},
	{ GC9A01_DATA, 0x26},
	{ GC9A01_DATA, 0x2A},

	{ GC9A01_CMD, 0xF1}, // SET_GAMMA2
	{ GC9A01_DATA, 0x43},
	{ GC9A01_DATA, 0x70},
	{ GC9A01_DATA, 0x72},
	{ GC9A01_DATA, 0x36},
	{ GC9A01_DATA, 0x37},
	{ GC9A01_DATA, 0x6F},

	{ GC9A01_CMD, 0xF2}, // SET_GAMMA3
	{ GC9A01_DATA, 0x45},
	{ GC9A01_DATA, 0x09},
	{ GC9A01_DATA, 0x08},
	{ GC9A01_DATA, 0x08},
	{ GC9A01_DATA, 0x26},
	{ GC9A01_DATA, 0x2A},

	{ GC9A01_CMD, 0xF3}, // SET_GAMMA4
	{ GC9A01_DATA, 0x43},
	{ GC9A01_DATA, 0x70},
	{ GC9A01_DATA, 0x72},
	{ GC9A01_DATA, 0x36},
	{ GC9A01_DATA, 0x37},
	{ GC9A01_DATA, 0x6F},

	{ GC9A01_CMD, 0xED},
	{ GC9A01_DATA, 0x1B},
	{ GC9A01_DATA, 0x0B},

	{ GC9A01_CMD, 0xAE},
	{ GC9A01_DATA, 0x77},

	{ GC9A01_CMD, 0xCD},
	{ GC9A01_DATA, 0x63},

	{ GC9A01_CMD, 0x70},
	{ GC9A01_DATA, 0x07},
	{ GC9A01_DATA, 0x07},
	{ GC9A01_DATA, 0x04},
	{ GC9A01_DATA, 0x0E},
	{ GC9A01_DATA, 0x0F},
	{ GC9A01_DATA, 0x09},
	{ GC9A01_DATA, 0x07},
	{ GC9A01_DATA, 0x08},
	{ GC9A01_DATA, 0x03},

	{ GC9A01_CMD, 0xE8},
	{ GC9A01_DATA, 0x34},

	{ GC9A01_CMD, 0x62},
	{ GC9A01_DATA, 0x18},
	{ GC9A01_DATA, 0x0D},
	{ GC9A01_DATA, 0x71},
	{ GC9A01_DATA, 0xED},
	{ GC9A01_DATA, 0x70},
	{ GC9A01_DATA, 0x70},
	{ GC9A01_DATA, 0x18},
	{ GC9A01_DATA, 0x0F},
	{ GC9A01_DATA, 0x71},
	{ GC9A01_DATA, 0xEF},
	{ GC9A01_DATA, 0x70},
	{ GC9A01_DATA, 0x70},

	{ GC9A01_CMD, 0x63},
	{ GC9A01_DATA, 0x18},
	{ GC9A01_DATA, 0x11},
	{ GC9A01_DATA, 0x71},
	{ GC9A01_DATA, 0xF1},
	{ GC9A01_DATA, 0x70},
	{ GC9A01_DATA, 0x70},
	{ GC9A01_DATA, 0x18},
	{ GC9A01_DATA, 0x13},
	{ GC9A01_DATA, 0x71},
	{ GC9A01_DATA, 0xF3},
	{ GC9A01_DATA, 0x70},
	{ GC9A01_DATA, 0x70},

	{ GC9A01_CMD, 0x64},
	{ GC9A01_DATA, 0x28},
	{ GC9A01_DATA, 0x29},
	{ GC9A01_DATA, 0xF1},
	{ GC9A01_DATA, 0x01},
	{ GC9A01_DATA, 0xF1},
	{ GC9A01_DATA, 0x00},
	{ GC9A01_DATA, 0x07},

	{ GC9A01_CMD, 0x66},
	{ GC9A01_DATA, 0x3C},
	{ GC9A01_DATA, 0x00},
	{ GC9A01_DATA, 0xCD},
	{ GC9A01_DATA, 0x67},
	{ GC9A01_DATA, 0x45},
	{ GC9A01_DATA, 0x45},
	{ GC9A01_DATA, 0x10},
	{ GC9A01_DATA, 0x00},
	{ GC9A01_DATA, 0x00},
	{ GC9A01_DATA, 0x00},

	{ GC9A01_CMD, 0x67},
	{ GC9A01_DATA, 0x00},
	{ GC9A01_DATA, 0x3C},
	{ GC9A01_DATA, 0x00},
	{ GC9A01_DATA, 0x00},
	{ GC9A01_DATA, 0x00},
	{ GC9A01_DATA, 0x01},
	{ GC9A01_DATA, 0x54},
	{ GC9A01_DATA, 0x10},
	{ GC9A01_DATA, 0x32},
	{ GC9A01_DATA, 0x98},

	{ GC9A01_CMD, 0x74},
	{ GC9A01_DATA, 0x10},
	{ GC9A01_DATA, 0x85},
	{ GC9A01_DATA, 0x80},
	{ GC9A01_DATA, 0x00},
	{ GC9A01_DATA, 0x00},
	{ GC9A01_DATA, 0x4E},
	{ GC9A01_DATA, 0x00},

	{ GC9A01_CMD, 0x98},
	{ GC9A01_DATA, 0x3E},
	{ GC9A01_DATA, 0x07},

	{ GC9A01_CMD, 0x35}, // Tearing Effect Line ON
	{ GC9A01_CMD, 0x21}, // Display Inversion ON

	{ GC9A01_CMD, 0x11}, // Sleep Out Mode
	{ GC9A01_DELAY, 120},
	{ GC9A01_CMD, GC9A01_DISPON}, // Display ON
	{ GC9A01_DELAY, 255},
	{ GC9A01_END, GC9A01_END},
};



LVGLDispGC9A01::LVGLDispGC9A01(SPI &spi, PinName pinCS, PinName pinCMD, PinName pinRST, PinName pinBacklight, 
                               uint32_t nBufferRows, uint32_t resolutionX, uint32_t resolutionY) :
    LVGLDispDriver(resolutionX, resolutionY),
    _spi(spi),
    _cs(pinCS),
    _cmd(pinCMD),
    _rst(pinRST),
    _backlight(pinBacklight),
    _nBufferRows(nBufferRows)
{
    // low level hardware init
    GC9A01_init();

    // tft controller init
    init();
}

void LVGLDispGC9A01::init()
{
    size_t bufferSize = _horRes * _nBufferRows;

    // allocate memory for display buffer
    _buf1_1 = new lv_color16_t[bufferSize];             /* a buffer for n rows */
    MBED_ASSERT(_buf1_1 != nullptr);
    memset(_buf1_1, 0, bufferSize*sizeof(lv_color16_t));

    lv_display_set_flush_cb(_disp, disp_flush);
    lv_display_set_buffers(_disp, _buf1_1, nullptr, bufferSize, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(_disp, this);
}

void LVGLDispGC9A01::disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LVGLDispGC9A01* instance = (LVGLDispGC9A01*)lv_display_get_user_data(disp);	

    instance->flush(area, (lv_color16_t*)px_map);

    // lv_disp_flush_ready(disp_drv);                 // called by async SPI transfer
}

void LVGLDispGC9A01::flush(const lv_area_t *area, lv_color16_t *color_p)
{
  	_spi.format(8, 0);		// switch to 8 bit transfer for commands

    _cs = 0;
    GC9A01_set_addr_win(area->x1, area->y1, area->x2, area->y2);  	// set display area

  	_spi.format(16, 0);		// switch to 16 bit transfer for data
  	_cmd = 1;

    // int32_t len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1); 	// in 16 bit words
    // _spi.write((const char*)color_p, len, nullptr, 0);						// transfer pixel data

    int len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1) * 2; 	// in bytes
    [[maybe_unused]] volatile int rc = _spi.transfer((uint16_t*)color_p, len, nullptr,  0, callback(this, &LVGLDispGC9A01::flush_ready));

    _cs = 1;
}

void LVGLDispGC9A01::flush_ready(int event_flags)
{
    if (event_flags & SPI_EVENT_COMPLETE) {
        lv_display_flush_ready(_disp);         /* Indicate you are ready with the flushing*/
    }
}

/**
 * Write a command to the GC9A01
 * @param cmd the command
 */
void LVGLDispGC9A01::GC9A01_command(uint8_t cmd)
{
	_cmd = 0;
    _spi.write(cmd);
}

/**
 * Write data to the GC9A01
 * @param data the data
 */
void LVGLDispGC9A01::GC9A01_data(uint8_t data)
{
	_cmd = 1;
    _spi.write(data);
}


// hard reset of the tft controller

void LVGLDispGC9A01::GC9A01_hard_reset()
{
    if (_rst.is_connected()) {
        _rst = 1;
        ThisThread::sleep_for(50ms);
        _rst = 0;
        ThisThread::sleep_for(50ms);
        _rst = 1;
        ThisThread::sleep_for(50ms);
    }
}

// Configuration of the tft controller

void LVGLDispGC9A01::GC9A01_run_cfg_script(void)
{
	int i = 0;
	int end_script = 0;

	do {
		switch (GC9A01_cfg_table[i].cmd)
		{
			case GC9A01_START:
				break;
			case GC9A01_CMD:
				GC9A01_command( GC9A01_cfg_table[i].data & 0xFF );
				break;
			case GC9A01_DATA:
				GC9A01_data( GC9A01_cfg_table[i].data & 0xFF );
				break;
			case GC9A01_DELAY:
                {
                    chrono::milliseconds delaytime(GC9A01_cfg_table[i].data);
                    ThisThread::sleep_for(delaytime);
                    break;
                }
			case GC9A01_END:
				end_script = 1;
                break;
		}
		i++;
	} while (!end_script);
}


void LVGLDispGC9A01::GC9A01_setRotation(uint8_t m) 
{
  GC9A01_command(GC9A01_MADCTL);
  m %= 4; // can't be higher than 3
  switch (m) {
   case 0:
     GC9A01_data(GC9A01_MADCTL_MX | GC9A01_MADCTL_MY | GC9A01_MADCTL_RGB);

    //  _xstart = _colstart;
    //  _ystart = _rowstart;
     break;
   case 1:
     GC9A01_data(GC9A01_MADCTL_MY | GC9A01_MADCTL_MV | GC9A01_MADCTL_RGB);

    //  _ystart = _colstart;
    //  _xstart = _rowstart;
     break;
  case 2:
     GC9A01_data(GC9A01_MADCTL_RGB);
 
    //  _xstart = _colstart;
    //  _ystart = _rowstart;
     break;

   case 3:
     GC9A01_data(GC9A01_MADCTL_MX | GC9A01_MADCTL_MV | GC9A01_MADCTL_RGB);

    //  _ystart = _colstart;
    //  _xstart = _rowstart;
     break;
  }
}

/**
 * Initialize the GC9A01
 */
int LVGLDispGC9A01::GC9A01_init()
{
	GC9A01_hard_reset();
    _cs = 0;
	GC9A01_run_cfg_script();
    _cs = 1;

    _backlight =1;
	
    // GC9A01_fillScreen(0x0000); // Black
	// GC9A01_fillScreen(0xFFFF); // White
	// GC9A01_fillScreen(0xAAAA); // ?

	return 0;
}

void LVGLDispGC9A01::GC9A01_set_addr_win(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  GC9A01_command(GC9A01_CASET); // Column addr set
  GC9A01_data(x0 >> 8);
  GC9A01_data(x0 & 0xFF);     // XSTART 
  GC9A01_data(x1 >> 8);
  GC9A01_data(x1 & 0xFF);     // XEND

  GC9A01_command(GC9A01_RASET); // Row addr set
  GC9A01_data(y0 >> 8);
  GC9A01_data(y0 & 0xFF);     // YSTART
  GC9A01_data(y1 >> 8);
  GC9A01_data(y1 & 0xFF);     // YEND

  GC9A01_command(GC9A01_RAMWR);
}

