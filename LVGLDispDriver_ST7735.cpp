
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

#include "LVGLDispDriver_ST7735.h"


// some flags for initR() :(
#define INITR_GREENTAB 0x0
#define INITR_REDTAB   0x1
#define INITR_BLACKTAB   0x2

#define ST7735_TFTWIDTH  160
#define ST7735_TFTHEIGHT 128

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Color definitions
#define ST7735_BLACK   0x0000
#define ST7735_BLUE    0x001F
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0  
#define ST7735_WHITE   0xFFFF

#define DELAY 0x80
#define CMD_END 0xff

static const unsigned char 	            // Multiple LCD init commands removed
  QDTech[] = {                          // QDTech support only now
	0xf0,	2,	0x5a, 0x5a,				// Excommand2
	ST7735_PWCTR6,	2,	0x5a, 0x5a,		// Excommand3
	0x26,	1,	0x01,					// Gamma set
	0xfa,	15,	0x02, 0x1f,	0x00, 0x10,	0x22, 0x30, 0x38, 0x3A, 0x3A, 0x3A,	0x3A, 0x3A,	0x3d, 0x02, 0x01,	// Positive gamma control
	0xfb,	15,	0x21, 0x00,	0x02, 0x04,	0x07, 0x0a, 0x0b, 0x0c, 0x0c, 0x16,	0x1e, 0x30,	0x3f, 0x01, 0x02,	// Negative gamma control
	0xfd,	11,	0x00, 0x00, 0x00, 0x17, 0x10, 0x00, 0x01, 0x01, 0x00, 0x1f, 0x1f,							// Analog parameter control
	0xf4,	15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x07, 0x00, 0x3C, 0x36, 0x00, 0x3C, 0x36, 0x00,	// Power control
	0xf5,	13, 0x00, 0x70, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6d, 0x66, 0x06,				// VCOM control
	0xf6,	11, 0x02, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x02, 0x00, 0x06, 0x01, 0x00,							// Source control
	0xf2,	17, 0x00, 0x01, 0x03, 0x08, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x04, 0x08, 0x08,	//Display control
	0xf8,	1,	0x11,					// Gate control
	0xf7,	4, 0xc8, 0x20, 0x00, 0x00,	// Interface control
	0xf3,	2, 0x00, 0x00,				// Power sequence control
	ST7735_SLPOUT,	DELAY, 50,			// Wake
	0xf3,	2+DELAY, 0x00, 0x01, 50,	// Power sequence control
	0xf3,	2+DELAY, 0x00, 0x03, 50,	// Power sequence control
	0xf3,	2+DELAY, 0x00, 0x07, 50,	// Power sequence control
	0xf3,	2+DELAY, 0x00, 0x0f, 50,	// Power sequence control
	0xf4,	15+DELAY, 0x00, 0x04, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x07, 0x00, 0x3C, 0x36, 0x00, 0x3C, 0x36, 0x00, 50,	// Power control
	0xf3,	2+DELAY, 0x00, 0x1f, 50,	// Power sequence control
	0xf3,	2+DELAY, 0x00, 0x7f, 50,	// Power sequence control
	0xf3,	2+DELAY, 0x00, 0xff, 50,	// Power sequence control
	0xfd,	11, 0x00, 0x00, 0x00, 0x17, 0x10, 0x00, 0x00, 0x01, 0x00, 0x16, 0x16,							// Analog parameter control
	0xf4,	15, 0x00, 0x09, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x07, 0x00, 0x3C, 0x36, 0x00, 0x3C, 0x36, 0x00,	// Power control
	ST7735_MADCTL,	1, 0x08,			// Memory access data control
	0x35,	1, 0x00,					// Tearing effect line on
	ST7735_COLMOD,	1+DELAY, 0x05, 150,	// Interface pixel control 16 Bit RGB565
	ST7735_DISPON,	0,					// Display on
	ST7735_RAMWR,	0,					// Memory write
    CMD_END
  }; 


