#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "debug_frmwrk.h"
#include "AppCommon.h"
#include "UMDShared.h"
#include "Strings.h"
#include "AppSettings.h"
#include "monitor.h"
#include "Serial.h"
#include "StatusBar.h"
#include "GuiConsole.h"
#include "FlashDriver.h"
#include "BSP.h"
#include "ACodec.h"
#include "ATManuelFreq.h"
#include "ATAutoFreq.h"
#include "ATMenu.h"
#include "ATDistance.h"
#include "ATLang.h"

uint16_t const at_auto_freqs[AT_AUTO_FREQ_COUNT] = {
	AT_LONG_GOLD_FREQ_HZ, AT_SHORT_GOLD_FREQ_HZ, AT_WATER_FREQ_HZ, AT_CAVITY_FREQ_HZ, AT_ALLMETAL_FREQ_HZ
}; 
static uint8_t App_calcCRC8(uint8_t *ptr, uint8_t size);
static uAppStore AppSettings;	// Global settings store object located in RAM // 
const sSettingType aSettings[ST_COUNT] = { 
	{	// Version // 
		ST_VERS,
		200,
		101,
		1
	},
	{	// Battery //
		ST_BAT,
		100,
		0,
		1
	},
	{	// Sensitivity // 
		ST_SENS,
		SENSITIVITY_MAX,
		SENSITIVITY_MIN,
		5
	},
	{	// Volume // 
		ST_VOL,
		VOLUME_MAX,
		VOLUME_MIN,
		5
	},
	{	// Brightness //
		ST_BRIGHT,
		BRIGHTNESS_MAX,
		BRIGHTNESS_MIN,
		5
	},
	{	// Mineralization // 
		ST_GROUND_ID,	// indx // 
		GROUND_ID_MAX,
		GROUND_ID_MIN,
		1
	},
	{	// Coil Type // 
		ST_COIL_T,
		COIL_TYPE_MAX,
		COIL_TYPE_MIN,
		1
	},
	{	// Language //
		ST_LANG,
		LANG_MAX,
		LANG_MIN,
		LANG_EN
	},
	{	// Ferros Enable / Disable // 
		ST_FERROs,
		1,
		0,
		1
	},
	{	// LeftCH Duty //
		ST_LEFT_DUTY,
		100,
		0,
		1
	},
	{	// Right CH Duty //
		ST_RIGHT_DUTY,
		100,
		0,
		1
	},
	{	// PWM Freq HZ // 
		ST_PWM_FREQ_HZ,
		MAX_PWM_FREQ_HZ,
		MIN_PWM_FREQ_HZ,
		1
	},
	{	// field Scanner Output Voltage //
		ST_VOLTAGE,
		AT_MAX_VOLTAGE_P2P_V,
		AT_MIN_VOLTAGE_P2P_V,
		1
	},
	{	// field Scanner: Brightness //
		ST_AT_BRIGTH,
		BRIGHTNESS_MAX,
		BRIGHTNESS_MIN,
		5
	},
	{	// field Scanner: Language //
		ST_AT_LANG,
		AT_LANG_MAX,
		AT_LANG_MIN,
		AT_LANG_ENG
	},
	{	// Field Scanner: Volume // 
		ST_AT_VOL,
		VOLUME_MAX,
		VOLUME_MIN,
		5
	},
	{	// Field Scanner: Manuel Frequency // 
		ST_AT_MAN_FREQ,
		AT_MAN_FREQ_MAX,
		AT_MAN_FREQ_MIN,
		AT_ALLMETAL_FREQ_HZ
	},
	{	// Field Scanner: Auto Frequency // 
		ST_AT_AUTO_FREQ_INDX,
		AT_AUTO_FREQ_MAX,
		AT_AUTO_FREQ_MIN,
		AT_ALLMETAL_FREQ_HZ
	},
	{	// Field Scanner: Manuel/Auto Frequency Selection // 
		ST_AT_MAN_AUTO_SEL,
		1,
		0,
		AT_AUTO_FREQ_SELECTED
	},
	{	// Field Scanner: Distance // 
		ST_AT_DIST_RANGE,
		AT_DISTANCE_RANGE_COUNT,
		0,
		0
	},
	{	// Field Scanner: Search Type //
		ST_AT_SEARCH_TYPE,
		AT_SEARCH_TYPE_MAX,
		AT_SEARCH_TYPE_MIN,
		AT_SEARCH_SONAR
	},
	{	// DEVICE TYPE // 
		ST_DEV_TYPE,
		APP_MAIN_TYPE_COUNT-1,
		0,
		APP_IS_DETECTOR
	},
	
	{	// AZP FAST STATE // 
		ST_AZP_FAST,
		0xFFFF,		// max 
		0,			// min 
		0xFFFF		// default 
	}
};
typedef __packed struct {
	uint8_t state;
	uAppStore set;
} settings_spi_store;


