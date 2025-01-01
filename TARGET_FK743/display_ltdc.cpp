#include "display_ltdc.h"
// #include "sdram.h"

LTDC_HandleTypeDef hltdc;

#define LCD_NUM_LAYERS  1

// #define	ColorMode_0   LTDC_PIXEL_FORMAT_RGB565
//#define	ColorMode_0   LTDC_PIXEL_FORMAT_ARGB1555
//#define	ColorMode_0    LTDC_PIXEL_FORMAT_ARGB4444
// #define	ColorMode_0   LTDC_PIXEL_FORMAT_RGB888
#define	ColorMode_0   LTDC_PIXEL_FORMAT_ARGB8888
#define	ColorMode_1   LTDC_PIXEL_FORMAT_ARGB8888

#define LTDC_R0_PIN						GPIO_PIN_15
#define	LTDC_R0_PORT					GPIOI

#define LTDC_R1_PIN						GPIO_PIN_0
#define	LTDC_R1_PORT					GPIOJ

#define LTDC_G0_PIN						GPIO_PIN_7
#define	LTDC_G0_PORT					GPIOJ

#define LTDC_G1_PIN						GPIO_PIN_8
#define	LTDC_G1_PORT					GPIOJ

#define LTDC_B0_PIN						GPIO_PIN_12
#define	LTDC_B0_PORT					GPIOJ

#define LTDC_B1_PIN						GPIO_PIN_13
#define	LTDC_B1_PORT					GPIOJ

#define LCD_Backlight_PIN				GPIO_PIN_6
#define	LCD_Backlight_PORT				GPIOH
#define GPIO_LDC_Backlight_CLK_ENABLE   __HAL_RCC_GPIOH_CLK_ENABLE()

#define	LCD_Backlight_OFF		        HAL_GPIO_WritePin(LCD_Backlight_PORT, LCD_Backlight_PIN, GPIO_PIN_RESET);	// Turn off backlight
#define LCD_Backlight_ON		        HAL_GPIO_WritePin(LCD_Backlight_PORT, LCD_Backlight_PIN, GPIO_PIN_SET);		// Turn on backlight

#define HBP  80	// Set according to the on-screen manual
#define VBP  40
#define HSW  1
#define VSW  1
#define HFP  200
#define VFP  22

// The FK750M5-XBH6 core board uses external SDRAM as video memory, starting at 0xC0000000
// Video memory space required = resolution * number of bytes per pixel, 
// for example, a 800*480 screen, using 16-bit color (RGB565 or AEGB1555), requires video memory 800*480*2 = 768000 bytes

// Display direction parameters
// Example: LCD_DisplayDirection(Direction_H), set the screen to display horizontally
// Example: LCD_DisplayDirection(Direction_V), set the screen to display vertically

#define	Direction_H	0		// LCD horizontal display
#define	Direction_V	1		// LCD vertical display

/*---------------------------------------- Common colors ------------------------------------------------------

1. For the convenience of users, 32-bit color is defined here, and then it is automatically converted into the color required by the corresponding color format through code
2. In the 32-bit color, from high to low, it corresponds to 4 color channels A, R, G, and B, where A represents the transparent channel
3. Up to 255 levels of transparent color can be set, ff represents opaque, and 0 represents completely transparent
4. Unless you use color formats that support transparent colors such as ARGB1555 and ARGB8888, transparent colors will not work. ARGB1555 only supports one transparent color, that is, there are only two states of transparent and opaque. ARGB4444 has 16 levels of transparency, and ARGB8888 supports 255 levels of transparency
5. Users can use the color palette on the computer to obtain 24-bit RGB color, and then add the transparent channel to get 32-bit color, and then input this 32-bit color
   LCD_SetColor() or LCD_SetBackColor() to display the corresponding color.
6. Usage example: The RGB value of pure blue is 0x0000FF. If transparent color is not required, the corresponding 32-bit color value is 0xff0000FF
7. The colors defined below are all set to opaque. Users can define the corresponding colors according to their needs */


// Set whether to fill extra digits with 0 or space when displaying variables
// Only LCD_DisplayNumber() to display integers and LCD_DisplayDecimals() to display decimals are used
// Usage example: LCD_ShowNumMode(Fill_Zero) Set extra digits to fill with 0, for example, 123 can be displayed as 000123

#define  Fill_Zero  0		// Fill with 0
#define  Fill_Space 1		// Filling spaces


#if ( (ColorMode_0 == LTDC_PIXEL_FORMAT_RGB565) || (ColorMode_0 == LTDC_PIXEL_FORMAT_ARGB1555) || (ColorMode_0 ==LTDC_PIXEL_FORMAT_ARGB4444) )
#define BytesPerPixel_0		2		// 16-bit color mode occupies 2 bytes per pixel
#elif ColorMode_0 == LTDC_PIXEL_FORMAT_RGB888
#define BytesPerPixel_0		3		// 24-bit color mode occupies 3 bytes per pixel
#else
#define BytesPerPixel_0		4		// 32-bit color mode occupies 4 bytes per pixel
#endif

#if LCD_NUM_LAYERS == 2

