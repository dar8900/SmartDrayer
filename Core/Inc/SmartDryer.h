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

#define MAX_PROGRAMS	1

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
		PARAM_FLOAT_TYPE,
		MAX_PARAM_TYPE
	};

	enum
	{
		MAIN_MENU = 0,
		THERMO_CTRL,
		FAN_CTRL,
		TEMP_CTRL,
		START_DRYER_CTRL,
		CHANGE_TIME_MENU,
		CHANGE_DATE_MENU,
		CHANGE_PROGRAMS_LIST,
		START_PROGRAMS_LIST,
		SHOW_INFO_PAGE,
		MAX_MENU_ITEMS
	};

	enum
	{
		NAV_MENU = 0,
		CHANGE_TIME,
		CHANGE_DATE,
		SHOW_INFO,
		CHANGE_PROGRAM_1,
		START_PROGRAM_1,
		MAX_SCREENS
	};

	typedef struct
	{
		bool dryerOn = false;
		bool thermoOn = false;
		bool fanOn = false;
		float temperatureSetted = 30.0;
	}DRYER_PARAMS;

	typedef struct
	{
		String menuTitle;
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

	typedef struct
	{
		DS1307_RTC::TIME_DATE_T startTime = {0};
		DS1307_RTC::TIME_DATE_T endTime = {0};
		float tempSetted = 0.0;
	}PROGRAM_STRUCURE;

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
	ChronoTimer *showHelpMessageTimer;
	ChronoTimer *testTimer;


	SerialDebug *dbgDryer;

	bool rtcRunning = false;

	DRYER_PARAMS *statusParam;
	PROGRAM_STRUCURE dryerPrograms[MAX_PROGRAMS] = {0};

	uint16_t ledStatus = THERMO_OFF_FAN_OFF;
	uint32_t readedTemperature = 0;

	uint8_t *paramTemperatures;


	StrVector mainMenuHelpMsgs;

	MENU_STRUCTURE *mainMenu;
	MENU_STRUCTURE *thermoMenuCtrl;
	MENU_STRUCTURE *fanMenuCtrl;
	MENU_STRUCTURE *tempMenuCtrl;
	MENU_STRUCTURE *startDryerMenu;
	MENU_STRUCTURE *changeProgramsMenu;
	MENU_STRUCTURE *startProgramsMenu;


	uint8_t screen = NAV_MENU;

	bool eepromEnabled = false;

	void blinkLed(uint8_t WichLed, uint16_t BlinkDelay);
	void toggleLed(uint8_t WichLed);
	void turnOnLed(uint8_t WichLed);
	void turnOffLed(uint8_t WichLed);
	void ledControl();
	void physicalReleCtrl();
	void thermoRegulation(float WichTemp);
	void peripheralsControl();

	void navMenu();
	void changeTime();
	void changeDate();
	void showInfo();
	void changeProgram(uint8_t WichProgram);
	void startProgram(uint8_t WichProgram);

public:
	bool enableTest = false;
	SmartDryer();
	void setup();
	void run();

	void test();

};

#endif /* INC_SMARTDRYER_H_ */
