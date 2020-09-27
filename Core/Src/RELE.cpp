/*
 * RELE.cpp
 *
 *  Created on: Sep 23, 2020
 *      Author: Deo
 */

#include "RELE.h"

void RELE::setPin(PIN_STATE State)
{
	if(State != LOW && State != HIGH)
	{
		actualState = LOW;
	}
	else
	{
		if(actualState != State)
		{
			actualState = State;
		}
	}
	HAL_GPIO_WritePin(relePort, relePin, (GPIO_PinState)actualState);
}




RELE::RELE(GPIO_TypeDef *RelePort, uint16_t RelePin, PIN_STATE InitialState)
{
	relePort = RelePort;
	relePin = RelePin;
	actualState = InitialState;

}

void RELE::setState(PIN_STATE NewState)
{
	setPin(NewState);
}

void RELE::toggleStatus()
{
	actualState == LOW ? actualState = HIGH : actualState = LOW;
	setPin(actualState);
}


PIN_STATE RELE::getState()
{
	return actualState;
}