#if ( (ColorMode_1 == LTDC_PIXEL_FORMAT_RGB565) || (ColorMode_1 == LTDC_PIXEL_FORMAT_ARGB1555) || (ColorMode_1 == LTDC_PIXEL_FORMAT_ARGB4444) )
#define BytesPerPixel_1		2	
#elif ColorMode_1 == LTDC_PIXEL_FORMAT_RGB888
#define BytesPerPixel_1		3	
#else
#define BytesPerPixel_1		4	
#endif

#define LCD_MemoryAdd_OFFSET   LCD_Width * LCD_Height * BytesPerPixel_0 	 // The offset address of the second layer of video memory 

#endif

DisplayLTDC::DisplayLTDC()
{
}

void DisplayLTDC::LCD_Init()
{
    LTDC_LayerCfgTypeDef pLayerCfg = {0};		    // layer0 parameters
    __HAL_RCC_DMA2D_CLK_ENABLE();					// Enable DMA2D clock

    hltdc.Instance 	        = LTDC;
    hltdc.Init.HSPolarity   = LTDC_HSPOLARITY_AL;	// Low level is effective
    hltdc.Init.VSPolarity   = LTDC_VSPOLARITY_AL;   // Low level is effective
    hltdc.Init.DEPolarity   = LTDC_DEPOLARITY_AL;   // Low level is effective，It should be noted that many panels are high level effective, but the 750 needs to be set to low level to display normally.
    hltdc.Init.PCPolarity   = LTDC_PCPOLARITY_IPC;  // Normal clock signal

    hltdc.Init.HorizontalSync 		= HSW - 1;									// Just set the parameters according to the screen
    hltdc.Init.VerticalSync 		= VSW - 1;
    hltdc.Init.AccumulatedHBP		= HBP + HSW -1;
    hltdc.Init.AccumulatedVBP 		= VBP + VSW -1;
    hltdc.Init.AccumulatedActiveW   = LCD_Width  + HSW + HBP -1;
    hltdc.Init.AccumulatedActiveH   = LCD_Height + VSW + VBP -1;
    hltdc.Init.TotalWidth 			= LCD_Width  + HSW + HBP + HFP - 1;
    hltdc.Init.TotalHeigh 			= LCD_Height + VSW + VBP + VFP - 1;

    hltdc.Init.Backcolor.Red 		= 0;	    // Initial background color, R
    hltdc.Init.Backcolor.Green 	    = 0;        // Initial background color, G
    hltdc.Init.Backcolor.Blue 		= 0;	    // Initial background color, B

    HAL_LTDC_Init(&hltdc);	        // Initialize LTDC parameters

    /*---------------------------------- layer0 显示配置 --------------------------------*/

    pLayerCfg.WindowX0 			= 0;							// Horizontal start
    pLayerCfg.WindowX1 			= LCD_Width;					// Horizontal end point
    pLayerCfg.WindowY0 			= 0;							// Vertical start point
    pLayerCfg.WindowY1 			= LCD_Height;					// Vertical end point
    pLayerCfg.ImageWidth 		= LCD_Width;                    // Display area width
    pLayerCfg.ImageHeight 		= LCD_Height;                   // Display area height
    pLayerCfg.PixelFormat 		= ColorMode_0;					// Color Format

    // Configure the constant transparency of layer0 and finally write it into the LTDC_LxCACR register
    // Note that this parameter directly configures the transparency of the entire layer0. Here, setting it to 255 means it is opaque.    
    pLayerCfg.Alpha 				= 255;									// The value range is 0~255, 255 means opaque, 0 means completely transparent

    // Set the layer blending coefficient of layer0 and finally write it to the LTDC_LxBFCR register
    // This parameter is used to set the color blending coefficient between layer0 and the underlying background. The calculation formula is:
    // Blended color = BF1 * layer0 color + BF2 * underlying background color
    // If layer0 uses a transparent color, it must be configured as LTDC_BLENDING_FACTOR1_PAxCA and LTDC_BLENDING_FACTOR2_PAxCA, otherwise the A channel in ARGB will not work
    // For more information, please refer to the reference manual for the introduction of the LTDC_LxBFCR register    
    pLayerCfg.BlendingFactor1 	= LTDC_BLENDING_FACTOR1_CA;		// Mixing factor 1
    pLayerCfg.BlendingFactor2 	= LTDC_BLENDING_FACTOR2_CA;     // Mixing factor 2

    pLayerCfg.FBStartAdress 	= LCD_MemoryAdd;                // Video memory address

    // Configure the initial default color of layer0, 
    // including the values ​​of A, R, G, and B, and finally write them into the LTDC_LxDCCR register
    pLayerCfg.Alpha0 			= 0;		        // Initial color, A
    pLayerCfg.Backcolor.Blue 	= 0;                //	Initial color, R
    pLayerCfg.Backcolor.Green 	= 0;                //	Initial color, G
    pLayerCfg.Backcolor.Red 	= 0;		        //	Initial color, B

    HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0);	// Configure layer0

#if ( ( ColorMode_0 == LTDC_PIXEL_FORMAT_RGB888 ) || ( ColorMode_0 == LTDC_PIXEL_FORMAT_ARGB8888 ) ) // Determine whether to use 24-bit or 32-bit color
    // Because the low bits of each channel of the 750 are output using pseudo-random dithering, 
    // if color dithering is not enabled, 24-bit or 32-bit color cannot be displayed normally.
    HAL_LTDC_EnableDither(&hltdc); // Enable color dithering
#endif


    /*---------------------------------- layer1 显示配置 --------------------------------*/

