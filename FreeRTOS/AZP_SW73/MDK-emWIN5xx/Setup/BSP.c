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
File    : BSP.c
Purpose : BSP for IAR LPC1788-SK
--------  END-OF-HEADER  ---------------------------------------------
*/

#define BSP_C

#include <string.h>
#include "LPC177x_8x.h"         // Device specific header file, contains CMSIS
#include "system_LPC177x_8x.h"  // Device specific header file, contains CMSIS
#include "lpc177x_8x_gpio.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_clkpwr.h"
#include "lpc177x_8x_adc.h"
#include "lpc177x_8x_timer.h"
#include "lpc177x_8x_pwm.h"
#include "SDRAM_K4S561632C_32M_16BIT.h"
#include <FreeRTOS.h>
#include <event_groups.h>
#include <task.h>
#include "emc_nor.h"
#include "WM.h"
#include "BSP.h"
#include "AppCommon.h"
#include "debug_frmwrk.h"
#include "monitor.h"
#include "Serial.h"
#include "AppSettings.h"
#include "led.h"
#include "APlay.h"
#include "TouchPanel.h"
#include "Analog.h"
#include "Battery.h"
#include "CoreFirFilter.h"

/*********************************************************************
*
*       Shared Resources
*/
extern EventGroupHandle_t xRendezvousBits;	// Used for Task Rendezvous //

volatile uint8_t lcd_backlight_reduce_state = TRUE;	//:TODO: USe mutex instead of this variable for safety // 
static int32_t last_key_event_time_ms = 0;
static uint8_t lcd_bcklight_reduced = FALSE;
static uint16_t lcd_backlight_process_cnt = 0;
static uint8_t lcd_bcklight_real_level = BRIGHTNESS_MAX;

void BSP_Init_IRQ_Priorities(void) 
{
	uint32_t priorityGroup;                                     /* Variables to store priority group and priority */
	uint32_t priority;

	__disable_irq();
	NVIC_SetPriorityGrouping(APP_INTERRRUPT_PRIORITY_GROUPING);
	priorityGroup =  NVIC_GetPriorityGrouping();

	// Set EXTI interrupt priority //  
    priority = NVIC_EncodePriority(priorityGroup, EXTI_IRQ_GROUP_PRIO, EXTI_IRQ_SUB_PRIO); 
	#if(POWERMCU_DEV_BOARD == USED_HW)
	    NVIC_SetPriority(EINT0_IRQn, priority);  
	#elif(UMD_DETECTOR_BOARD == USED_HW)
		NVIC_SetPriority(EINT1_IRQn, priority);  
	#endif
	
	// Set NMI, MM, BusFault, UsageFault, SVCall, DebugMonitor, PendSV interrupts priority //  
	priority = NVIC_EncodePriority(priorityGroup, CORTEXM3_SYS_IRQ_GROUP_PRIO, CORTEXM3_SYS_IRQ_SUB_PRIO); 
	NVIC_SetPriority(NonMaskableInt_IRQn, priority);  
	NVIC_SetPriority(MemoryManagement_IRQn, priority);  
	NVIC_SetPriority(BusFault_IRQn, priority);  
	NVIC_SetPriority(UsageFault_IRQn, priority);  
	NVIC_SetPriority(SVCall_IRQn, priority);  
	NVIC_SetPriority(DebugMonitor_IRQn, priority);  
	NVIC_SetPriority(PendSV_IRQn, priority);  

	// Set I2S interrupt priority //  
	priority = NVIC_EncodePriority(priorityGroup, I2S_IRQ_GROUP_PRIO, I2S_IRQ_SUB_PRIO); 
	NVIC_SetPriority(I2S_IRQn, priority);  

	// Set I2C interrupt priority //  
	priority = NVIC_EncodePriority(priorityGroup, I2C_IRQ_GROUP_PRIO, I2C_IRQ_SUB_PRIO); 
	NVIC_SetPriority(I2C0_IRQn, priority);  // Change if AUDIO_CODEC i2c channel changed // 

	// Set LCD interrupt priority //  
	priority = NVIC_EncodePriority(priorityGroup, LCD_IRQ_GROUP_PRIO, LCD_IRQ_SUB_PRIO); 
	NVIC_SetPriority(LCD_IRQn, priority);  
	
	// Set DMA interrupt priority //  
	priority = NVIC_EncodePriority(priorityGroup, DMA_IRQ_GROUP_PRIO, DMA_IRQ_SUB_PRIO); 
	NVIC_SetPriority(DMA_IRQn, priority); 
	
	// Set SYSTICK interrupt priority //  
	priority = NVIC_EncodePriority(priorityGroup, SYSTICK_IRQ_GROUP_PRIO, SYSTICK_IRQ_SUB_PRIO); 
	NVIC_SetPriority(SysTick_IRQn, priority);  

	// Set ADC interrupt priority //  
	priority = NVIC_EncodePriority(priorityGroup, ADC_IRQ_GROUP_PRIO, ADC_IRQ_SUB_PRIO); 
	NVIC_SetPriority(ADC_IRQn, priority);  
	
	// Set TIMER2 interrupt priority //  
	priority = NVIC_EncodePriority(priorityGroup, TMR2_IRQ_GROUP_PRIO, TMR2_IRQ_SUB_PRIO); 
	NVIC_SetPriority(TIMER2_IRQn, priority);  

	// Set TIMER1 interrupt priority //  
	priority = NVIC_EncodePriority(priorityGroup, TMR1_IRQ_GROUP_PRIO, TMR1_IRQ_SUB_PRIO); 
	NVIC_SetPriority(TIMER1_IRQn, priority);  

	// Set UART1 interrupt priority //  
	priority = NVIC_EncodePriority(priorityGroup, UART1_IRQ_GROUP_PRIO, UART1_IRQ_SUB_PRIO); 
	NVIC_SetPriority(UART1_IRQn, priority);  

	// Set MCI interrupt priority //  
	priority = NVIC_EncodePriority(priorityGroup, MCI_IRQ_GROUP_PRIO, MCI_IRQ_SUB_PRIO); 
	NVIC_SetPriority(MCI_IRQn, priority);  

	// Set USB interrupt priority //  
	priority = NVIC_EncodePriority(priorityGroup, USB_IRQ_GROUP_PRIO, USB_IRQ_SUB_PRIO); 
	NVIC_SetPriority(USB_IRQn, priority);

	__enable_irq();
}


