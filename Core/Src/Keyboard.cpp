/*
 * Keyboard.cpp
 *
 *  Created on: Sep 20, 2020
 *      Author: Deo
 */

#include "Keyboard.h"


bool KEYBOARD::readPin()
{
	bool State = false;
	if(HAL_GPIO_ReadPin(buttonPort, buttonPin) == (GPIO_PinState)LOW)
	{
		State = true;
	}
	return State;
}

KEYBOARD::KEYBOARD(GPIO_TypeDef *ButtonPort, uint16_t ButtonPin, uint16_t LongPressDelay)
{
	buttonPort = ButtonPort;
	buttonPin = ButtonPin;
	delay = LongPressDelay;
}

uint8_t KEYBOARD::checkButton()
{
	uint8_t Status = NO_PRESS;
	bool ButtonState = readPin();
	if(ButtonState)
	{
		longPressDelay = HAL_GetTick();
		while(ButtonState)
		{
			ButtonState = readPin();
			if(HAL_GetTick() - longPressDelay > delay)
			{
				Status = LONG_PRESSED;
				wasLongPressed = true;
				break;
			}
		}
		if(!wasLongPressed)
		{
			Status = PRESSED;
		}
		else
		{
			wasLongPressed = false;
		}
		longPressDelay = 0;
		HAL_Delay(25);
	}
	return Status;
}
