/*
 * SmartDryer.h
 *
 *  Created on: 27 set 2020
 *      Author: dar89
 */

#ifndef INC_SMARTDRYER_H_
#define INC_SMARTDRYER_H_

#include "main.h"
#include "DS1307RTC.h"
#include "Display.h"
#include "ChronoTimer.h"
#include "Keyboard.h"
#include "RELE.h"
#include "thermoSensor.h"
#include "SerialDebug.h"

#include "Eeprom.h"


#define MENU_SELECTED 		true
#define MENU_UNSELECTED 	false

#define WITH_CHECKBOX		true
#define WITHOUT_CHECKBOX	false

class SmartDryer
{
private:

	enum
	{
		RED_LED = 0,
		GREEN_LED
	};
	enum
	{
		// Stato led funzionamento
		THERMO_ON_FAN_OFF = 0,
		THERMO_OFF_FAN_ON,
		THERMO_OFF_FAN_OFF,
		THERMO_ON_FAN_ON,

		// Stato led attivazioni particolari
		TEMP_REACHED = 500,
		PROGRAM_INIT,
		PROGRAM_END,
		UNKNOWN_STATE
	};

	enum
	{
		NO_TYPE = 0,
		PARAM_BOOL_TYPE,
		PARAM_VALUE_UINT_TYPE,
		PARAM_VALUE_INT_TYPE,
		MAX_PARAM_TYPE
	};

	typedef struct
	{
		bool thermoOn = false;
		bool fanOn = false;

	}DRYER_FLAG;

	typedef struct
	{
		String menuTitle;
		const uint8_t *menuTitleFont;
		const char **menuVoices;
		uint8_t XPos;
		uint8_t YPos;
		const uint8_t *menuFont;
		uint8_t topItemPos;
		uint8_t itemSelected;
		uint8_t maxMenuLines;
		uint8_t maxMenuItems;
		bool withChebox;
		bool *itemsChecked;
		void *paramAssociated;
		uint8_t paramType;
		bool menuSelected;
	}MENU_STRUCTURE;


	NHDST7565_LCD *display;
	DS1307_RTC *clock;
	DryerKey *keyboard;
	THERMO_SENSOR *tempSens;
	RELE *fanCtrl;
	RELE *thermoCtrl;

	STM32_EEPROM *memory;

	ChronoTimer *getTempTimer;
	ChronoTimer *blinkRedLedTimer;
	ChronoTimer *blinkGreenLedTimer;
	ChronoTimer *testTimer;


	SerialDebug *dbgDryer;

	DRYER_FLAG statusFlags;
	uint16_t ledStatus = UNKNOWN_STATE;
	uint32_t readedTemperature = 0.0;

	enum
	{
		MAIN_MENU = 0,
		THERMO_CTRL,
		FAN_CTRL,
		MAX_MENU
	};

	MENU_STRUCTURE *mainMenu;
	MENU_STRUCTURE *thermoMenuCtrl;
	MENU_STRUCTURE *fanMenuCtrl;

//	MENU_STRUCTURE mainMenu =
//	{
//			"Menu principale",
//			display->displayFonts[NHDST7565_LCD::W_6_H_13_B],
//			mainMenuVoices,
//			5,
//			14,
//			display->displayFonts[NHDST7565_LCD::W_5_H_8],
//			0,
//			0,
//			0,
//			sizeof(mainMenuVoices)/sizeof(mainMenuVoices[0]),
//			WITHOUT_CHECKBOX,
//			new bool(2),
//			NULL,
//			MENU_SELECTED,
//	};
//
//
//	MENU_STRUCTURE thermoCtrl =
//	{
//			"Ctrl. thermo",
//			display->displayFonts[NHDST7565_LCD::W_6_H_13_B],
//			OnOff,
//			60,
//			14,
//			display->displayFonts[NHDST7565_LCD::W_5_H_8],
//			0,
//			0,
//			0,
//			2,
//			WITH_CHECKBOX,
//			new bool(2),
//			(bool *)&statusFlags.thermoOn,
//			MENU_UNSELECTED,
//	};

//	MENU_STRUCTURE fanCtrl =
//	{
//			"Ctrl. ventola",
//			display->displayFonts[NHDST7565_LCD::W_6_H_13_B],
//			OnOff,
//			60,
//			14,
//			display->displayFonts[NHDST7565_LCD::W_5_H_8],
//			0,
//			0,
//			0,
//			2,
//			WITH_CHECKBOX,
//			new bool(2),
//			(bool *)&statusFlags.fanOn,
//			MENU_UNSELECTED,
//	};

	void blinkLed(uint8_t WichLed, uint16_t BlinkDelay);
	void toggleLed(uint8_t WichLed);
	void turnOnLed(uint8_t WichLed);
	void turnOffLed(uint8_t WichLed);
	void ledControl();
	void physicalReleCtrl();

	void thermoRegulation(float WichTemp);

	void navMenu();


public:
	bool enableTest = true;
	SmartDryer();
	void run();

	void test();

};

#endif /* INC_SMARTDRYER_H_ */