#if ( LCD_NUM_LAYERS == 2 )

    LTDC_LayerCfgTypeDef pLayerCfg1 = {0};

    pLayerCfg1.WindowX0 				= 0;
    pLayerCfg1.WindowX1 				= LCD_Width;
    pLayerCfg1.WindowY0 				= 0;
    pLayerCfg1.WindowY1 				= LCD_Height;
    pLayerCfg1.ImageWidth 			    = LCD_Width;
    pLayerCfg1.ImageHeight 			    = LCD_Height;
    pLayerCfg1.PixelFormat 			    = ColorMode_1;

    pLayerCfg1.Alpha 					= 255;

    pLayerCfg1.BlendingFactor1 	= LTDC_BLENDING_FACTOR1_PAxCA;
    pLayerCfg1.BlendingFactor2 	= LTDC_BLENDING_FACTOR2_PAxCA;

    pLayerCfg1.FBStartAdress 		= LCD_MemoryAdd + LCD_MemoryAdd_OFFSET;


    pLayerCfg1.Alpha0 				= 0;
    pLayerCfg1.Backcolor.Red 		= 0;
    pLayerCfg1.Backcolor.Green 	    = 0;
    pLayerCfg1.Backcolor.Blue 		= 0;

    HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg1, 1);	// Initialize layer1 configuration

#if ( ( ColorMode_1 == LTDC_PIXEL_FORMAT_RGB888 ) || ( ColorMode_1 == LTDC_PIXEL_FORMAT_ARGB8888 ) ) // Determine whether to use 24-bit or 32-bit color
    // Because the low bits of each channel of 750 are output using pseudo-random dithering, 
    // if color dithering is not enabled, 24-bit or 32-bit color cannot be displayed normally.
    HAL_LTDC_EnableDither(&hltdc); // Enable color dithering
#endif

#endif

    /*---------------------------------- 初始化一些默认配置 --------------------------------*/

    LCD_DisplayDirection(Direction_H); 	//	设置横屏显示
    LCD_SetFont(&Font24); 				 	//	设置默认字体
    LCD_ShowNumMode(Fill_Space);			//	设置数字显示默认填充空格

    LCD_SetLayer(0);  						// 切换到 layer0
    LCD_SetBackColor(LCD_BLACK); 			//	设置背景色
    LCD_SetColor(LCD_WHITE);				//	设置画笔颜色
    LCD_Clear(); 								//	清屏，刷背景色

    /*---------------------------------- 如果开启了两层显示 --------------------------------*/

#if LCD_NUM_LAYERS == 2

    LCD_SetLayer(1); 							// 切换到前景层 layer1，layer1始终在layer0之上
    LCD_SetBackColor(LCD_BLACK); 			//	设置背景色
    LCD_SetColor(LCD_WHITE);				//	设置画笔颜色
    LCD_Clear(); 								//	清屏，刷背景色

#endif

// LTDC在初始化之后，上电的瞬间会有一个短暂的白屏，
//	即使一开始就将背光引脚拉低并且屏幕背光引脚用电阻下拉还是会有这个现象，
//	如果需要消除这个现象，可以在初始化完毕之后，进行一个短暂的延时再打开背光
//
//	HAL_Delay(200);	// 延时200ms

    LCD_Backlight_ON;							// 开启背光
}

void DisplayLTDC::LCD_SetLayer(uint8_t layer)
{
#if LCD_NUM_LAYERS == 2		// 如果开了双层
	
	if (layer == 0)			// 如果设置的是 layer0
	{
		LayerMemoryAdd = LCD_MemoryAdd; 	// 获取 layer0 的显存地址
		ColorMode      = ColorMode_0;		// 获取 layer0 的颜色格式
		BytesPerPixel  = BytesPerPixel_0;	// 获取 layer0 的每个像素所需字节数的大小
	}
	else if(layer == 1)	 // 如果设置的是 layer1
	{
		LayerMemoryAdd = LCD_MemoryAdd + LCD_MemoryAdd_OFFSET;	// 获取 layer1 的显存地址
		ColorMode      = ColorMode_1;                           // 获取 layer1 的颜色格式
		BytesPerPixel  = BytesPerPixel_1;		                  // 获取 layer1 的每个像素所需字节数的大小
	}
	Layer = layer;	//记录当前所在的层
	
#else		// 如果只开启单层，默认操作 layer0
	
	LayerMemoryAdd = LCD_MemoryAdd;		// 获取 layer0 的显存地址
	ColorMode      = ColorMode_0;      // 获取 layer0 的颜色格式
	BytesPerPixel  = BytesPerPixel_0;	// 获取 layer0 的每个像素所需字节数的大小
	Layer = 0;		// 层标记设置为 layer0
	
#endif
}