static const unsigned char
Bcmd[] = {                              // Initialization commands for 7735B screens
    ST7735_SWRESET, DELAY,              //  1: Software reset, no args, w/delay
    50,                                 //     50 ms delay
    ST7735_SLPOUT, DELAY,               //  2: Out of sleep mode, no args, w/delay
    255,                                //     500 ms delay
    ST7735_COLMOD, 1+DELAY,             //  3: Set color mode, 1 arg + delay:
    0x05,                               //     16-bit color
    10,                                 //     10 ms delay
    ST7735_FRMCTR1, 3+DELAY,            //  4: Frame rate control, 3 args + delay:
    0x00,                               //     fastest refresh
    0x06,                               //     6 lines front porch
    0x03,                               //     3 lines back porch
    10,                                 //     10 ms delay
    ST7735_MADCTL, 1,                   //  5: Memory access ctrl (directions), 1 arg:
    0x08,                               //     Row addr/col addr, bottom to top refresh
    ST7735_DISSET5, 2,                  //  6: Display settings #5, 2 args, no delay:
    0x15,                               //     1 clk cycle nonoverlap, 2 cycle gate
                                        //     rise, 3 cycle osc equalize
    0x02,                               //     Fix on VTL
    ST7735_INVCTR, 1,                   //  7: Display inversion control, 1 arg:
    0x0,                                //     Line inversion
    ST7735_PWCTR1, 2+DELAY,             //  8: Power control, 2 args + delay:
    0x02,                               //     GVDD = 4.7V
    0x70,                               //     1.0uA
    10,                                 //     10 ms delay
    ST7735_PWCTR2, 1,                   //  9: Power control, 1 arg, no delay:
    0x05,                               //     VGH = 14.7V, VGL = -7.35V
    ST7735_PWCTR3, 2,                   // 10: Power control, 2 args, no delay:
    0x01,                               //     Opamp current small
    0x02,                               //     Boost frequency
    ST7735_VMCTR1, 2+DELAY,             // 11: Power control, 2 args + delay:
    0x3C,                               //     VCOMH = 4V
    0x38,                               //     VCOML = -1.1V
    10,                                 //     10 ms delay
    ST7735_PWCTR6, 2,                   // 12: Power control, 2 args, no delay:
    0x11, 0x15,
    ST7735_GMCTRP1, 16,                 // 13: Magical unicorn dust, 16 args, no delay:
    0x09, 0x16, 0x09, 0x20,             //     (seriously though, not sure what
    0x21, 0x1B, 0x13, 0x19,             //      these config values represent)
    0x17, 0x15, 0x1E, 0x2B,
    0x04, 0x05, 0x02, 0x0E,
    ST7735_GMCTRN1, 16+DELAY,           // 14: Sparkles and rainbows, 16 args + delay:
    0x0B, 0x14, 0x08, 0x1E,             //     (ditto)
    0x22, 0x1D, 0x18, 0x1E,
    0x1B, 0x1A, 0x24, 0x2B,
    0x06, 0x06, 0x02, 0x0F,
    10,                                 //     10 ms delay
    ST7735_CASET, 4,                    // 15: Column addr set, 4 args, no delay:
    0x00, 2,                            //     XSTART = 2
    0x00, 127+2,                        //     XEND = 129
    ST7735_RASET, 4,                    // 16: Row addr set, 4 args, no delay:
    0x00, 2,                            //     XSTART = 1
    0x00, 159+2,                        //     XEND = 160
    ST7735_NORON, DELAY,                // 17: Normal display on, no args, w/delay
    10,                                 //     10 ms delay
    ST7735_DISPON, DELAY,               // 18: Main screen turn on, no args, w/delay
    255,                                //     255 = 500 ms delay
    CMD_END
},                  

Rcmd1[] = {                             // Init for 7735R, part 1 (red or green tab)
    ST7735_SWRESET, DELAY,              //  1: Software reset, 0 args, w/delay
    150,                                //     150 ms delay
    ST7735_SLPOUT, DELAY,               //  2: Out of sleep mode, 0 args, w/delay
    255,                                //     500 ms delay
    ST7735_FRMCTR1, 3,                  //  3: Frame rate ctrl - normal mode, 3 args:
    0x01, 0x2C, 0x2D,                   //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3,                  //  4: Frame rate control - idle mode, 3 args:
    0x01, 0x2C, 0x2D,                   //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6,                  //  5: Frame rate ctrl - partial mode, 6 args:
    0x01, 0x2C, 0x2D,                   //     Dot inversion mode
    0x01, 0x2C, 0x2D,                   //     Line inversion mode
    ST7735_INVCTR, 1,                   //  6: Display inversion ctrl, 1 arg, no delay:
    0x07,                               //     No inversion
    ST7735_PWCTR1, 3,                   //  7: Power control, 3 args, no delay:
    0xA2,
    0x02,                               //     -4.6V
    0x84,                               //     AUTO mode
    ST7735_PWCTR2, 1,                   //  8: Power control, 1 arg, no delay:
    0xC5,                               //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3, 2,                   //  9: Power control, 2 args, no delay:
    0x0A,                               //     Opamp current small
    0x00,                               //     Boost frequency
    ST7735_PWCTR4, 2,                   // 10: Power control, 2 args, no delay:
    0x8A,                               //     BCLK/2, Opamp current small & Medium low
    0x2A,
    ST7735_PWCTR5, 2,                   // 11: Power control, 2 args, no delay:
    0x8A, 0xEE,
    ST7735_VMCTR1, 1,                   // 12: Power control, 1 arg, no delay:
    0x0E,
    ST7735_INVOFF, 0,                   // 13: Don't invert display, no args, no delay
    ST7735_MADCTL, 1,                   // 14: Memory access control (directions), 1 arg:
    0xC0,                               //     row addr/col addr, bottom to top refresh
    ST7735_COLMOD, 1,                   // 15: set color mode, 1 arg, no delay:
    0x05,                               //     16-bit color
    CMD_END
},                 

