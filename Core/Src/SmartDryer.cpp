/*
 * SmartDryer.cpp
 *
 *  Created on: 27 set 2020
 *      Author: dar89
 */

#include <SmartDryer.h>

#define TOLLERANCE_DEGREES	55

SmartDryer::SmartDryer()
{
	display = new NHDST7565_LCD(128, 64);
	clock = new DS1307_RTC();
	keyboard = new DryerKey();
	tempSens = new THERMO_SENSOR();
	fanCtrl = new RELE(FanCtrl_GPIO_Port, FanCtrl_Pin, LOW);
	thermoCtrl = new RELE(ThermoCtrl_GPIO_Port, ThermoCtrl_Pin, LOW);

	memory = new STM32_EEPROM();

	getTempTimer = new ChronoTimer(ChronoTimer::SECONDS);

	blinkRedLedTimer = new ChronoTimer(ChronoTimer::MILLIS);
	blinkGreenLedTimer = new ChronoTimer(ChronoTimer::MILLIS);

	testTimer = new ChronoTimer(ChronoTimer::MILLIS);

	dbgDryer = new SerialDebug();

}


void SmartDryer::blinkLed(uint8_t WichLed, uint16_t BlinkDelay)
{
//	GPIO_TypeDef *LedPort;
//	uint16_t LedPin = 0;
	if(BlinkDelay <= 1)
	{
		BlinkDelay = 2;
	}
	switch(WichLed)
	{
	case RED_LED:
//		LedPort = RedLed_GPIO_Port;
//		LedPin = RedLed_Pin;
		if(blinkRedLedTimer->isFinished(true, BlinkDelay))
		{
			toggleLed(WichLed);
		}
		break;
	case GREEN_LED:
		if(blinkGreenLedTimer->isFinished(true, BlinkDelay))
		{
			toggleLed(WichLed);
		}
//		LedPort = GreenLed_GPIO_Port;
//		LedPin = GreenLed_Pin;
		break;
	default:
		break;
	}

}

void SmartDryer::toggleLed(uint8_t WichLed)
{
	GPIO_TypeDef *LedPort;
	uint16_t LedPin = 0;
	switch(WichLed)
	{
	case RED_LED:
		LedPort = RedLed_GPIO_Port;
		LedPin = RedLed_Pin;
		break;
	case GREEN_LED:
		LedPort = GreenLed_GPIO_Port;
		LedPin = GreenLed_Pin;
		break;
	default:
		break;
	}
	HAL_GPIO_TogglePin(LedPort, LedPin);
}

void SmartDryer::turnOnLed(uint8_t WichLed)
{
	GPIO_TypeDef *LedPort;
	uint16_t LedPin = 0;
	switch(WichLed)
	{
	case RED_LED:
		LedPort = RedLed_GPIO_Port;
		LedPin = RedLed_Pin;
		break;
	case GREEN_LED:
		LedPort = GreenLed_GPIO_Port;
		LedPin = GreenLed_Pin;
		break;
	default:
		break;
	}
	HAL_GPIO_WritePin(LedPort, LedPin, GPIO_PIN_SET);
}

void SmartDryer::turnOffLed(uint8_t WichLed)
{
	GPIO_TypeDef *LedPort;
	uint16_t LedPin = 0;
	switch(WichLed)
	{
	case RED_LED:
		LedPort = RedLed_GPIO_Port;
		LedPin = RedLed_Pin;
		break;
	case GREEN_LED:
		LedPort = GreenLed_GPIO_Port;
		LedPin = GreenLed_Pin;
		break;
	default:
		break;
	}
	HAL_GPIO_WritePin(LedPort, LedPin, GPIO_PIN_RESET);
}

void SmartDryer::ledControl()
{

	switch(ledStatus)
	{
	case THERMO_ON_FAN_OFF:
		blinkLed(RED_LED, 10);
		turnOffLed(GREEN_LED);
		break;
	case THERMO_OFF_FAN_ON:
		turnOffLed(RED_LED);
		blinkLed(GREEN_LED, 500);
		break;
	case THERMO_OFF_FAN_OFF:
		turnOffLed(GREEN_LED);
		turnOffLed(RED_LED);
		break;
	case THERMO_ON_FAN_ON:
		blinkLed(RED_LED, 10);
		blinkLed(GREEN_LED, 500);
		break;
	case TEMP_REACHED:
		turnOnLed(RED_LED);
		HAL_Delay(250);
		break;
	case PROGRAM_INIT:
		for(int i = 0; i < 50*10; i++)
		{
			blinkLed(RED_LED, 50);
			blinkLed(GREEN_LED, 50);
			HAL_Delay(1);
		};
		break;
	case PROGRAM_END:
		for(int i = 0; i < 50*20; i++)
		{
			blinkLed(RED_LED, 50);
			blinkLed(GREEN_LED, 50);
			HAL_Delay(1);
		};
		break;
	case UNKNOWN_STATE:
	default:
		blinkLed(RED_LED, 5);
		blinkLed(GREEN_LED, 5);
		break;
	}
}

