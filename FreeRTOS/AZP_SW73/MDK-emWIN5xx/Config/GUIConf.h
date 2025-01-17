/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2010     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File        : GUIConf.h
Purpose     : Configuration of available features and default values
----------------------------------------------------------------------
*/

#ifndef GUICONF_H
#define GUICONF_H

/*********************************************************************
*
*       Multi layer/display support
*/
#define GUI_NUM_LAYERS            (16) // Maximum number of available layers

/*********************************************************************
*
*       Multi tasking support
*/
#define GUI_OS                    (0)  // Compile with multitasking support

/*********************************************************************
*
*       Configuration of available packages
*/
#ifndef   GUI_SUPPORT_TOUCH
  #define GUI_SUPPORT_TOUCH       (0)  // Support touchscreen
									  // SRK-NOTE: If you enable this be carefull becase LPC_TIM1 is shared with 
  										// "clock generation for detector" and "touch screen detection" //
#endif
#define GUI_SUPPORT_MOUSE         (0)  // Support a mouse
#define GUI_SUPPORT_UNICODE       (1)  // Support mixed ASCII/UNICODE strings
#define GUI_WINSUPPORT            (1)  // Window manager package available
#define GUI_SUPPORT_MEMDEV        (1)  // Memory devices available
#define GUI_SUPPORT_AA            (1)  // Anti aliasing available
#define WM_SUPPORT_STATIC_MEMDEV  (1)  // Static memory devices available
#define MULTIBUFF_USE_ISR  				(1)

/*********************************************************************
*
*       Default background color
*/
//#define GUI_DEFAULT_BKCOLOR          GUI_BLACK
/*********************************************************************
*
*       Default font
*/
#define GUI_DEFAULT_FONT          APP_24B_FONT
#define GUI_MEMCPY(pSrc, pDest, NumBytes) GUI__memcpy(pSrc, pDest, NumBytes)

#endif  /* Avoid multiple inclusion */



/*************************** End of file ****************************/