static void __task_BSP_processKS(void *pvArg);
void KS_GpioInit(void);
#if( POWERMCU_DEV_BOARD == USED_HW )
	// UP, DOWN, LEFT, RIGHT, OK, ESC // 
	static const int16_t ports[KEY_COUNT] = {00, 00, 00, 00, 00, 00};
	static const int16_t pins[KEY_COUNT]  = {17, 18, 19, 20, 21, 22};
#elif( UMD_DETECTOR_BOARD == USED_HW )
	// OLD LAYOUT // 
	// UP:DEPTH(P2.15), 
	// DOWN:OTO(P4.27), 
	// LEFT:MINUS(P2.14), 
	// RIGHT:PLUS(P0.17), 
	// OK:CONFIRM(P0.20), 
	// ESC:MENU(P0.18) // 
	// static const int16_t ports[KEY_COUNT] = {2, 04, 02, 0, 0, 0};
	// static const int16_t pins[KEY_COUNT] = {15, 27, 14, 17, 20, 18};

	// LATEST PRODUCT LAYOUT // 
	// UP:PLUS:RIGHT		(P0.17), #
	// DOWN:MINUS:LEFT	(P2.14), #
	// OTO					(P4.27), #
	// OK					(P0.20), #
	// MENU				(P0.18), #
	
	static const int16_t ports[KEY_COUNT] = {0, 02, 04, 00, 00};
	static const int16_t pins[KEY_COUNT] = {17, 14, 27, 20, 18};

	#else
	#error "NOT IMPLEMENED"
#endif	
static xTaskHandle KSTaskHandle;
static void __nor_mem_init(void);

