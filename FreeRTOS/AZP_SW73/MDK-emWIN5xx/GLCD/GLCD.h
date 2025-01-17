/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			GLCD.h
** Descriptions:		None
**
**------------------------------------------------------------------------------------------------------
** Created by:			AVRman
** Created date:		2012-6-20
** Version:				1.0
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:	
** Version:
** Descriptions:		
********************************************************************************************************/

#ifndef __GLCD_H 
#define __GLCD_H

/* Includes ------------------------------------------------------------------*/
#include "lpc_types.h"
#include "SDRAM_K4S561632C_32M_16BIT.h"
#include "AppCommon.h"
#include "LCDConf.h"

/* Private define ------------------------------------------------------------*/
/* LCD_VRAM_BASE_ADDR: (Appcommon.h icinde tanimli) frame buffer' larin bulundugu adres oluyor.  */
#define LCD_CURSOR_BASE_ADDR 	((uint32_t)0x20088800)

/* what LCD type do you want to use */
#if (USED_LCD == LCD_4_3_TFT)
	#define GLCD_X_SIZE        480	         /* LCD Horizontal length */
	#define GLCD_Y_SIZE        272		     /* LCD Vertical length */
	#define PPL                GLCD_X_SIZE   /* Pixels per line */

	#define HSW                20             /* HSYNC pulse width */
	#define HFP                2             /* Horizontal front porch */
	#define HBP                2            /* Horizontal back porch */

	#define LPP                GLCD_Y_SIZE   /* Lines per panel */
	#define VSW                15             /* VSYNC pulse width */
	#define VFP                2             /* Vertical front porch */
	#define VBP                2             /* Vertical back porch */

	#define ACB                1             /* AC bias frequency (not used) */
	#define IVS				   			1             /* ? Invert VSYNC */
	#define IHS                1             /* ? Invert HSYNC */
	#define IPC                0             /* ? Invert panel clock */
	#define CPL                PPL           /* Clock per line (identical to pixels per line) */
	#define BPP                6             /* Bits per pixel b110 = 16 bpp 5:6:5 mode */
	#define BGR                1             /* Swap red and blue */
	#define LCDTFT             1             /* LCD TFT panel */
	#define OPT_CLK            (8*1000000)      /* Optimal clock rate (Hz) between 1-8.22 MHz */
#elif (USED_LCD == LCD_4_3_TFT_UMD)
	#define GLCD_X_SIZE        480	         /* LCD Horizontal length */
	#define GLCD_Y_SIZE        272		     /* LCD Vertical length */
	#define PPL                GLCD_X_SIZE   /* Pixels per line */

	#define HSW                40             /* HSYNC pulse width */
	#define HFP                2             /* Horizontal front porch */
	#define HBP                2            /* Horizontal back porch */

	#define LPP                GLCD_Y_SIZE   /* Lines per panel */
	#define VSW                10             /* VSYNC pulse width */
	#define VFP                2             /* Vertical front porch */
	#define VBP                10             /* Vertical back porch */

	#define ACB                1             /* AC bias frequency (not used) */
	#define IVS				   				0             /* ? Invert VSYNC */
	#define IHS                1             /* ? Invert HSYNC */
	#define IPC                0             /* ? Invert panel clock */
	#define CPL                PPL           /* Clock per line (identical to pixels per line) */
	#define BPP                6             /* Bits per pixel b110 = 16 bpp 5:6:5 mode */
	#define BGR                1             /* Swap red and blue */
	#define LCDTFT             1             /* LCD TFT panel */
	#define OPT_CLK            (12*1000000)      /* Optimal clock rate (Hz) between 1-8.22 MHz */

#elif (USED_LCD == LCD_4_3_TFT_PRO)
	/* nothıng :)) */
#elif (USED_LCD == LCD_5_0_TFT)
	#define GLCD_X_SIZE        480	         /* LCD Horizontal length */
	#define GLCD_Y_SIZE        272		     /* LCD Vertical length */
	#define PPL                GLCD_X_SIZE   /* Pixels per line */

	#define HSW                30             /* HSYNC pulse width */
	#define HFP                3             /* Horizontal front porch */
	#define HBP                3            /* Horizontal back porch */

	#define LPP                GLCD_Y_SIZE   /* Lines per panel */
	#define VSW                20             /* VSYNC pulse width */
	#define VFP                3             /* Vertical front porch */
	#define VBP                3             /* Vertical back porch */

	#define ACB                1             /* AC bias frequency (not used) */
	#define IVS				   1             /* ? Invert VSYNC */
	#define IHS                1             /* ? Invert HSYNC */
	#define IPC                0             /* ? Invert panel clock */
	#define CPL                PPL           /* Clock per line (identical to pixels per line) */
	#define BPP                6             /* Bits per pixel b110 = 16 bpp 5:6:5 mode */
	#define BGR                1             /* Swap red and blue */
	#define LCDTFT             1             /* LCD TFT panel */
	#define OPT_CLK            (8*1000000)      /* Optimal clock rate (Hz) between 1-8.22 MHz */