Scmd[] = {                              //  Initialization commands for 7735S screens
    ST7735_SLPOUT, DELAY,               //  2: Out of sleep mode, no args, w/delay
    120,                                //     120 ms delay
    ST7735_FRMCTR1, 3,                  //  4: Frame rate control 1, 3 args
    0x05,                               //     fastest refresh
    0x3C,                               //     6 lines front porch
    0x3C,                               //     3 lines back porch
    ST7735_FRMCTR2, 3,
    0x05,
    0x3C,
    0x3C,
    ST7735_FRMCTR3, 6,
    0x05,
    0x3C,
    0x3C,
    0x05,
    0x3C,
    0x3C,
    ST7735_INVCTR, 1,                   //  7: Display inversion control, 1 arg:
    0x03,                               //     Line inversion
    ST7735_PWCTR1, 3,                   //  7: Power control, 3 args, no delay:
    0xA2,
    0x02,                               //     -4.6V
    0x84,                               //     AUTO mode
    ST7735_PWCTR2, 1,                   //  8: Power control, 1 arg, no delay:
    0xC5,                               //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3, 2,                   //  9: Power control, 2 args, no delay:
    0x0A,                               //     Opamp current small
    0x00,                               //     Boost frequency
    ST7735_PWCTR4, 2,                   // 10: Power control, 2 args, no delay:
    0x8A,                               //     BCLK/2, Opamp current small & Medium low
    0x2A,
    ST7735_PWCTR5, 2,                   // 11: Power control, 2 args, no delay:
    0x8A, 0xEE,
    ST7735_VMCTR1, 1,                   // 11: Power control, 2 args + delay:
    0x1A,                               //     VCOMH = 4V
    ST7735_GMCTRP1, 16,                 // 13: Magical unicorn dust, 16 args, no delay:
    0x04, 0x22, 0x07, 0x0A,             //     (seriously though, not sure what
    0x2E, 0x30, 0x25, 0x2A,             //      these config values represent)
    0x28, 0x26, 0x2E, 0x3A,
    0x00, 0x01, 0x03, 0x13,
    ST7735_GMCTRN1, 16,                 // 14: Sparkles and rainbows, 16 args + delay:
    0x04, 0x16, 0x06, 0x0D,             //     (ditto)
    0x2D, 0x26, 0x23, 0x27,
    0x27, 0x25, 0x2D, 0x3B,
    0x00, 0x01, 0x04, 0x13,
    ST7735_COLMOD, 1,                   //  3: Set color mode, 1 arg + delay:
    0x05,                               //     16-bit color
    ST7735_CASET, 4,                    // 15: Column addr set, 4 args, no delay:
    0x00, 0x00,                         //     XSTART = 0
    0x00, 128-1,                        //     XEND = 80
    ST7735_RASET, 4,                    // 16: Row addr set, 4 args, no delay:
    0x00, 0x00,                         //     XSTART = 0
    0x00, 160-1,                        //    XEND = 160
    ST7735_INVON, 0,                    // 13: Don't invert display, no args, no delay
    ST7735_MADCTL, 1,                   //  5: Memory access ctrl (directions), 1 arg:
    0xC8,                               //     Row addr/col addr, bottom to top refresh
    ST7735_NORON, DELAY,                // 13: Don't invert display, no args, no delay
    10,
    ST7735_DISPON, DELAY,               // 18: Main screen turn on, no args, w/delay
    255,                                //     255 = 500 ms delay
    CMD_END
},