static volatile uint32_t ADCValues[ADC_CH_COUNT] = {0};
static volatile uint8_t EnabledADCCHs = 0;
static volatile uint8_t completed_ch_bits = 0;
volatile uint8_t ADC_done = FALSE;

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
volatile eKeyType pkey_buf[PKEY_STACK_COUNT];
volatile uint8_t readcnt, writecnt;

inline void set_lcd_bcklight_reduce_state(uint8_t new_state) {
	uint8_t current_setting = \
		(APP_IS_DETECTOR == APP_GetValue(ST_DEV_TYPE)) ? APP_GetValue(ST_BRIGHT) : APP_GetValue(ST_AT_BRIGTH);
	if(new_state) {
		last_key_event_time_ms = GUI_X_GetTime();
		lcd_bcklight_reduced = FALSE;
		lcd_backlight_process_cnt = 0;
	}
	if(lcd_bcklight_real_level != current_setting) {
		lcd_bcklight_real_level = current_setting;
		BSP_PWMSet(0, BSP_PWM_LCD, current_setting);
	}			
	lcd_backlight_reduce_state = new_state;
}

void KS_init(void) 
{
	KS_GpioInit();
	memset((void *)pkey_buf,0,sizeof(pkey_buf));
	readcnt = writecnt = 0;
	xTaskCreate( __task_BSP_processKS , "KSTask" , KS_TASK_STACK_SIZE , NULL , KS_TASK_PRIORITY , &KSTaskHandle );
}

static void PushPkey(eKeyType newpkey)
{
	pkey_buf[writecnt++] = newpkey;
	if(writecnt == PKEY_STACK_COUNT)
		writecnt = 0;
}

eKeyType PullPkey(void)
{
	eKeyType ret = NO_KEY_PRESSED;
	uint8_t writecntL;
	vTaskSuspend(KSTaskHandle);
	writecntL = writecnt;
	vTaskResume(KSTaskHandle);
	if(writecntL != readcnt) {
		ret = pkey_buf[readcnt++];
		if(readcnt == PKEY_STACK_COUNT)
			readcnt = 0;
	}
	return ret;
}

#if(1)
void  Delay (uint32_t nCount) 
{
  for(; nCount != 0; nCount--);
}

uint8_t get_kp(void) 
{
	uint8_t pressed[KEY_COUNT];
	memset(pressed, 0, sizeof(pressed));
	for(volatile uint8_t indy=0 ; indy<100 ; indy++) {
		for(volatile uint8_t key=(KEY_COUNT-1) ;; key--) {
			if( !(GPIO_ReadValue(ports[key])&(1<<pins[key]))) 
				pressed[key]++;
			if(0==key) 
				break;
		}
	}

	for(volatile uint8_t key=(KEY_COUNT-1) ;; key--) {
		if(pressed[key] >= 50)
			return key;
		if(0 == key)
			break;
	}
	return NO_KEY_PRESSED;
}
	 
