#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lpc_types.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_gpio.h"
#include "lpc177x_8x_ssp.h"
#include "lpc177x_8x_mci.h"
#include <FreeRTOS.h>
#include <event_groups.h>
#include <task.h>
#include <semphr.h>
#include "UMDShared.h"
#include "GLCD.h"
#include "WM.h"
#include "BSP.h"
#include "AppCommon.h"
#include "debug_frmwrk.h"
#include "monitor.h"
#include "Serial.h"
#include "AppSettings.h"
#include "GuiConsole.h"
#include "Dac.h"
#include "APlay.h"
#include "Analog.h"
#include "ATMenu.h"

volatile AppState_t AppStatus = {
	.long_gb_done = FALSE,
	.short_gb_done = FALSE,
	.gb_type_required = GB_TYPE_LONG,
	.search_before_gb = STD_SEARCH,
	.detector_hs_state = DETECTOR_INIT_NOTDONE
};

#if MCI_DMA_ENABLED
/******************************************************************************
**  DMA Handler
******************************************************************************/
void DMA_IRQHandler (void)
{
	if(WAVE_DAC_DMA_Handler())
		return;
	if(APLAY_DAC_DMA_Handler())
		return;
	if(MCI_DMA_IRQHandler())
		return;
}
#else
	#error "where is DMA IRQ Handler???"
#endif

void Print_Mem_Data(uint8_t force)
{
	static uint16_t cnt = 0;
	if((TRUE == force) || (cnt++ == 250)) {
		cnt = 0;
		int32_t bytes = GUI_ALLOC_GetNumFreeBytes();
		DEBUGM("GUI Free Bytes KB(%u) MB(%u)\n", bytes/1024, bytes/(1024u*1024u));
	}	
}

uint32_t get_block_crc32(uint8_t *block, uint32_t size)
{
	const uint32_t odd = 0x5a;
	const uint32_t even = 0x78;
	uint32_t retval=0;

	uint8_t *u8ptr = (uint8_t *)&retval;	
	for(volatile uint32_t indx=0,indy=0 ; indx<size ; indx++, indy++, block++) {
		indy &= 0x3;
		if(indx & 0x1)	// if it is odd // 
				u8ptr[indy] ^= (odd & (*block));
			else	// if it is even // 
				u8ptr[indy] ^= (even & (*block));
	}
				
	return retval;
}

uint16_t get_block_crc16(uint8_t *block, uint32_t size)
{
	const uint32_t odd = 0x5a;
	const uint32_t even = 0x78;
	uint16_t retval=0;

	uint8_t *u8ptr = (uint8_t *)&retval;	
	for(volatile uint32_t indx=0,indy=0 ; indx<size ; indx++, indy++, block++) {
		indy &= 0x1;
		if(indx & 0x1)	// if it is odd // 
				u8ptr[indy] ^= (odd & (*block));
			else	// if it is even // 
				u8ptr[indy] ^= (even & (*block));
	}
				
	return retval;
}


void App_waitMS(uint32_t wMS)
{
	if(taskSCHEDULER_RUNNING == xTaskGetSchedulerState()) {	// use freeRTOS based wait // 
		vTaskDelay(wMS * (configTICK_RATE_HZ/1000));
	} else {	// Use manual systick based wait // 
		#if(1)
			volatile uint32_t indx;
			while(wMS--) 
				for(indx=0x1FFF; indx ; indx--) 
					__NOP;
		#else
			BSP_DelayMS(wMS);		// NOT TESTED, This function is NOT thread-safe // 
		#endif
	}
}

void init_TPs(uint8_t tp_num) 
{
	switch(tp_num) {
		case 2:
			// TP2: P0.1 @ UMD Detector board, output // 
			PINSEL_ConfigPin(MUTE_SPK_PORT_NUM, MUTE_SPK_PIN_NUM, 0);	 /* GPIO */
			GPIO_SetDir(MUTE_SPK_PORT_NUM, 1<<MUTE_SPK_PIN_NUM, 1);		// Configured as output //
			break;			
		case 4:
			// TP4: P2.12 @ UMD Detector board, output // 
			PINSEL_ConfigPin(TP4_PORT_NUM, TP4_PIN_NUM, 0);	 /* GPIO */
			GPIO_SetDir(TP4_PORT_NUM, 1<<TP4_PIN_NUM, 1);		// Configured as output //
			SET_TP(TP4_PORT_NUM, TP4_PIN_NUM, 0);
			SET_TP(TP4_PORT_NUM, TP4_PIN_NUM, 1);
			SET_TP(TP4_PORT_NUM, TP4_PIN_NUM, 0);
			break;
		case 5:
			// TP5 : P0.22 @UMD Dedector board, input //
			PINSEL_ConfigPin(TP5_PORT_NUM, TP5_PIN_NUM, 0);	 /* GPIO */
			GPIO_SetDir(TP5_PORT_NUM, 1<<TP5_PIN_NUM, 0);		// Configured as input and pin is pulled-up internally // 
			// The function below is DISABLING PULL-UP feature of pin, interesting // 
			//if(PINSEL_RET_OK != PINSEL_SetPinMode(TP5_PORT_NUM, TP5_PIN_NUM, IOCON_MODE_PULLUP))
				//while(1);	
			break;
		default:
			break;
	}	
}

