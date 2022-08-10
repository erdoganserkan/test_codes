#ifndef GB_H
#define GB_H

#include <stdint.h>

#define GB_ANIM		FALSE
#define ACTIVE_GB_POINTER		TRUE

//------------------------------------//
// SCREEN OBJECT LOCATION COORDINATEs //
//------------------------------------//
#define ACTIVE_POINTER_DEF_MAX		(+100*CLOCK_GENERATE_PHASE_FRACTION)
#define ACTIVE_POINTER_MAX_SAFE		(+15*CLOCK_GENERATE_PHASE_FRACTION)
#define ACTIVE_POINTER_DEF_MIN		(-100*CLOCK_GENERATE_PHASE_FRACTION)
#define ACTIVE_POINTER_MIN_SAFE		(-15*CLOCK_GENERATE_PHASE_FRACTION)
#define ACTIVE_POINER_GUI_RANGE_MULTIPLER	(3)

#define GBBACK_SIZEX	480
#define GBBACK_SIZEY	272
#define GBBACK_LEFTX	0
#define GBBACK_LEFTY	0
#define GB_SCR_NAME_STR_LEFTUPX		105
#define GB_SCR_NAME_STR_LEFTUPY		15

#define HAND_WINDOW_LEFTX		0
#define HAND_WINDOW_LEFTY		69
#define HAND_WINDOW_SIZEX		91
#define HAND_WINDOW_SIZEY		118

#define HAND_ON_OFF_ANIM_MS	250
#define GB_ACTIVE_ANIM_MS		300

// {109, 88}, {333, 140} //
#define HOLDER_LEFTX		109
#define HOLDER_LEFTY		88
#define HOLDER_SIZEX		224
#define HOLDER_SIZEY		52

#define POINTER_MIN_POSX	117
#define POINTER_MAX_POSX	281
#define POINTER_POSY		94
#define POINTER_SIZEX		45
#define POINTER_SIZEY		38
#define POINTER_STEP_COUNT		12
#define POINTER_STEP_VAL			((POINTER_MAX_POSX - POINTER_MIN_POSX)/POINTER_STEP_COUNT)

#define ACTIVE_POINTER_MIN_POSX			5
#define ACTIVE_POINTER_MAX_POSX			(HOLDER_SIZEX -5)
#define ACTIVE_POINTER_MIN_POSY			(10)
#define ACTIVE_POINTER_MAX_POSY			(HOLDER_SIZEY-10)
#define ACTIVE_POINTER_MIDDLE_WIDTH		(10)

#define EXPLANATION_STR_LEFTUPX		95
#define EXPLANATION_STR_LEFTUPY		145
#define EXPLANATION_STR_RIGHTDOWNX		350
#define EXPLANATION_STR_RIGHTDOWNY		200

// {349, 56} {477, 205} //
#define COIL_LEFTX		349
#define COIL_MIN_LEFTY		56
#define COIL_MAX_LEFTY		120
#define COIL_SIZEX		128
#define COIL_SIZEY		85
#define COIL_STEP_COUNT			8
#define COIL_STEP_VAL				((COIL_MAX_LEFTY - COIL_MIN_LEFTY)/COIL_STEP_COUNT)

#define GID_STR_LEFTX		131
#define GID_STR_LEFTY		213

// GB RESULT POPUP DEFINITIONs //
#define GB_RESULT_POPUP_ANIM_MS		250
#define GB_RESULT_POPUP_LEFTX	96
#define GB_RESULT_POPUP_LEFTY	54
#define GB_RESULT_POPUP_SIZEX	408
#define GB_RESULT_POPUP_SIZEY	231

#define GB_RESULT_POPUP_ARMOR_LEFTX	296
#define GB_RESULT_POPUP_ARMOR_LEFTY	10
#define GB_RESULT_POPUP_ARMOR_SIZEX	80
#define GB_RESULT_POPUP_ARMOR_SIZEY	99

#define GB_RESULT_POPUP_SHAND_LEFTX	330
#define GB_RESULT_POPUP_SHAND_LEFTY	140
#define GB_RESULT_POPUP_SHAND_SIZEX	80
#define GB_RESULT_POPUP_SHAND_SIZEY	99

#define GB_RESULT_POPUP_STR_LEFTX		20
#define GB_RESULT_POPUP_STR_LEFTY		15
#define GB_RESULT_POPUP_STR2_LEFTY	90
#define GB_RESULT_POPUP_STR_RIGHTX	290
#define GB_RESULT_POPUP_STR_RIGHTY	180

typedef enum
{
	GB_WAIT_to_START	 = 0,	// Waiting for user intervention // 
	GB_PROCESSING,					// Detector is processing //
	GB_COMPLETED,						// Failed or Succeed //
	
	GB_STATE_COUNT
} eGBState;

extern uint8_t GB(void);

#endif