static void __task_BSP_processKS(void *pvArg)
{
	static uint8_t key_state[KEY_COUNT] = {FALSE, FALSE, FALSE, FALSE, FALSE};
	static uint8_t key_cnt[KEY_COUNT];
	volatile uint8_t indx;
	
	// Wait for GUI & Other tasks to be ready before generating key_press events // 
	if(NULL != xRendezvousBits)
		xEventGroupSync( xRendezvousBits, (0x1<<UMD_TASK_KS), ALL_TASK_BITs, portMAX_DELAY );
	else
		while(TODO_ON_ERR);

	last_key_event_time_ms = GUI_X_GetTime();
	
	// Update Key Counters //
	for(indx=KEY_FIRST ;;) {
		if(FALSE == key_state[indx]) {
			// Key is NOT pressed before //
			if( !(GPIO_ReadValue(ports[indx])&(1<<pins[indx]))) {   
				// Key is pressed NOW // 
				if((MIN_KEYPRESS_MS/KEY_SCAN_MS) == (++key_cnt[indx])) {
					key_cnt[indx] = (MIN_KEY_WAIT_MS/KEY_SCAN_MS);
					key_state[indx] = TRUE;
					// PushPkey((eKeyType)indx);
					GUI_StoreKeyMsg(indx + KEY_EVENT_OFFSET_CH, 1);	// Invoke emWin event handling mechanism // 

					if(lcd_backlight_reduce_state) {
						// Record last keypress event time and ecover back LCD Backlight reduction, if it is done // 
						last_key_event_time_ms = GUI_X_GetTime();
						if(TRUE == lcd_bcklight_reduced) {
							uint8_t current_setting = \
								(APP_IS_DETECTOR == APP_GetValue(ST_DEV_TYPE)) ? APP_GetValue(ST_BRIGHT) : APP_GetValue(ST_AT_BRIGTH);
							lcd_bcklight_reduced = FALSE;
							lcd_backlight_process_cnt = 0;
							lcd_bcklight_real_level = current_setting;
							BSP_PWMSet(0, BSP_PWM_LCD, current_setting);
						}
					}
				}
			}
			else {
				// Key is NOT pressed NOW // 
				if(0 != key_cnt[indx])
					key_cnt[indx]--;
			}
		}
		else {
			// Key is pressed before and we are waiting //
			if(0 == (--key_cnt[indx]))
				key_state[indx] = FALSE;
		}
		if(KEY_LAST == indx) {
			indx = KEY_FIRST;
			vTaskDelay(KEY_SCAN_MS * (configTICK_RATE_HZ/1000));

			if(lcd_backlight_reduce_state) {
				// Process lastkeypress event time, decide if it is required to reduce lcd-backlight or not // 
				if((LCD_BACKLIGHT_REDUCE_PROCESS_INTERVAL_MS / KEY_SCAN_MS) < (++lcd_backlight_process_cnt)) {
					lcd_backlight_process_cnt = 0;
					
					int32_t diff_s = (GUI_X_GetTime() - last_key_event_time_ms)/1000;
					if(BACKLIGHT_DECREASE_START_INTERVAL_S <= diff_s) {
						lcd_bcklight_reduced = TRUE;
						uint16_t decrease_level = \
							(((diff_s - BACKLIGHT_DECREASE_START_INTERVAL_S) / BACKLIGHT_DECREASE_TIME_STEP_S) + 1 ) * \
								BACKLIGHT_DECREASE_LEVEL_STEP;
						uint8_t current_setting = \
							(APP_IS_DETECTOR == APP_GetValue(ST_DEV_TYPE)) ? APP_GetValue(ST_BRIGHT) : APP_GetValue(ST_AT_BRIGTH);
						if(decrease_level > (current_setting - BRIGHTNESS_MIN)) {
							if(BRIGHTNESS_MIN != lcd_bcklight_real_level) {
								lcd_bcklight_real_level = BRIGHTNESS_MIN;
								BSP_PWMSet(0, BSP_PWM_LCD, lcd_bcklight_real_level);
							}
						}
						else {
							if((current_setting - decrease_level) != lcd_bcklight_real_level) {
								lcd_bcklight_real_level = current_setting - decrease_level;
								BSP_PWMSet(0, BSP_PWM_LCD, lcd_bcklight_real_level);
							}
						}
					}	
				}
			}
		}
		else 
			indx++;
	}
}
/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : Configure GPIO Pin
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void KS_GpioInit(void)
{ 
	volatile uint8_t indx;
	for(indx=0 ; indx<KEY_COUNT ; indx++) {
		PINSEL_ConfigPin(ports[indx], pins[indx], 0);	   /* Set pin mode as GPIO */
		GPIO_SetDir(ports[indx], (1<<(pins[indx])), 0);    /* Input Mode */
	}	
}
#endif