void LCD_PanelModify()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;				//	输出模式
    GPIO_InitStruct.Pull 	= GPIO_NOPULL;							//	无上下拉
    GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW;				// 速度等级2M

    GPIO_InitStruct.Pin 		= LTDC_R0_PIN; 			//	R0
    HAL_GPIO_Init(LTDC_R0_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin 		= LTDC_R1_PIN; 			//	R1
    HAL_GPIO_Init(LTDC_R1_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin 		= LTDC_G0_PIN; 			//	G0
    HAL_GPIO_Init(LTDC_G0_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin 		= LTDC_G1_PIN; 			//	G1
    HAL_GPIO_Init(LTDC_G1_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin 		= LTDC_B0_PIN; 			//	B0
    HAL_GPIO_Init(LTDC_B0_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin 		= LTDC_B1_PIN; 			//	B1
    HAL_GPIO_Init(LTDC_B1_PORT, &GPIO_InitStruct);

    // The old 5-inch screen only leads to the 18-bit interface, and the remaining data pins are grounded
    // Therefore, it is necessary to output the low 2-bit data pin corresponding to LTDC to a low level

    HAL_GPIO_WritePin(LTDC_R0_PORT, LTDC_R0_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LTDC_R1_PORT, LTDC_R1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LTDC_G0_PORT, LTDC_G0_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LTDC_G1_PORT, LTDC_G1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LTDC_B0_PORT, LTDC_B0_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LTDC_B1_PORT, LTDC_B1_PIN, GPIO_PIN_RESET);
}

extern "C" void HAL_LTDC_MspInit(LTDC_HandleTypeDef *hltdc)
{
    /* 设置LTDC时钟，这里设置为33MHz，即刷新率在60帧左右，过高或者过低都会造成闪烁 */
    /* LCD clock configuration */
    /* PLL3_VCO Input = HSE_VALUE/PLL3M = 1 Mhz */
    /* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 330 Mhz */
    /* PLLLCDCLK = PLL3_VCO Output/PLL3R = 330/10 = 33 Mhz */
    /* LTDC clock frequency = PLLLCDCLK = 33 Mhz */  
    
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
        
    PeriphClkInitStruct.PLL3.PLL3M = 25;
    PeriphClkInitStruct.PLL3.PLL3N = 330;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 2;
    PeriphClkInitStruct.PLL3.PLL3R = 10;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;               
    // PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_D1HCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        MBED_ERROR(MBED_MAKE_CUSTOM_ERROR(MBED_MODULE_HAL, -1), "HAL_LTDC_MspInit failed");
    }

    uint8_t Modify_Flag = 0;	// 旧面板识别标志位

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hltdc->Instance==LTDC)
    {

        __HAL_RCC_LTDC_CLK_ENABLE();		// 使能LTDC时钟

        __HAL_RCC_GPIOK_CLK_ENABLE();	// 使能IO口时钟
        __HAL_RCC_GPIOG_CLK_ENABLE();
        __HAL_RCC_GPIOJ_CLK_ENABLE();
        __HAL_RCC_GPIOH_CLK_ENABLE();
        __HAL_RCC_GPIOI_CLK_ENABLE();

        GPIO_LDC_Backlight_CLK_ENABLE;	// 使能背光引脚IO口时钟

        /*------------------------------LTDC GPIO Configuration--------------------------

            PI15    ------> LTDC_R0		PJ7     ------> LTDC_G0			 PJ12    ------> LTDC_B0
            PJ0     ------> LTDC_R1	   PJ8     ------> LTDC_G1        PJ13    ------> LTDC_B1
            PJ1     ------> LTDC_R2      PJ9     ------> LTDC_G2	       PJ14    ------> LTDC_B2
            PJ2     ------> LTDC_R3      PG10    ------> LTDC_G3	       PJ15    ------> LTDC_B3
            PJ3     ------> LTDC_R4      PH15    ------> LTDC_G4	       PK3     ------> LTDC_B4
            PJ4     ------> LTDC_R5      PH4     ------> LTDC_G5        PK4     ------> LTDC_B5
            PJ5     ------> LTDC_R6	   PK1     ------> LTDC_G6	       PK5     ------> LTDC_B6
            PJ6     ------> LTDC_R7	   PK2     ------> LTDC_G7	       PK6     ------> LTDC_B7

            PI12     ------> LTDC_HSYNC
            PI13     ------> LTDC_VSYNC
            PK7      ------> LTDC_DE
            PI14     ------> LTDC_CLK
        */

        /*----------------------------旧面板识别---------------------------------*/


        GPIO_InitStruct.Mode 	= GPIO_MODE_INPUT;		//	输入模式
        GPIO_InitStruct.Pull 	= GPIO_NOPULL;				//	不上下拉

        GPIO_InitStruct.Pin 		= LTDC_R0_PIN; 			//	R0
        HAL_GPIO_Init(LTDC_R0_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin 		= LTDC_G0_PIN; 			//	G0
        HAL_GPIO_Init(LTDC_G0_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin 		= LTDC_B0_PIN; 			//	B0
        HAL_GPIO_Init(LTDC_B0_PORT, &GPIO_InitStruct);


        // 旧面板识别修改，此处只对旧款5寸屏有用，也就是 RGB050M1  V1.0的版本
        // 旧款5寸屏只引出了18位的接口，低2位数据脚直接接地，如果对应的引脚为低电平则可以确定接入的是旧款5寸屏
        // 其它面板无需理会此处，用户移植的时候可以直接删掉
        if(   (HAL_GPIO_ReadPin(LTDC_R0_PORT,LTDC_R0_PIN) == 0) \
                && (HAL_GPIO_ReadPin(LTDC_G0_PORT,LTDC_G0_PIN) == 0) \
                && (HAL_GPIO_ReadPin(LTDC_B0_PORT,LTDC_B0_PIN) == 0) )
        {

            Modify_Flag	= 1;	// 如果检测到了旧款5寸屏，则置位标志位
        }

        /*-----------------------------------------------------------------------*/


        GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_6|GPIO_PIN_3
                              |GPIO_PIN_7|GPIO_PIN_2|GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_12|GPIO_PIN_13
                              |GPIO_PIN_9|GPIO_PIN_0|GPIO_PIN_8|GPIO_PIN_7
                              |GPIO_PIN_6|GPIO_PIN_1|GPIO_PIN_5|GPIO_PIN_3|GPIO_PIN_2
                              |GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
        HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

        // 初始化背光引脚

        GPIO_InitStruct.Pin 		= LCD_Backlight_PIN;				// 背光引脚
        GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;			// 推挽输出模式
        GPIO_InitStruct.Pull 	= GPIO_NOPULL;						// 无上下拉
        GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW;			// 速度等级低
        HAL_GPIO_Init(LCD_Backlight_PORT, &GPIO_InitStruct);	// 初始化

        LCD_Backlight_ON;	// 先关闭背光引脚，初始化LTDC之后再开启

        // 旧面板识别修改，此处只对旧款5寸屏有用，也就是 RGB050M1  V1.0的版本
        // 旧款5寸屏只引出了18位的接口，低2位数据脚直接接地，如果对应的引脚为低电平则可以确定接入的是旧款5寸屏
        // 其它面板无需理会此处，用户移植的时候可以直接删掉

        if (Modify_Flag == 1 )
        {
            LCD_PanelModify();	// 将对应的低2位引脚输出低电平，否则单片机功耗会很高
        }

        /* LTDC interrupt Init */
        HAL_NVIC_SetPriority(LTDC_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(LTDC_IRQn);

    }
}

void DisplayLTDC::LCD_SetColor(uint32_t color)
{
    uint16_t Alpha_Value = 0, Red_Value = 0, Green_Value = 0, Blue_Value = 0; //各个颜色通道的值

    if (ColorMode == LTDC_PIXEL_FORMAT_RGB565	)	//将32位色转换为16位色
    {
        Red_Value   = (uint16_t)((color&0x00F80000)>>8);
        Green_Value = (uint16_t)((color&0x0000FC00)>>5);
        Blue_Value  = (uint16_t)((color&0x000000F8)>>3);
        Color = (uint16_t)(Red_Value | Green_Value | Blue_Value);
    }
    else if (ColorMode == LTDC_PIXEL_FORMAT_ARGB1555 )	//将32位色转换为ARGB1555颜色
    {
        if( (Color & 0xFF000000) == 0 )	//判断是否使用透明色
            Alpha_Value = 0x0000;
        else
            Alpha_Value = 0x8000;

        Red_Value   = (uint16_t)((color&0x00F80000)>>9);
        Green_Value = (uint16_t)((color&0x0000F800)>>6);
        Blue_Value  = (uint16_t)((color&0x000000F8)>>3);
        Color = (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
    }
    else if (ColorMode == LTDC_PIXEL_FORMAT_ARGB4444 )	//将32位色转换为ARGB4444颜色
    {

        Alpha_Value = (uint16_t)((Color&0xf0000000)>>16);
        Red_Value   = (uint16_t)((Color&0x00F00000)>>12);
        Green_Value = (uint16_t)((Color&0x0000F000)>>8);
        Blue_Value  = (uint16_t)((Color&0x000000F8)>>4);
        Color = (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
    }
    else
        Color = color;	//24位色或32位色不需要转换
}

void DisplayLTDC::LCD_SetBackColor(uint32_t color)
{
    uint16_t Alpha_Value = 0, Red_Value = 0, Green_Value = 0, Blue_Value = 0;  //各个颜色通道的值

    if( ColorMode == LTDC_PIXEL_FORMAT_RGB565	)	//将32位色转换为16位色
    {
        Red_Value   	= (uint16_t)((color&0x00F80000)>>8);
        Green_Value 	= (uint16_t)((color&0x0000FC00)>>5);
        Blue_Value  	= (uint16_t)((color&0x000000F8)>>3);
        BackColor	= (uint16_t)(Red_Value | Green_Value | Blue_Value);
    }
    else if( ColorMode == LTDC_PIXEL_FORMAT_ARGB1555 )	//将32位色转换为ARGB1555颜色
    {
        if( (Color & 0xFF000000) == 0 )	//判断是否使用透明色
            Alpha_Value = 0x0000;
        else
            Alpha_Value = 0x8000;

        Red_Value   	= (uint16_t)((color&0x00F80000)>>9);
        Green_Value 	= (uint16_t)((color&0x0000F800)>>6);
        Blue_Value  	= (uint16_t)((color&0x000000F8)>>3);
        BackColor 	= (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
    }
    else if( ColorMode == LTDC_PIXEL_FORMAT_ARGB4444 )	//将32位色转换为ARGB4444颜色
    {

        Alpha_Value 	= (uint16_t)((color&0xf0000000)>>16);
        Red_Value   	= (uint16_t)((color&0x00F00000)>>12);
        Green_Value 	= (uint16_t)((color&0x0000F000)>>8);
        Blue_Value  	= (uint16_t)((color&0x000000F8)>>4);
        BackColor 	= (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
    }

    else
        BackColor = color;	// No conversion required for 24-bit or 32-bit color
}

void DisplayLTDC::LCD_SetFont(pFONT *fonts)
{
    Fonts = fonts;
}

void DisplayLTDC::LCD_DisplayDirection(uint8_t direction)
{
    Direction = direction;
}

void DisplayLTDC::LCD_Clear()
{

    DMA2D->CR	    &=	~(DMA2D_CR_START);				//	stop DMA2D
    DMA2D->CR		=	DMA2D_R2M;						//	register to SDRAM
    DMA2D->OPFCCR	=	ColorMode;						//	format colors
    DMA2D->OOR		=	0;								//	set row offset
    DMA2D->OMAR		=	LayerMemoryAdd ;				//  address
    DMA2D->NLR		=	(LCD_Width<<16)|(LCD_Height);	//	set length and width
    DMA2D->OCOLR	=	BackColor;						//	set backcolor

    /******
    Waiting for the vertical data to enable display status, that is, when the LTDC is about to finish brushing a whole screen of data
    Because the screen is refreshed before a frame is refreshed, there will be tearing
    Users can also use the register reload interrupt to make judgments, but in order to ensure the simplicity of the routine and the convenience of transplantation, the method of judging the register is directly used here
    If no judgment is made, the DMA2D screen refresh speed is as follows
    Color format RGB565 RGB888 ARGB888
    Time consumption 4.3ms 7.5ms 11.9ms
    After adding, no matter which format, it takes 17.6ms to refresh a screen, but the refresh rate of the screen itself is only about 60 frames (LTDC clock 33MHz),
    The speed of 17.6ms is enough. Unless the speed requirement is particularly high, it is recommended to add a statement to judge the vertical wait to avoid the tearing effect.
    ******/

    while( LTDC->CDSR != 0X00000001);	    //  determine the display status register LTDC_CDSR bit 0 VDES: vertical data enable display status

    DMA2D->CR	  |=	DMA2D_CR_START;		//  start DMA2D

    while (DMA2D->CR & DMA2D_CR_START) ;	//	wait for the transfer to complete
}

void DisplayLTDC::LCD_ClearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    DMA2D->CR	  &=	~(DMA2D_CR_START);				//	停止DMA2D
    DMA2D->CR		=	DMA2D_R2M;							//	寄存器到SDRAM
    DMA2D->OPFCCR	=	ColorMode;						//	设置颜色格式
    DMA2D->OCOLR	=	BackColor ;					//	颜色

    if(Direction == Direction_H)  //横屏填充
    {
        DMA2D->OOR		=	LCD_Width - width;				//	设置行偏移
        DMA2D->OMAR		=	LayerMemoryAdd + BytesPerPixel*(LCD_Width * y + x);	// 地址;
        DMA2D->NLR		=	(width<<16)|(height);			//	设定长度和宽度
    }
    else	//竖屏填充
    {
        DMA2D->OOR		=	LCD_Width - height;		//	设置行偏移
        DMA2D->OMAR		=	LayerMemoryAdd + BytesPerPixel*((LCD_Height - x - 1 - width)*LCD_Width + y);	// 地址
        DMA2D->NLR		=	(width)|(height<<16);	//	设定长度和宽度
    }

    DMA2D->CR	  |=	DMA2D_CR_START;					//	启动DMA2D

    while (DMA2D->CR & DMA2D_CR_START) ;			//	等待传输完成
}

void DisplayLTDC::LCD_DrawPoint(uint16_t x, uint16_t y, uint32_t color)
{

    /*----------------------- 32位色 ARGB8888 模式 ----------------------*/

    if( ColorMode == LTDC_PIXEL_FORMAT_ARGB8888 )
    {
        if (Direction == Direction_H) //水平方向
        {
            *(__IO uint32_t*)( LayerMemoryAdd + 4*(x + y*LCD_Width) ) = color ;
        }
        else if(Direction == Direction_V)	//垂直方向
        {
            *(__IO uint32_t*)( LayerMemoryAdd + 4*((LCD_Height - x - 1)*LCD_Width + y) ) = color ;
        }
    }
    /*----------------------------- 24位色 RGB888 模式 -------------------------*/

    else if ( ColorMode == LTDC_PIXEL_FORMAT_RGB888 )
    {
        if (Direction == Direction_H) //水平方向
        {
            *(__IO uint16_t*)( LayerMemoryAdd + 3*(x + y*LCD_Width) ) = color ;
            *(__IO uint8_t*)( LayerMemoryAdd + 3*(x + y*LCD_Width) + 2 ) = color>>16 ;
        }
        else if(Direction == Direction_V)	//垂直方向
        {
            *(__IO uint16_t*)( LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) ) = color ;
            *(__IO uint8_t*)( LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) +2) = color>>16 ;
        }
    }

    /*----------------------- 16位色 ARGB1555、RGB565或者ARGB4444 模式 ----------------------*/
    else
    {
        if (Direction == Direction_H) //水平方向
        {
            *(__IO uint16_t*)( LayerMemoryAdd + 2*(x + y*LCD_Width) ) = color ;
        }
        else if(Direction == Direction_V)	//垂直方向
        {
            *(__IO uint16_t*)( LayerMemoryAdd + 2*((LCD_Height - x - 1)*LCD_Width + y) ) = color ;
        }
    }
}

uint32_t DisplayLTDC::LCD_ReadPoint(uint16_t x, uint16_t y)
{
    uint32_t color = 0;

    /*----------------------- 32位色 ARGB8888 模式 ----------------------*/
    if( ColorMode == LTDC_PIXEL_FORMAT_ARGB8888 )
    {
        if (Direction == Direction_H) //水平方向
        {
            color = *(__IO uint32_t*)( LayerMemoryAdd + 4*(x + y*LCD_Width) );
        }
        else if(Direction == Direction_V)	//垂直方向
        {
            color = *(__IO uint32_t*)( LayerMemoryAdd + 4*((LCD_Height - x - 1)*LCD_Width + y) );
        }
    }

    /*----------------------------- 24位色 RGB888 模式 -------------------------*/
    else if ( ColorMode == LTDC_PIXEL_FORMAT_RGB888 )
    {
        if (Direction == Direction_H) //水平方向
        {
            color = *(__IO uint32_t*)( LayerMemoryAdd + 3*(x + y*LCD_Width) ) &0x00ffffff;
        }
        else if(Direction == Direction_V)	//垂直方向
        {
            color = *(__IO uint32_t*)( LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) ) &0x00ffffff;
        }
    }

    /*----------------------- 16位色 ARGB1555、RGB565或者ARGB4444 模式 ----------------------*/
    else
    {
        if (Direction == Direction_H) //水平方向
        {
            color = *(__IO uint16_t*)( LayerMemoryAdd + 2*(x + y*LCD_Width) );
        }
        else if(Direction == Direction_V)	//垂直方向
        {
            color = *(__IO uint16_t*)( LayerMemoryAdd + 2*((LCD_Height - x - 1)*LCD_Width + y) );
        }
    }
    return color;
}

void DisplayLTDC::LCD_DisplayChar(uint16_t x, uint16_t y, uint8_t c)
{
    uint16_t  index = 0, counter = 0;		// 计数变量
    uint8_t   disChar;		//存储字符的地址
    uint16_t  Xaddress = x; //水平坐标

    c = c - 32; 	// 计算ASCII字符的偏移

    for(index = 0; index < Fonts->Sizes; index++)
    {
        disChar = Fonts->pTable[c*Fonts->Sizes + index]; //获取字符的模值
        for(counter = 0; counter < 8; counter++)
        {
            if(disChar & 0x01)
            {
                LCD_DrawPoint(Xaddress,y,Color);		//当前模值不为0时，使用画笔色绘点
            }
            else
            {
                LCD_DrawPoint(Xaddress,y,BackColor);	//否则使用背景色绘制点
            }
            disChar >>= 1;
            Xaddress++;  //水平坐标自加

            if( (Xaddress - x)==Fonts->Width ) //如果水平坐标达到了字符宽度，则退出当前循环
            {   //进入下一行的绘制
                Xaddress = x;
                y++;
                break;
            }
        }
    }
}

void DisplayLTDC::LCD_DisplayString(uint16_t x, uint16_t y, const char *p)
{
    while ((x < LCD_Width) && (*p != 0))	//判断显示坐标是否超出显示区域并且字符是否为空字符
    {
        LCD_DisplayChar( x,y,*p);
        x += Fonts->Width; //显示下一个字符
        p++;	//取下一个字符地址
    }
}

void DisplayLTDC::LCD_SetTextFont(pFONT *fonts)
{
    CHFonts = fonts;		// 设置中文字体
    switch(fonts->Width )
    {
    case 12:
        Fonts = &Font12;
        break;	// 设置ASCII字符的字体为 1206
    case 16:
        Fonts = &Font16;
        break;	// 设置ASCII字符的字体为 1608
    case 20:
        Fonts = &Font20;
        break;	// 设置ASCII字符的字体为 2010
    case 24:
        Fonts = &Font24;
        break;	// 设置ASCII字符的字体为 2412
    case 32:
        Fonts = &Font32;
        break;	// 设置ASCII字符的字体为 3216
    default:
        break;
    }
}

void DisplayLTDC::LCD_DrawImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *pImage)
{
    uint8_t   disChar;	//字模的值
    uint16_t  Xaddress = x; //水平坐标
    uint16_t  i=0,j=0,m=0;

    for(i = 0; i <height; i++)
    {
        for(j = 0; j <(float)width/8; j++)
        {
            disChar = *pImage;

            for(m = 0; m < 8; m++)
            {
                if(disChar & 0x01)
                {
                    LCD_DrawPoint(Xaddress,y,Color);	//当前模值不为0时，使用画笔色绘点
                }
                else
                {
                    LCD_DrawPoint(Xaddress,y,BackColor);	//否则使用背景色绘制点
                }
                disChar >>= 1;
                Xaddress++;  //水平坐标自加

                if( (Xaddress - x)==width ) //如果水平坐标达到了字符宽度，则退出当前循环
                {   //进入下一行的绘制
                    Xaddress = x;
                    y++;
                    break;
                }
            }
            pImage++;
        }
    }
}

#define ABS(X)  ((X) > 0 ? (X) : -(X))

void DisplayLTDC::LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
            yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
            curpixel = 0;

    deltax = ABS(x2 - x1);        /* The difference between the x's */
    deltay = ABS(y2 - y1);        /* The difference between the y's */
    x = x1;                       /* Start x off at the first pixel */
    y = y1;                       /* Start y off at the first pixel */

    if (x2 >= x1)                 /* The x-values are increasing */
    {
        xinc1 = 1;
        xinc2 = 1;
    }
    else                          /* The x-values are decreasing */
    {
        xinc1 = -1;
        xinc2 = -1;
    }

    if (y2 >= y1)                 /* The y-values are increasing */
    {
        yinc1 = 1;
        yinc2 = 1;
    }
    else                          /* The y-values are decreasing */
    {
        yinc1 = -1;
        yinc2 = -1;
    }

    if (deltax >= deltay)         /* There is at least one x-value for every y-value */
    {
        xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
        yinc2 = 0;                  /* Don't change the y for every iteration */
        den = deltax;
        num = deltax / 2;
        numadd = deltay;
        numpixels = deltax;         /* There are more x-values than y-values */
    }
    else                          /* There is at least one y-value for every x-value */
    {
        xinc2 = 0;                  /* Don't change the x for every iteration */
        yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
        den = deltay;
        num = deltay / 2;
        numadd = deltax;
        numpixels = deltay;         /* There are more y-values than x-values */
    }
    for (curpixel = 0; curpixel <= numpixels; curpixel++)
    {
        LCD_DrawPoint(x,y,Color);             /* Draw the current pixel */
        num += numadd;              /* Increase the numerator by the top of the fraction */
        if (num >= den)             /* Check if numerator >= denominator */
        {
            num -= den;               /* Calculate the new numerator value */
            x += xinc1;               /* Change the x as appropriate */
            y += yinc1;               /* Change the y as appropriate */
        }
        x += xinc2;                 /* Change the x as appropriate */
        y += yinc2;                 /* Change the y as appropriate */
    }
}

void DisplayLTDC::LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    /* draw horizontal lines */
    LCD_DrawLine(x, y, x+width, y);
    LCD_DrawLine(x, y+height, x+width, y+height);

    /* draw vertical lines */
    LCD_DrawLine(x, y, x, y+height);
    LCD_DrawLine(x+width, y, x+width, y+height);
}

void DisplayLTDC::LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t r)
{
    int Xadd = -r, Yadd = 0, err = 2-2*r, e2;
    do {

        LCD_DrawPoint(x-Xadd,y+Yadd,Color);
        LCD_DrawPoint(x+Xadd,y+Yadd,Color);
        LCD_DrawPoint(x+Xadd,y-Yadd,Color);
        LCD_DrawPoint(x-Xadd,y-Yadd,Color);

        e2 = err;
        if (e2 <= Yadd) {
            err += ++Yadd*2+1;
            if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
        }
        if (e2 > Xadd) err += ++Xadd*2+1;
    }
    while (Xadd <= 0);
}

void DisplayLTDC::LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    DMA2D->CR	  &=	~(DMA2D_CR_START);				//	停止DMA2D
    DMA2D->CR		=	DMA2D_R2M;							//	寄存器到SDRAM
    DMA2D->OPFCCR	=	ColorMode;						//	设置颜色格式
    DMA2D->OCOLR	=	Color;							//	颜色

    if(Direction == Direction_H)  //横屏填充
    {
        DMA2D->OOR		=	LCD_Width - width;				//	设置行偏移
        DMA2D->OMAR		=	LayerMemoryAdd + BytesPerPixel*(LCD_Width * y + x);	// 地址;
        DMA2D->NLR		=	(width<<16)|(height);			//	设定长度和宽度
    }
    else	//竖屏填充
    {
        DMA2D->OOR		=	LCD_Width - height;		//	设置行偏移
        DMA2D->OMAR		=	LayerMemoryAdd + BytesPerPixel*((LCD_Height - x - 1 - width)*LCD_Width + y);	// 地址
        DMA2D->NLR		=	(width)|(height<<16);	//	设定长度和宽度
    }

    DMA2D->CR	  |=	DMA2D_CR_START;					//	启动DMA2D

    while (DMA2D->CR & DMA2D_CR_START) ;			//	等待传输完成
}

