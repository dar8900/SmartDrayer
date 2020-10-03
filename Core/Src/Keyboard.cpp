/*
 * Keyboard.cpp
 *
 *  Created on: Sep 20, 2020
 *      Author: Deo
 */

#include "Keyboard.h"


bool BUTTON::readPin()
{
	bool State = false;
	if(HAL_GPIO_ReadPin(buttonPort, buttonPin) == (GPIO_PinState)LOW)
	{
		State = true;
	}
	return State;
}

BUTTON::BUTTON(GPIO_TypeDef *ButtonPort, uint16_t ButtonPin, uint16_t LongPressDelay)
{
	buttonPort = ButtonPort;
	buttonPin = ButtonPin;
	delay = LongPressDelay;
}

uint8_t BUTTON::checkButton()
{
	uint8_t Status = NO_PRESS;
	bool LongPressed = false;
	bool ButtonState = readPin();
	if(ButtonState)
	{
		longPressDelay = HAL_GetTick();
		while(ButtonState)
		{
			ButtonState = readPin();
			if(HAL_GetTick() - longPressDelay > delay)
			{
				LongPressed = true;
				break;
			}
		}
		if(!LongPressed)
		{
			if(!wasLongPressed)
				Status = PRESSED;
			else
				wasLongPressed = false;
		}
		else
		{
			Status = LONG_PRESSED;
			wasLongPressed = true;
		}
		longPressDelay = 0;
		HAL_Delay(10);
	}
	return Status;
}

DryerKey::DryerKey()
{
	keys[0] = new BUTTON(UpButton_GPIO_Port, UpButton_Pin, 250);
	keys[1] = new BUTTON(DownButton_GPIO_Port, DownButton_Pin, 250);
	keys[2] = new BUTTON(LeftButton_GPIO_Port, LeftButton_Pin, 1000);
	keys[3] = new BUTTON(OkButton_GPIO_Port, OkButton_Pin, 1000);
}

uint8_t DryerKey::checkKey()
{
	uint8_t KeyPressed = NO_KEY;
	for(int Key = 0; Key < MAX_KEYS; Key++)
	{
		uint8_t Status = keys[Key]->checkButton();
		if(Status == PRESSED)
		{
			KeyPressed = Key;
		}
		else if(Status == LONG_PRESSED)
		{
			KeyPressed = Key + MAX_KEYS;
		}
		if(Status != NO_PRESS)
		{
			break;
		}
	}
	return KeyPressed;
}