void BSP_GPIOInit(void) {
	GPIO_Init();
	//:TODO:// 
	#if(UMD_DETECTOR_BOARD == USED_HW)
		// LCD BCKLGHT CONTROL : P2.1 
		GPIO_SetDir(2, (1<<1), 1);    /* Output Mode */
		GPIO_ClearValue(2, (1<<1));  
		// Analog Power Enable Port & Pin DeInit // 
		PINSEL_ConfigPin(ANALOG_POWER_EN_PORT, ANALOG_POWER_EN_PIN, 0);
		GPIO_SetDir(ANALOG_POWER_EN_PORT, (1<<ANALOG_POWER_EN_PIN), 1);
		GPIO_ClearValue(ANALOG_POWER_EN_PORT, (1<<ANALOG_POWER_EN_PIN));	// Disable Analog power // 

		// Uart transmit First Byte Indicator PIN P2.25 //
		PINSEL_ConfigPin(UART_TRANSMIT_FIRST_BYTE_PORT, UART_TRANSMIT_FIRST_BYTE_PIN, 0);
		GPIO_SetDir(UART_TRANSMIT_FIRST_BYTE_PORT, (1<<UART_TRANSMIT_FIRST_BYTE_PIN), 1);
		GPIO_ClearValue(UART_TRANSMIT_FIRST_BYTE_PORT, (1<<UART_TRANSMIT_FIRST_BYTE_PIN));	// Default Case is LOW // 		
			
		// UMD AT FS REGULATOR #SHTDOWN PIN: P3.17
		#if (TRUE == USE_PWM2_CH_AS_AT_VOLTAGE_DISABLE)
			GPIO_SetDir(FS_PWM_REG_SHTDWN_PORT, (1<<FS_PWM_REG_SHTDWN_PIN), 1);   // Set GPIO as output mode //  
			GPIO_SetValue(FS_PWM_REG_SHTDWN_PORT, (1<<FS_PWM_REG_SHTDWN_PIN));  	// HIGH:Power of fs pwm regulator, LOW:PowerON // 
			PINSEL_ConfigPin (FS_PWM_REG_SHTDWN_PORT, FS_PWM_REG_SHTDWN_PIN, 0); // GPIO CONFIG //
		#endif
		// ChargerState3 	: Input : P0.4
		// ChargerState2 	: Input : P0.5
		// USB2 Power 		: Output : P0.12
		// Battery Level ADC : Analog IN : ADC0_IN1 : P0.24
		// Battery Temp ADC : Analog IN : ADC0_IN2 : P0.25
		// Field Scanner 15V enable : Output : P1.0
		// Field Scanner 10V enable : Output : P1.1
	#endif
}


static uint32_t __pwm_freq_hz = 0;
static uint8_t pwm_duty[BSP_PWM_COUNT] = {0,50};	// LCD is @ %50 and others are @ %0 DUTY level // 
const uint8_t pwm_ch[BSP_PWM_COUNT] = {1,3};

void BSP_set_pwm_freq_hz(uint8_t pwm_hw, uint32_t freq_hz) 
{
	PWM_TIMERCFG_Type PWMCfgDat;
	if(1 < pwm_hw) {
		while(STALLE_ON_ERR);
		pwm_hw = 0;
	}
	if(MAX_PWM_FREQ_HZ < freq_hz) {
		while(TODO_ON_ERR);
		freq_hz = MAX_PWM_FREQ_HZ;
	}
	if(MIN_PWM_FREQ_HZ > freq_hz) {
		while(TODO_ON_ERR);
		freq_hz = MIN_PWM_FREQ_HZ;
	}

	__pwm_freq_hz = freq_hz;	// Set global resource with new value // 

	// CH0 is controlling frequency, because only ch-0 will be configured to RESET on MATCH //  
	uint32_t peripheral_clk = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);
	PWM_MatchUpdate(pwm_hw, 0, peripheral_clk/freq_hz, PWM_MATCH_UPDATE_NOW);	

	// Because of frequency is changed update all channels' match valuesfor proper operation // 
	volatile uint8_t indx;
	for(indx=0 ; indx<BSP_PWM_COUNT ; indx++) {
		PWM_MatchUpdate(pwm_hw, pwm_ch[indx], ((peripheral_clk/freq_hz)*pwm_duty[indx])/100, PWM_MATCH_UPDATE_NOW);	// CH0 is controlling frequency // 				
	}
}