void DisplayLTDC::LCD_FillCircle(uint16_t x, uint16_t y, uint16_t r)
{
    int32_t  D;    /* Decision Variable */
    uint32_t  CurX;/* Current X Value */
    uint32_t  CurY;/* Current Y Value */

    D = 3 - (r << 1);

    CurX = 0;
    CurY = r;

    while (CurX <= CurY)
    {
        if(CurY > 0)
        {
            LCD_DrawLine(x - CurX, y - CurY,x - CurX,y - CurY + 2*CurY);
            LCD_DrawLine(x + CurX, y - CurY,x + CurX,y - CurY + 2*CurY);
        }

        if(CurX > 0)
        {
            LCD_DrawLine(x - CurY, y - CurX,x - CurY,y - CurX + 2*CurX);
            LCD_DrawLine(x + CurY, y - CurX,x + CurY,y - CurX + 2*CurX);
        }
        if (D < 0)
        {
            D += (CurX << 2) + 6;
        }
        else
        {
            D += ((CurX - CurY) << 2) + 10;
            CurY--;
        }
        CurX++;
    }

    LCD_DrawCircle(x, y, r);
}

void DisplayLTDC::LCD_ShowNumMode(uint8_t mode)
{
    ShowNum_Mode = mode;
}

extern "C" void LTDC_IRQHandler(void)
{
    HAL_LTDC_IRQHandler(&hltdc);
}