Rcmd2green[] = {                // Init for 7735R, part 2 (green tab only)
    ST7735_CASET, 4,            //  1: Column addr set, 4 args, no delay:
    0x00, 2,                    //     XSTART = 0
    0x00, 127 + 2,              //     XEND = 127
    ST7735_RASET, 4,            //  2: Row addr set, 4 args, no delay:
    0x00, 1,                    //     XSTART = 0
    0x00, 159 + 1,              //     XEND = 159
    CMD_END
},      

Rcmd2red[] = {                  // Init for 7735R, part 2 (red tab only)
    ST7735_CASET, 4,            //  1: Column addr set, 4 args, no delay:
    0x00, 0x00,                 //     XSTART = 0
    0x00, 0x7F,                 //     XEND = 127
    ST7735_RASET, 4,            //  2: Row addr set, 4 args, no delay:
    0x00, 0x00,                 //     XSTART = 0
    0x00, 159,                  //     XEND = 159
    CMD_END
},

Rcmd3[] = {                     // Init for 7735R, part 3 (red or green tab)
    ST7735_GMCTRP1, 16,         //  1: Magical unicorn dust, 16 args, no delay:
    0x02, 0x1c, 0x07, 0x12,
    0x37, 0x32, 0x29, 0x2d,
    0x29, 0x25, 0x2B, 0x39,
    0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16,         //  2: Sparkles and rainbows, 16 args, no delay:
    0x03, 0x1d, 0x07, 0x06,
    0x2E, 0x2C, 0x29, 0x2D,
    0x2E, 0x2E, 0x37, 0x3F,
    0x00, 0x00, 0x02, 0x10,
    ST7735_NORON, DELAY,        //  3: Normal display on, no args, w/delay
    10,                         //     10 ms delay
    ST7735_DISPON, DELAY,       //  4: Main screen turn on, no args w/delay
    100,                        //     100 ms delay
    CMD_END
};



LVGLDispST7735::LVGLDispST7735(SPI &spi, PinName pinCS, PinName pinCMD, PinName pinRST, 
                               uint32_t nBufferRows, uint32_t resolutionX, uint32_t resolutionY) :
    LVGLDispDriver(resolutionX, resolutionY),
    _spi(spi),
    _cs(pinCS),
    _cmd(pinCMD),
    _rst(pinRST),
    _nBufferRows(nBufferRows)
{
    // low level hardware init
  	_spi.format(8, 0);		// should be set already

  	initR(INITR_REDTAB);
    _cs = 0;
    setRotation(1);
    _cs = 1;
    
    // tft controller init
    init();
}

void LVGLDispST7735::init()
{
    size_t bufferSize = _horRes * _nBufferRows;

    // allocate memory for display buffer
    _buf1_1 = new lv_color_t[bufferSize];             /* a buffer for n rows */
    MBED_ASSERT(_buf1_1 != nullptr);
    memset(_buf1_1, 0, bufferSize*sizeof(lv_color_t));

    lv_disp_draw_buf_init(&_disp_buf_1, _buf1_1, NULL, bufferSize);   /* Initialize the display buffer */

    /*Finally register the driver*/
    _disp_drv.flush_cb = disp_flush;
    _disp_drv.draw_buf = &_disp_buf_1;
    _disp_drv.user_data = this;
    _disp = lv_disp_drv_register(&_disp_drv);
}

// static function, calling object method
void LVGLDispST7735::disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    LVGLDispST7735* instance = (LVGLDispST7735*)disp_drv->user_data;

    instance->flush(area, color_p);

    //lv_disp_flush_ready(disp_drv);         // called by async SPI transfer
}

void LVGLDispST7735::flush(const lv_area_t *area, lv_color_t *color_p)
{
  	_spi.format(8, 0);		// switch to 8 bit transfer for commands

    _cs = 0;
    set_addr_win(area->x1, area->y1, area->x2, area->y2);  	// set display area

  	_spi.format(16, 0);		// switch to 16 bit transfer for data
  	_cmd = 1;

    // int len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1); 	// in 16 bit words
    // for (int i=0; i<len; i++) {
    //     _spi.write(color_p->full);
    //     color_p++;
    // }
    
    // int len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1); 	// in 16 bit words
    // _spi.write((const char*)color_p, len, nullptr, 0);						// transfer pixel data
    
    int len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1) * 2; 	// in bytes
    [[maybe_unused]] volatile int rc = _spi.transfer((uint16_t*)color_p, len, nullptr,  0, callback(this, &LVGLDispST7735::flush_ready));
}