static void __BSP_PWMInit(void) {
	#if(UMD_DETECTOR_BOARD == USED_HW)
		volatile uint8_t indx;

		uint8_t pwmChannel, channelVal;
		PWM_MATCHCFG_Type PWMMatchCfgDat;
		PWM_TIMERCFG_Type PWMCfgDat;
	
		/* PWM block section -------------------------------------------- */
		/* Initialize PWM peripheral, timer mode
		* PWM prescale value = 1 (absolute value - tick value) */
		PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;	// Prescaler is means of timer ticks // 
		PWMCfgDat.PrescaleValue = 1;
		PWM_Init(0, PWM_MODE_TIMER, (void *) &PWMCfgDat);
	
		// Initialize PWM pin connect
		// UMD AT PWM0.1 : P3.16 
		GPIO_SetDir(3, (1<<16), 1);    /* Output Mode */
		GPIO_SetValue(3, (1<<16));  
		PINSEL_ConfigPin (3, 16, 2);//PWM0.1

		// LCD PWM : PWM0.3 : P1.5
		GPIO_SetDir(1, (1<<5), 1);    /* Output Mode */
		GPIO_SetValue(1, (1<<5));  
		PINSEL_ConfigPin (1, 5, 3);//PWM0.3
	
		/* Set match value for PWM match channels, update immediately */
		BSP_set_pwm_freq_hz(0, DEFAULT_PWM_FREQ_HZ);
		
		/* PWM Timer/Counter will be reset when channel 0 matching
		* no interrupt when match
		* no stop when match */
		PWMMatchCfgDat.IntOnMatch = DISABLE;
		PWMMatchCfgDat.MatchChannel = 0;
		PWMMatchCfgDat.ResetOnMatch = ENABLE;
		PWMMatchCfgDat.StopOnMatch = DISABLE;
		PWM_ConfigMatch(0, &PWMMatchCfgDat);
	
		/* Configure each PWM channel: --------------------------------------------- */
		/* - Single edge
		* - PWM Duty on each PWM channel determined by
		* the match on channel 0 to the match of that match channel.
		* Example: PWM Duty on PWM channel 1 determined by
		* the match on channel 0 to the match of match channel 1.
		*/
	
		/* Configure PWM channel edge option
		* Note: PWM Channel 1 is in single mode as default state and
		* can not be changed to double edge mode */
		/* Output for channel 0 is NA */
		for(indx=0 ; indx<BSP_PWM_COUNT ; indx++) {
			if((0 != pwm_ch[indx]) && (1 != pwm_ch[indx]))
				PWM_ChannelConfig(0, pwm_ch[indx], PWM_CHANNEL_SINGLE_EDGE);
		}
	
		/* Configure match value for each match channel */
		// Channel 0 is DONE before and so skip it // 
		for(indx=0 ; indx<BSP_PWM_COUNT ; indx++) {
			/* Configure match option */
			PWMMatchCfgDat.IntOnMatch = DISABLE;
			PWMMatchCfgDat.MatchChannel = pwm_ch[indx];
			PWMMatchCfgDat.ResetOnMatch = DISABLE;
			PWMMatchCfgDat.StopOnMatch = DISABLE;
			PWM_ConfigMatch(0, &PWMMatchCfgDat);

			if(BSP_PWM_LCD == indx) {
				BSP_PWMSet(0, pwm_ch[indx], 50);	// Default duty for LCD is %50 until app-settings read // 
			} else {
				BSP_PWMSet(0, pwm_ch[indx], 0);	// Default duty for FS Probes is %0 until AT_Menu() opened //
			}
			PWM_ChannelCmd(0, pwm_ch[indx], ENABLE);
		}
	
		/* Reset and Start counter */
		PWM_ResetCounter(0);
	
		PWM_CounterCmd(0, ENABLE);
	
		/* Start PWM now */
		PWM_Cmd(0, ENABLE);
	#endif
}

void BSP_PWM_start(uint8_t pwm_hw, ePWMCHs pwm_ch)
{
	/* Enable PWM Channel Output */
	PWM_ChannelCmd(pwm_hw, pwm_ch, ENABLE);

}

void BSP_PWM_stop(uint8_t pwm_hw, ePWMCHs pwm_ch)
{
	/* Enable PWM Channel Output */
	PWM_ChannelCmd(pwm_hw, pwm_ch, DISABLE);
}

