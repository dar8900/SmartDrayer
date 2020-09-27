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
	clock = new DS1307_RTC();
	keyboard = new DryerKey();
	tempSens = new THERMO_SENSOR();
	fanCtrl = new RELE(FanCtrl_GPIO_Port, FanCtrl_Pin, LOW);
	thermoCtrl = new RELE(ThermoCtrl_GPIO_Port, ThermoCtrl_Pin, LOW);

	memory = new STM32_EEPROM();

	getTempTimer = new ChronoTimer(ChronoTimer::SECONDS);
	testTimer = new ChronoTimer(ChronoTimer::MILLIS);

	dbgDryer = new SerialDebug();

}

void SmartDryer::run()
{
	while(1)
	{

	}
}

void SmartDryer::blinkLed(uint8_t WichLed, uint16_t BlinkDelay)
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
	if(BlinkDelay <= 1)
	{
		BlinkDelay = 2;
	}
	HAL_GPIO_WritePin(LedPort, LedPin, GPIO_PIN_SET);
	HAL_Delay(BlinkDelay);
	HAL_GPIO_WritePin(LedPort, LedPin, GPIO_PIN_RESET);
	HAL_Delay(BlinkDelay);

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

void SmartDryer::ledControl(uint8_t State)
{
	switch(State)
	{
	case THERMO_ON:
		blinkLed(RED_LED, 5);
		break;
	case THERMO_OFF:
		turnOffLed(RED_LED);
		break;
	case TEMP_REACHED:
		turnOnLed(RED_LED);
		break;
	default:
		break;
	}
}

void SmartDryer::thermoRegulation(float WichTemp)
{
	float ReadedTemp = 0.0;
	uint32_t TempTarget = (uint32_t)roundf(WichTemp * 10), TempReaded = 0;
	if(getTempTimer->isFinished(true, 2))
	{
		ReadedTemp = tempSens->getTemperature();
		TempReaded = (uint32_t)roundf(ReadedTemp * 10);
	}
	if(TempReaded != 0)
	{
		if(TempReaded >= TempTarget - TOLLERANCE_DEGREES)
		{
			statusFlags.fanOn = true;
		}
		else if(TempReaded >= TempTarget)
		{
			statusFlags.fanOn = true;
			statusFlags.thermoOn = false;
		}
		else if(TempReaded < TempTarget - TOLLERANCE_DEGREES && !statusFlags.thermoOn)
		{
			statusFlags.thermoOn = true;
			statusFlags.fanOn = false;
		}
		else if(TempReaded < TempTarget - TOLLERANCE_DEGREES)
		{
			statusFlags.fanOn = false;
		}
	}
}



void SmartDryer::test()
{
	while(1)
	{
		  uint8_t WichKey = DryerKey::NO_KEY, TestKey = 0;
		  char SerialData[RECEIVE_BUFFER_LEN] = {0};
	//	  Display.testDisplay("Test");

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

		  if(testTimer->isFinished(true, 2000))
		  {
			  dbgDryer->sendDbgStr(clock->getTimeDateStr(DS1307_RTC::ONLY_TIME));
		  }

	}
}
