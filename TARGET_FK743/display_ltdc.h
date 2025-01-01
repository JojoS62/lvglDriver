#pragma once

#include "mbed.h"
#include "lcd_fonts.h"

#define LCD_Width     		800						//	LCD的像素长度
#define LCD_Height    		480						//	LCD的像素宽度
#define LCD_MemoryAdd   	((uint32_t)0xC0000000)  //SDRAM_BANK_ADDR 		//	显存的起始地址  


#define 	LCD_WHITE       0xffFFFFFF		// 纯白色
#define 	LCD_BLACK       0xff000000		// 纯黑色
                           
#define 	LCD_BLUE        0xff0000FF		//	纯蓝色
#define 	LCD_GREEN       0xff00FF00    //	纯绿色
#define 	LCD_RED         0xffFF0000    //	纯红色
#define 	LCD_CYAN        0xff00FFFF    //	蓝绿色
#define 	LCD_MAGENTA     0xffFF00FF    //	紫红色
#define 	LCD_YELLOW      0xffFFFF00    //	黄色
#define 	LCD_GREY        0xff2C2C2C    //	灰色
													
#define 	LIGHT_BLUE      0xff8080FF    //	亮蓝色
#define 	LIGHT_GREEN     0xff80FF80    //	亮绿色
#define 	LIGHT_RED       0xffFF8080    //	亮红色
#define 	LIGHT_CYAN      0xff80FFFF    //	亮蓝绿色
#define 	LIGHT_MAGENTA   0xffFF80FF    //	亮紫红色
#define 	LIGHT_YELLOW    0xffFFFF80    //	亮黄色
#define 	LIGHT_GREY      0xffA3A3A3    //	亮灰色
													
#define 	DARK_BLUE       0xff000080    //	暗蓝色
#define 	DARK_GREEN      0xff008000    //	暗绿色
#define 	DARK_RED        0xff800000    //	暗红色
#define 	DARK_CYAN       0xff008080    //	暗蓝绿色
#define 	DARK_MAGENTA    0xff800080    //	暗紫红色
#define 	DARK_YELLOW     0xff808000    //	暗黄色
#define 	DARK_GREY       0xff404040    //	暗灰色

class DisplayLTDC : private NonCopyable<DisplayLTDC> {
    public:
    DisplayLTDC();
    void LCD_Init();
    void LCD_SetLayer(uint8_t layer);
    void LCD_SetColor(uint32_t Color);
    void LCD_SetBackColor(uint32_t Color);
    void LCD_SetFont(pFONT *fonts);
    void LCD_DisplayDirection(uint8_t direction);
    void LCD_Clear();
    void LCD_ClearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void LCD_DrawPoint(uint16_t x, uint16_t y, uint32_t color);
    uint32_t LCD_ReadPoint(uint16_t x, uint16_t y);
    void LCD_DisplayChar(uint16_t x, uint16_t y, uint8_t c);
    void LCD_DisplayString( uint16_t x, uint16_t y, const char *p);
    void LCD_SetTextFont(pFONT *fonts);
    void LCD_DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *pImage);
    void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t r);
    void LCD_DrawEllipse(int x, int y, int r1, int r2);
    void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void LCD_FillCircle(uint16_t x, uint16_t y, uint16_t r);
    void LCD_ShowNumMode(uint8_t mode);

    private:
    DMA2D_HandleTypeDef hdma2d;	

    pFONT *Fonts;		// English Font
    pFONT *CHFonts;		// Chinese Font

	uint32_t Color;
	uint32_t BackColor;
	uint32_t ColorMode;
	uint32_t LayerMemoryAdd;
	uint8_t  Layer;
	uint8_t  Direction;
	uint8_t  BytesPerPixel;
	uint8_t  ShowNum_Mode;


};