void BSP_PWMSet(uint8_t pwm_hw, ePWMCHs ch, uint8_t pwm_duty) {
	if(pwm_duty > 100)
		pwm_duty = 100;
	if(BSP_PWM_COUNT <= ch)
		ch = BSP_PWM_COUNT-1;
	uint32_t match0_mat_val = LPC_PWM0->MR0;
	PWM_MatchUpdate(pwm_hw, pwm_ch[ch], (match0_mat_val*pwm_duty)/100, PWM_MATCH_UPDATE_NOW); 
}

/*********************************************************************
*
*       BSP_Init()
*/
void BSP_Init(void) {
	float const jd_hp_filter_coefs[JACK_DETECT_FILTER_TAP_COUNT] = {0};
	InitFilterCFs(jd_hp_filter_coefs); 
	BSP_GPIOInit();
	__BSP_PWMInit();	// LCD backlight pwm channel started // 
	#if(TRUE == TS_TASK_STATE)
		TP_Init();
	#endif
	init_TPs(2);
}

/*********************************************************************
*
*       LED switching routines
*       LEDs are switched on with low level on port lines
*/
void BSP_SetLED(int Index) {
  BSP_USE_PARA(Index);
  //
  // No dedicated LEDs available
  //
}

void BSP_ClrLED(int Index) {
  BSP_USE_PARA(Index);
  //
  // No dedicated LEDs available
  //
}

void BSP_ToggleLED(int Index) {
  BSP_USE_PARA(Index);
  //
  // No dedicated LEDs available
  //
}


/*********************************************************************
*
*       _DelayMs()
*
* Function description:
*   Starts a timer and waits for the given delay in ms.
*/
void BSP_DelayMS(U32 ms) {
	#if(EMWIN_GUI_TMR_NUM == 0)
		LPC_TIM0->TCR = 0x02;  // Reset timer
		LPC_TIM0->PR  = 0x00;  // Set prescaler to zero
		LPC_TIM0->MR0 = ms * (SystemCoreClock / (LPC_SC->PCLKSEL & 0x1F) / 1000 - 1);
		LPC_TIM0->IR  = 0xFF;  // Reset all interrrupts
		LPC_TIM0->MCR = 0x04;  // Stop timer on match
		LPC_TIM0->TCR = 0x01;  // Start timer
		//
		// Wait until delay time has elapsed
		//
		//:TODO: 
			// Enable timer interrupt //
			// Sleep (Interruptible)
		while (LPC_TIM0->TCR & 1);
	#else
	  #err "GUI WAIT TIMER NOT DEFINED"
		// Bu opsiyon hardware_fault ' a sebep oluyor // 
		Delay(0xffff);
	#endif
}


/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief		ADC interrupt handler sub-routine
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void ADC_IRQHandler(void)
{	
	volatile uint8_t ch;
	uint8_t const ch_mask[ADC_CH_COUNT] =  {
		0x1<<0, 0x1<<1, 0x1<<2, 0x1<<3, 0x1<<4, 0x1<<5, 0x1<<6, 0x1<<7
	};
	
	for(ch = 0 ; ch<ADC_CH_COUNT ; ch++) {
		if((EnabledADCCHs & ch_mask[ch]) && (ADC_ChannelGetStatus(LPC_ADC, ch, 1))) {
			completed_ch_bits |= ch_mask[ch];
			ADCValues[ch] =  ADC_ChannelGetData(LPC_ADC, ch);	
		}
	}
	if(completed_ch_bits == EnabledADCCHs) {
		completed_ch_bits = 0;
		ADC_done = TRUE;
		BSP_ADCStop();
	}
	
}