void SmartDryer::thermoRegulation(float WichTemp)
{
	float ReadedTemp = 0.0;
	uint32_t TempTarget = (uint32_t)roundf(WichTemp * 10);
	if(getTempTimer->isFinished(true, 2))
	{
		ReadedTemp = tempSens->getTemperature();
		readedTemperature = (uint32_t)roundf(ReadedTemp * 10);
	}
	if(readedTemperature != 0)
	{
		if(readedTemperature >= TempTarget - TOLLERANCE_DEGREES)
		{
			statusFlags.fanOn = true;
			statusFlags.thermoOn = true;
			ledStatus = THERMO_ON_FAN_ON;
		}
		else if(readedTemperature >= TempTarget)
		{
			statusFlags.fanOn = true;
			statusFlags.thermoOn = false;
			ledStatus = THERMO_OFF_FAN_ON;
		}
		else if(readedTemperature < TempTarget - TOLLERANCE_DEGREES)
		{
			statusFlags.fanOn = false;
			statusFlags.thermoOn = true;
			ledStatus = THERMO_ON_FAN_OFF;
		}
	}
	else
	{
		statusFlags.fanOn = false;
		statusFlags.thermoOn = false;
		ledStatus = THERMO_OFF_FAN_OFF;
	}
}

void SmartDryer::physicalReleCtrl()
{
	if(statusFlags.fanOn)
	{
		fanCtrl->setState(HIGH);
	}
	else
	{
		fanCtrl->setState(LOW);
	}
	if(statusFlags.thermoOn)
	{
		thermoCtrl->setState(HIGH);
	}
	else
	{
		thermoCtrl->setState(LOW);
	}
}

void SmartDryer::test()
{
	testTimer->setTimer(5000);

	display->setupLcd();

	while(1)
	{
		  uint8_t WichKey = DryerKey::NO_KEY, TestKey = 0;
		  char SerialData[RECEIVE_BUFFER_LEN] = {0};

		  dbgDryer->readSerialIT((uint8_t *)SerialData);
		  if(SerialData[5] != 0)
		  {
			  std::string Rec = "";
			  for(int i = 0; i < RECEIVE_BUFFER_LEN; i++)
				  Rec += SerialData[i];
			  dbgDryer->sendDbgStr("Ricevuto: " + Rec);
		  }

		  WichKey = keyboard->checkKey();
		  switch(WichKey)
		  {
		  case DryerKey::UP_KEY:
			  TestKey = 1;
			  break;
		  case DryerKey::DOWN_KEY:
			  TestKey = 2;
			  break;
		  case DryerKey::LEFT_KEY:
			  TestKey = 3;
			  break;
		  case DryerKey::OK_KEY:
			  TestKey = 4;
			  break;
		  case DryerKey::LONG_UP_KEY:
			  TestKey = 5;
			  break;
		  case DryerKey::LONG_DOWN_KEY:
			  TestKey = 6;
			  break;
		  case DryerKey::LONG_LEFT_KEY:
			  TestKey = 7;
			  break;
		  case DryerKey::LONG_OK_KEY:
			  TestKey = 8;
			  break;
		  default:
			  break;
		  }
		  if(TestKey != 0)
		  {
			  dbgDryer->sendDbgStr("Il tasto premuto vale " + std::to_string(TestKey));
		  }

//		  if(testTimer->isFinished(true, 2000))
//		  {
//			  dbgDryer->sendDbgStr(clock->getTimeDateStr(DS1307_RTC::ONLY_TIME));
//		  }
		  physicalReleCtrl();

		  if(testTimer->isFinished(true))
		  {
			  ledStatus = PROGRAM_END;
		  }
		  else
		  {
			  ledStatus = THERMO_ON_FAN_OFF;
		  }
		  ledControl();

		  display->testDisplay("Test");
	}
}

void SmartDryer::run()
{
	while(1)
	{

	}
}