#elif (USED_LCD == LCD_7_0_TFT)
	#define GLCD_X_SIZE        800	         /* LCD Horizontal length */
	#define GLCD_Y_SIZE        480           /* LCD Vertical length */
	#define PPL                GLCD_X_SIZE   /* Pixels per line */

	#define HSW                1             /* HSYNC pulse width, can be 1-48 */
	#define HFP                40            /* Horizontal front porch, MUST be 40 */
	#define HBP                88            /* Horizontal back porch, MUST be 88 */

	#define LPP                GLCD_Y_SIZE   /* Lines per panel */
	#define VSW                3             /* VSYNC pulse width, MSUT be 3 */
	#define VFP                3             /* Vertical front porch, 1 or 3 is rational */
	#define VBP                3             /* Vertical back porch, 1 or 3 is rational */

	#define ACB                1             /* AC bias frequency (not used) */
	#define IVS				   1             /* ? Invert VSYNC */
	#define IHS                1             /* ? Invert HSYNC */
	#define IPC                0             /* ? Invert panel clock */
	#define CPL                PPL           /* Clock per line (identical to pixels per line) */
	#define BPP                6             /* Bits per pixel b110 = 16 bpp 5:6:5 mode */
	#define BGR                1             /* Swap red and blue */
	#define LCDTFT             1             /* LCD TFT panel */
	/* #define OPT_CLK            (28.5*1000000) */     /* OLD VAL : Optimal clock rate (Hz) between 1-8.22 MHz */ 
	#define OPT_CLK            (33*1000000UL)      /* Optimal clock rate (Hz) between 29 - 33MHz (Nominal) - 38 MHz */
#else
	#error "Invalid LCD!"
#endif


#define BCD_BIT            26        /* Bypass pixel clock divider */

#define GLCD_PWR_ENA_DIS_DLY  10000

/* LCD color */
#define Black           0x0000      /*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0x780F      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Green           0x07E0      /*   0, 255,   0 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0xF81F      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255, 0   */
#define White           0xFFFF      /* 255, 255, 255 */

/* Private function prototypes -----------------------------------------------*/
void GLCD_Cursor_Config(int config);
void GLCD_Cursor_Enable(int cursor);
void GLCD_Cursor_Disable(void);
void GLCD_Move_Cursor(int x, int y);
void GLCD_Copy_Cursor (const uint32_t *pCursor, int cursor, int size);
void GLCD_Init (void);
void GLCD_Clear(uint32_t buf, uint16_t color);
void GLCD_DrawLine( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color );

/******************************************************************************
* Function Name  : GLCD_SetPixel_16bpp
* Description    : Set Point
* Input          : - Xpos: 
*                  - Ypos: 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
/* if declare to __inline will more faster */
static __inline void GLCD_SetPixel_16bpp_0(uint16_t Xpos, uint16_t Ypos, uint16_t color)
{
	//uint16_t *fbp0;
	((uint16_t *)LCD_VRAM_BASE_ADDR + (XSIZE_PHYS * YSIZE_PHYS*0))[ Ypos * GLCD_X_SIZE + Xpos ] = color;	
	//fbp0 = ((uint16_t *)LCD_VRAM_BASE_ADDR + (XSIZE_PHYS * YSIZE_PHYS*0));
}

static __inline void GLCD_SetPixel_16bpp_1(uint16_t Xpos, uint16_t Ypos, uint16_t color)
{
	//uint16_t *fbp1;
	((uint16_t *)LCD_VRAM_BASE_ADDR + (XSIZE_PHYS * YSIZE_PHYS*1))[ Ypos * GLCD_X_SIZE + Xpos ] = color;	
	//fbp1 = ((uint16_t *)LCD_VRAM_BASE_ADDR + (XSIZE_PHYS * YSIZE_PHYS*1));
}

static __inline void GLCD_SetPixel_16bpp_2(uint16_t Xpos, uint16_t Ypos, uint16_t color)
{
	//uint16_t *fbp2;
	((uint16_t *)LCD_VRAM_BASE_ADDR + (XSIZE_PHYS * YSIZE_PHYS*2))[ Ypos * GLCD_X_SIZE + Xpos ] = color;	
	//fbp2 = ((uint16_t *)LCD_VRAM_BASE_ADDR + (XSIZE_PHYS * YSIZE_PHYS*2));
}

static __inline void GLCD_SetPixel_16bpp_3(uint16_t Xpos, uint16_t Ypos, uint16_t color)
{
	//uint16_t *fbp3;
	((uint16_t *)LCD_VRAM_BASE_ADDR + (XSIZE_PHYS * YSIZE_PHYS*3))[ Ypos * GLCD_X_SIZE + Xpos ] = color;	
	//fbp3 = ((uint16_t *)LCD_VRAM_BASE_ADDR + (XSIZE_PHYS * YSIZE_PHYS*3));
}

#endif

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