void BSP_ADCInit(uint8_t adc_ch_bits) 
{
	volatile uint8_t indx;
	
	EnabledADCCHs = adc_ch_bits;
	/*
	 * Init ADC pin that currently is being used on the board
	 */
	 for(indx=0 ; indx<ADC_CH_COUNT ; indx++) {
	 	if(adc_ch_bits & (0x1<<indx)) {
			switch(indx) {
				case 4:	// Potentiometer level read @ developmet board // 
					PINSEL_ConfigPin (1, 30, 1);
					PINSEL_SetAnalogPinMode(1,30,ENABLE);
					break;
				case 2:	// HP Jack State Detect // 
					PINSEL_ConfigPin (0, 25, 1);
					PINSEL_SetAnalogPinMode(0, 25, ENABLE);
					break;
				case 1:	// Used for battery read // 
					PINSEL_ConfigPin (0, 24, 1);
					PINSEL_SetAnalogPinMode(0, 24, ENABLE);
					break;
				default:
					while(1);// Undefined CH // 
					break;
			}
	 	}
	 }
	/* Configuration for ADC:
	 *  ADC conversion rate = 400KHz
	 */
	ADC_Init(LPC_ADC, ADC_MAX_SPEED_HZ);

	for(indx=0 ; indx<ADC_CH_COUNT ; indx++) {
		if(adc_ch_bits & (0x1<<indx)) {
			switch(indx) {
				case 4:
					ADC_IntConfig(LPC_ADC, ADC_ADINTEN4, ENABLE);
					ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_4, ENABLE);
					break;
				case 2: 
					ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, ENABLE);
					ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);
						break;
				case 1:	
					ADC_IntConfig(LPC_ADC, ADC_ADINTEN1, ENABLE);
					ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
					break;
				default:
					break;
			}
		}
	}

	// IRQ Priority has been set in BSP_Init_IRQ_Priorities() function // 
	/* preemption = 1, sub-priority = 1 */
	// NVIC_SetPriority(ADC_IRQn, 1);
}

void BSP_ADCStart (void) 
{
	/* Start Burst Mode ADC Convertion */
	ADC_BurstCmd(LPC_ADC,ENABLE);
	ADC_done = FALSE;
	/* Enable ADC in NVIC */
	NVIC_EnableIRQ(ADC_IRQn);
}

void BSP_ADCStop (void) 
{
	/* Start Burst Mode ADC Convertion */
	ADC_BurstCmd(LPC_ADC, DISABLE);
	/* Enable ADC in NVIC */
	NVIC_DisableIRQ(ADC_IRQn);
}

inline uint32_t BSP_GET_RAW_ADC(uint8_t adc_ch)
{
	if(8 > adc_ch)
		return ADCValues[adc_ch];
	else
		return 0;
}

/*********************************************************************
*
*       __low_level_init()
*
*       Initialize memory controller, clock generation and pll
*
*       Has to be modified, if another CPU clock frequency should be
*       used. This function is called during startup and
*       has to return 1 to perform segment initialization
*/
#ifdef __cplusplus
extern "C" {
#endif
int __low_level_init(void);  // Avoid "no ptototype" warning
#ifdef __cplusplus
  }
#endif

void __nor_mem_init(void) 
{
	NOR_IDTypeDef NOR_ID;
	/* Initialize the FSMC NOR Flash Interface */
	EMC_NOR_Init();
	/* Set the NOR read modes */
	EMC_NOR_ReturnToReadMode();

	EMC_NOR_ReadID(&NOR_ID);

	EMC_NOR_ReturnToReadMode();

	if( ( NOR_ID.Manufacturer_Code == 0x0001 ) && (NOR_ID.Device_Code1 == 0x227E) &&
		( NOR_ID.Device_Code2 == 0x2210 ) && ( NOR_ID.Device_Code3 == 0x2200 ) ) {
		// DEBUGM("\r\nNOR Type = S29GL064N 8M NOR Flash  ---------  "); DEBUG functions not initialized YET // 
	}
	else {
		// DEBUGM("\r\nNOR Type = Unknow  ---------  ");	// Debugging functions not initialized YET // 
	}
}

int __low_level_init(void) 
{
	SystemCoreClockUpdate();  // Ensure, the SystemCoreClock is set
	BSP_Init_IRQ_Priorities();

	//
	// Init SDRAM, NAND- and NOR-flash
	//
	//_EMC_Init();
	SDRAM_32M_16BIT_Init();
	//__nor_mem_init();
	//  Perform other initialization here, if required
	// Do Peripheral Initialization // 
	BSP_Init();

	return 1;
}

/****** End of File *************************************************/