void App_SetHWTypeStates(uint8_t detector_state, uint8_t fs_state)
{
	static uint8_t clock_init_done = FALSE;
	if(TRUE == detector_state) {
		#if(TRUE == CLOCK_GENERATION_STATE)
			clock_init_done = TRUE;
			Analog_init();
			init_clock_generation();
			start_clock_generation();
		#endif
	} else {
		#if(TRUE == CLOCK_GENERATION_STATE)
			if(FALSE == clock_init_done) {
				clock_init_done = TRUE;
				init_clock_generation();
			}
			// power-off detector sub-circuit // 
			Analog_Deinit();
			stop_clock_generation();
		#endif
	}	

	if(TRUE == fs_state) {	
		// set output voltage // 
		uint16_t at_range = APP_GetValue(ST_AT_DIST_RANGE);
		uint8_t at_range_voltages[AT_DISTANCE_RANGE_COUNT] = {
			AT_VOLTAGE_P2P_9V, 
			AT_VOLTAGE_P2P_10V, 
			AT_VOLTAGE_P2P_15V,
			AT_VOLTAGE_P2P_18V
		};
		AT_set_voltage_p2p(at_range_voltages[at_range]);
		App_waitMS(100);
		// Set PWM freq // 
		//:TODO: Change DUTY cycle with reverse-proportion to Vpp value // 
		// 10KHz - %25 //
		// 300Hz - %50 // 
		if(AT_MAN_FREQ_SELECTED == APP_GetValue(ST_AT_MAN_AUTO_SEL)) {
			AT_enable_PWM(APP_GetValue(ST_AT_MAN_FREQ), 30);
		} else {
			extern uint16_t const at_auto_freqs[AT_AUTO_FREQ_COUNT];
			int8_t auto_freq_indx = APP_GetValue(ST_AT_AUTO_FREQ_INDX);
			AT_enable_PWM(at_auto_freqs[auto_freq_indx], 30);
		}
		App_waitMS(100);
		// Enable regulator // 
		#if (TRUE == USE_PWM2_CH_AS_AT_VOLTAGE_DISABLE)
			GPIO_ClearValue(AT_VOLTAGE_OUTPUT_SHTDOWN_PORT, (1<<AT_VOLTAGE_OUTPUT_SHTDOWN_PIN));  	// LOW:PowerON FS Regulator // 
		#endif
		App_waitMS(100);
	}
	else {
		// Init and enable at PWM based frequency generation // 
		AT_disable_PWM();
		App_waitMS(100);
		AT_set_voltage_p2p(AT_MIN_VOLTAGE_P2P_V);
		App_waitMS(100);
	}
}

void start_dac_playing(uint32_t freq_multipler)
{
	extern uint8_t active_page;	// defined in GUIDEMO_Start.c // 
	uint8_t active_search_type;
	switch(active_page) {
		case AZP_ALL_METAL_SCR:
		case AZP_DISC_SCR:
		case AZP_FAST_SCR:
		case AZP_MENU_SCR:
		case A5P_ALL_METAL_SCR:
		case A5P_DISC_SCR:
		case A5P_AUTO_SCR:
		case A5P_MENU_SCR:
			active_search_type = AUTOMATIC_SEARCH_TYPE;
			break;
		default:
			active_search_type = STD_SEARCH_TYPE;
			ERRM("UNEXPECTED active_page(%u)\n", active_page);
			break;
	}
	WAVE_Generator_init(active_search_type);
	WAVE_Generator_start(UMD_GAUGE_MIN, MODULATOR_DEF_FREQ_HZ*freq_multipler, DAC_DEFAULT_AMP);	// Start DAC Wave for minimum GAUGE // 
}
