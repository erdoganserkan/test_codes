#ifndef AZP_FAST_H
#define AZP_FAST_H

#include <stdint.h>

#define AZP_FAST_TNUM_STR_MAX_LEN			8
#define AZP_FAST_NUMSTR_COUNT				9
#define AZP_FAST_NUMSTRs_UPX				0
#define AZP_FAST_NUMSTRs_UPY				SB_FULL_TOP_SIZE_Y
#define AZP_FAST_NUMSTRs_SIZEX				(GLCD_X_SIZE/AZP_FAST_NUMSTR_COUNT)
#define AZP_FAST_NUMSTRs_SIZEY				(GLCD_Y_SIZE/12)

#define AZP_FAST_BOX_COUNT				AZP_FAST_NUMSTR_COUNT
#define AZP_FAST_BOXES_UPX				0
#define AZP_FAST_BOXES_UPY				(AZP_FAST_NUMSTRs_UPY + AZP_FAST_NUMSTRs_SIZEY)
#define AZP_FAST_BOX_SIZEX				AZP_FAST_NUMSTRs_SIZEX
#define AZP_FAST_BOX_SIZEY				AZP_FAST_NUMSTRs_SIZEY

#define AZP_FAST_CURSOR_COUNT			AZP_FAST_BOX_COUNT
#define AZP_FAST_CURSOR_UPX				0
#define AZP_FAST_CURSOR_UPY				(AZP_FAST_BOXES_UPY + AZP_FAST_BOX_SIZEY)
#define AZP_FAST_CURSOR_SIZEX			AZP_FAST_BOX_SIZEX
#define AZP_FAST_CURSOR_SIZEY			(AZP_FAST_BOX_SIZEY / 2)

#define AZP_FAST_TARGETNUM_UPX			0
#define AZP_FAST_TARGETNUM_UPY			(AZP_FAST_CURSOR_UPY + AZP_FAST_CURSOR_SIZEY)
#define AZP_FAST_TARGETNUM_DOWNX		GLCD_X_SIZE
#define AZP_FAST_TARGETNUM_DOWNY		(GLCD_Y_SIZE - AZP_FAST_BOX_SIZEY)

#define AZP_FAST_TOLX					4
#define AZP_FAST_TOLY					4

extern uint8_t AZP_Fast(void);	

#endif

