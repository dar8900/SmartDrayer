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
		// Stato led
		THERMO_ON = 0,
		THERMO_OFF,
		TEMP_REACHED,
		FAN_ON,
		FAN_OFF,
		PROGRAM_INIT,
		PROGRAM_END,

		ALL_OFF = THERMO_OFF + FAN_OFF,

		MAX_STATUS
	};

	typedef struct
	{
		bool thermoOn;
		bool fanOn;

	}DRYER_FLAG;

	DS1307_RTC *clock;
	DryerKey *keyboard;
	THERMO_SENSOR *tempSens;
	RELE *fanCtrl;
	RELE *thermoCtrl;

	STM32_EEPROM *memory;

	ChronoTimer *getTempTimer;
	ChronoTimer *testTimer;


	SerialDebug *dbgDryer;

	DRYER_FLAG statusFlags;

	void blinkLed(uint8_t WichLed, uint16_t BlinkDelay);
	void toggleLed(uint8_t WichLed);
	void turnOnLed(uint8_t WichLed);
	void turnOffLed(uint8_t WichLed);
	void ledControl(uint8_t State);

	void thermoRegulation(float WichTemp);


public:
	SmartDryer();
	void run();

	void test();

};

#endif /* INC_SMARTDRYER_H_ */
