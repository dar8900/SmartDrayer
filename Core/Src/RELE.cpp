/*
 * RELE.cpp
 *
 *  Created on: Sep 23, 2020
 *      Author: Deo
 */

#include "RELE.h"

void RELE::setState(PIN_STATE NewState)
{
	HAL_GPIO_WritePin(relePort, relePin, (GPIO_PinState)NewState);
}

RELE::RELE(GPIO_TypeDef *RelePort, uint16_t RelePin, PIN_STATE InitialState)
{
	relePort = RelePort;
	relePin = RelePin;
	actualState = InitialState;

}

void RELE::toggleStatus()
{
	actualState == LOW ? actualState = HIGH : actualState = LOW;
	setState(actualState);
}

void RELE::setPin(PIN_STATE State)
{
	setState(State);
}