void LVGLDispST7735::flush_ready(int event_flags)
{
    if (event_flags & SPI_EVENT_COMPLETE) {
        _cs = 1;
        lv_disp_flush_ready(&_disp_drv);         /* Indicate you are ready with the flushing*/
    }
}

/**
 * Write a command to the ST7735
 * @param cmd the command
 */
void LVGLDispST7735::command(uint8_t cmd)
{
	_cmd = 0;
    _spi.write(cmd);
}

/**
 * Write data to the ST7735
 * @param data the data
 */
void LVGLDispST7735::data(uint8_t data)
{
	_cmd = 1;
    _spi.write(data);
}


// hard reset of the tft controller

void LVGLDispST7735::hard_reset()
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


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void LVGLDispST7735::setRotation(uint8_t m) 
{
    command(ST7735_MADCTL);
    uint rotation = m % 4; // can't be higher than 3
    switch (rotation) {
        case 0:
            data(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
            // _width  = ST7735_TFTWIDTH;
            // _height = ST7735_TFTHEIGHT;
            rowstart = 1;
            colstart = 26;
            break;
        case 1:
            data(MADCTL_MY | MADCTL_MV | MADCTL_RGB);
            // _width  = ST7735_TFTHEIGHT;
            // _height = ST7735_TFTWIDTH;
            break;
        case 2:
            data(MADCTL_RGB);
            // _width  = ST7735_TFTWIDTH;
            // _height = ST7735_TFTHEIGHT;
            break;
        case 3:
//            data(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
            data(MADCTL_MX | MADCTL_MV | MADCTL_BGR);
            // _width  = ST7735_TFTHEIGHT;
            // _height = ST7735_TFTWIDTH;
            rowstart = 26;
            colstart = 1;
            break;
    }
}

void LVGLDispST7735::set_addr_win(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    command(ST7735_CASET); // Column addr set
    data(0x00);
    data(x0+colstart);     // XSTART
    data(0x00);
    data(x1+colstart);     // XEND

    command(ST7735_RASET); // Row addr set
    data(0x00);
    data(y0+rowstart);     // YSTART
    data(0x00);
    data(y1+rowstart);     // YEND

    command(ST7735_RAMWR); // write to RAM
}

void LVGLDispST7735::commandList(const uint8_t *addr)
{

    uint8_t  numArgs;
    uint16_t ms;

    _cs = 0;
    //numCommands = *addr++;   // Number of commands to follow
    while(*addr != CMD_END) {                 // For each command...
        command(*addr++); //   Read, issue command
        numArgs  = *addr++;    //   Number of args to follow
        ms       = numArgs & DELAY;          //   If hibit set, delay follows args
        numArgs &= ~DELAY;                   //   Mask out delay bit
        while(numArgs--) {                   //   For each argument...
            data(*addr++);  //     Read, issue argument
        }

        if(ms) {
            ms = *addr++; // Read post-command delay time (ms)
            if(ms == 255) ms = 500;     // If 255, delay for 500 ms
            wait_us(ms*1000);
        }
    }
    _cs = 1;
}

// Initialization code common to both 'B' and 'R' type displays
void LVGLDispST7735::commonInit(const uint8_t *cmdList)
{

    colstart  = rowstart = 0; // May be overridden in init func

    _cmd = 1;
    _cs = 1;

    hard_reset();

    if(cmdList) commandList(cmdList);
}


// Initialization for ST7735B screens
void LVGLDispST7735::initB(void)
{
    // commonInit(Bcmd);
    commonInit(QDTech);
    rowstart = 1;
    colstart = 26;
}

void LVGLDispST7735::initS(void)
{
    commonInit(Scmd);
    rowstart = 1;
    colstart = 26;
}

// Initialization for ST7735R screens (green or red tabs)
void LVGLDispST7735::initR(uint8_t options)
{
    commonInit(Rcmd1);
    if(options == INITR_GREENTAB) {
        commandList(Rcmd2green);
        colstart = 26;
        rowstart = 1;
    } else {
        // colstart, rowstart left at default '0' values
        commandList(Rcmd2red);
    }
    commandList(Rcmd3);
}

