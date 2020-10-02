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

	typedef struct
	{
		bool thermoOn = false;
		bool fanOn = false;

	}DRYER_FLAG;

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

	void blinkLed(uint8_t WichLed, uint16_t BlinkDelay);
	void toggleLed(uint8_t WichLed);
	void turnOnLed(uint8_t WichLed);
	void turnOffLed(uint8_t WichLed);
	void ledControl();
	void physicalReleCtrl();

	void thermoRegulation(float WichTemp);


public:
	bool enableTest = true;
	SmartDryer();
	void run();

	void test();

};

#endif /* INC_SMARTDRYER_H_ */