void APP_StoreSettings(uAppStore *pSettings, uint8_t storeNVM)
{
	uint32_t crc32;
	// update global Appsettings object(located in RAM) with pSettings parameter //
	if(NULL == pSettings)
		return;
	else {
		if(pSettings != (&AppSettings))
			AppSettings = *pSettings;
	}
	// Update Non-Volatile memory // 
	#if(RES_LOAD_SPI_FLASH == RES_LOAD_SOURCE)
		if(TRUE == storeNVM) {
			settings_spi_store set_store = {
				.state = SPI_FLASH_APP_SETTING_INIT_DONE,
				.set = AppSettings
			};
			// Clear settings' sector on flash // 
			SPI_Flash_Erase( SEC_MAX, SEC_MAX );
			// write settngs store object // 
			df_write_seek(SEC_MAX*SEC_SIZE);	
			DEBUGM("AppSettings Stored @ADR(%u), SIZE(%u)\n", df_write_get_pos(), sizeof(set_store));
			df_write((uint8_t *)&set_store, sizeof(set_store)); // write header //
			// write settings store-object's CRC // 
			crc32 = get_block_crc32((uint8_t *)&set_store, sizeof(set_store));
			df_write_seek(SEC_MAX*SEC_SIZE + sizeof(set_store));	
			DEBUGM("AppSettings-CRC Stored @ADR(%u)\n", df_write_get_pos());
			df_write((uint8_t *)&crc32, sizeof(crc32)); // write crc32 // 
		}
	#elif(RES_LOAD_SDMMC == RES_LOAD_SOURCE)
		//:TODO: 
	#else
		#error "" 
	#endif
}

void App_ReloadSettings(uAppStore *pSettings, eAppReload_te reload_src)
{
	switch(reload_src)
	{
		case RELOAD_FROM_RAM:
			// AppSettings is already the source of RAM values, nothing todo // 
			break;
		case RELOAD_FROM_NVMEM: {
			uint8_t state;
			volatile uint8_t indx;
			settings_spi_store set_store;
			uint32_t read32, crc32;
			uint8_t failed = FALSE;
			#if(RES_LOAD_SOURCE == RES_LOAD_SDMMC)
				App_ReloadSettings(NULL, RELOAD_FROM_SAFE_VALUEs);
			#elif(RES_LOAD_SOURCE == RES_LOAD_SPI_FLASH)
				spi_flash_ResloadInit();
				// Read settings store object // 
				df_read_seek(SEC_MAX * SEC_SIZE);
				df_read((uint8_t *)&set_store, sizeof(set_store));
				// Read settings store object's CRC //
				crc32 = get_block_crc32((uint8_t *)&set_store, sizeof(set_store));
				df_read((uint8_t *)&read32, sizeof(read32));
				if(read32 != crc32) {
					ERRM("AppSettings CRC Check FAILED; read(0x%X), expected(0x%X)\n", read32, crc32);
					failed = TRUE;
				} else {
					DEBUGM("AppSettings CRC Check OK, crc32(0x%X)\n", crc32);
					if(SPI_FLASH_APP_SETTING_INIT_DONE != set_store.state) {
						ERRM("AppSettings initialization is NOT DONE BEFORE\n");
						failed = TRUE;
					} else {
						if(set_store.set.str.Version < APP_SETTING_STRUCT_VERSION) {
							INFOM("SETTING STRUCT VERSION OLD(%u < %u) is STORED, RENEWING IT\n", \
								set_store.set.str.Version, APP_SETTING_STRUCT_VERSION);
							AppSettings_StoreDefaults();
						} else
							AppSettings = set_store.set;
						char const *setting_strs[]={"VERS", "BAT", "SENS", "VOL", "BRIGHT", "GID", "COIL-T", \
							"LANG", "FERRO", "LDUTY", "RDUTY", "PWMFREQ", "VOLT", "AT-BRIGHT", "AT-LANG", 
							"AT-VOL", "AT-MAN", "AT-AUTO", "AT-MA-SEL", "AT-DIST", "AT-SEARCH", "ST-DEV-T", "CRC"};
						for(indx=0 ; indx<ST_COUNT ; indx++) {
							DEBUGM("AppSettings(%u:%s) Val(%u)\n", indx, setting_strs[indx], AppSettings.Settings[indx]);
						}
					}
				}
			#else
				#error "NOT IMPLEMENTED"
			#endif
			if(TRUE == failed)				
				AppSettings_StoreDefaults();
		}
		break;
		case RELOAD_FROM_SAFE_VALUEs: {
			uint16_t bat_level;
			// Set common settings // 
			AppSettings.str.Version = APP_SETTING_STRUCT_VERSION;
			SB_setget_component(SB_BATTERY, FALSE, &bat_level);
			AppSettings.str.Battery = bat_level;
			// Set detector settings // 
			AppSettings.str.Sensitivity = 60;
			AppSettings.str.Volume = 50;
			AppSettings.str.Brightness = 30;
			AppSettings.str.Mineralization = DEFAULT_GROUND_ID;
			AppSettings.str.CoilType = SMALL_COIL;
			AppSettings.str.Language = LANG_TR;
			AppSettings.str.FerrosState = FERROS_ENABLED;
			// set field-scanner settings // 
			AppSettings.str.FSLeftDuty = 50;
			AppSettings.str.FSRightDuty = 50;
			AppSettings.str.PWMFreqHZ = DEFAULT_PWM_FREQ_HZ;
			AppSettings.str.OutputVoltageMV = (AT_DEFAULT_VOLTAGE_P2P_V * 1000U);

			AppSettings.str.ATBright = 70;
			AppSettings.str.ATLang = AT_LANG_TR;
			AppSettings.str.ATVol = 100;
			AppSettings.str.ATManFreq = AT_ALLMETAL_FREQ_HZ;
			AppSettings.str.ATAutoFreq = AT_AUTO_FREQ_LONG_GOLD;
			AppSettings.str.ATManAutoSel = AT_AUTO_FREQ_SELECTED;
			AppSettings.str.ATDistanceRange = 0;
			AppSettings.str.ATSearchType = AT_SEARCH_SONAR;

			AppSettings.str.SYSDevType = APP_IS_DETECTOR;
			AppSettings.str.AZPFastState = 0xFFFF;
			
			AppSettings.str.Crc16 = get_block_crc16((uint8_t *)&AppSettings, sizeof(AppSettings)-sizeof(AppSettings.str.Crc16));
		}
		break;
		default:
			while(STALLE_ON_ERR);
			break;
	}
	// update pSettings parameter // 
	if((NULL != pSettings) && (pSettings != (&AppSettings)))
		*pSettings = AppSettings;
}

uAppStore *APP_GetSettingsAdr(void) {
	return &AppSettings;
}

uint16_t APP_GetValue(uint8_t indx) 
{
	uAppStore TempSettings;
	App_ReloadSettings(&TempSettings, RELOAD_FROM_RAM);
	
	return TempSettings.Settings[indx];
}

void APP_SetVal(uint8_t indx, uint16_t val, uint8_t storeNVM)
{
	uAppStore TempSettings;
	App_ReloadSettings(&TempSettings, RELOAD_FROM_RAM);
	TempSettings.Settings[indx] = val;
	APP_StoreSettings(&TempSettings, storeNVM);
}


static uint8_t App_calcCRC8(uint8_t *ptr, uint8_t size)
{
	volatile uint8_t indx;
	uint8_t retval = *ptr++;
	for(indx=0;indx<size;indx++)
		if(0 == (indx & 0x1))
			retval = (*ptr++) ^ retval;	// even
		else
			retval = (*ptr++) ^ (~retval);	// odd 
	return retval;
}

char const *GetString(uint8_t str_indx)
{
    if(APP_IS_DETECTOR == APP_GetValue(ST_DEV_TYPE))
        return AllLangStrs[AppSettings.Settings[ST_LANG]][str_indx];
    else {
        uint8_t at2detector_lang[] = {LANG_TR, LANG_EN, LANG_FR, LANG_AR, LANG_GE, LANG_SP};    // converting at language to detector language //
        return AllLangStrs[at2detector_lang[AppSettings.Settings[ST_AT_LANG]]][str_indx];
    }
}

char const *GetString2Lang(uint8_t str_indx, uint8_t lang) {
	return AllLangStrs[lang][str_indx];
}

void AppSettings_StoreDefaults(void)
{
	uint32_t crc32;
	settings_spi_store set_store = {
		.state = SPI_FLASH_APP_SETTING_INIT_DONE
	};
	App_ReloadSettings(&(set_store.set), RELOAD_FROM_SAFE_VALUEs);	// Reload settings from dfaults // 

	#if(RES_LOAD_SPI_FLASH == RES_LOAD_SOURCE)
		SPI_Flash_Erase( SEC_MAX, SEC_MAX );	 // erase sector // 
		// write settngs store object // 
		df_write_seek(SEC_MAX*SEC_SIZE);	
		DEBUGM("AppSettings Storing @ADR(%u), SIZE(%u)\n", df_write_get_pos(), sizeof(set_store));
		df_write((uint8_t *)&set_store, sizeof(set_store));	// write header //
		// write settings store-object's CRC // 
		crc32 = get_block_crc32((uint8_t *)&set_store, sizeof(set_store));
		df_write_seek(SEC_MAX*SEC_SIZE + sizeof(set_store));	
		DEBUGM("AppSettings-CRC Stored @ADR(%u)\n", df_write_get_pos());
		df_write((uint8_t *)&crc32, sizeof(crc32));	// write crc32 // 
	#elif(RES_LOAD_SDMMC == RES_LOAD_SOURCE)
		//:TODO: 
	#else
		# error "???"
	#endif
}

