/*
 * RELE.h
 *
 *  Created on: Sep 23, 2020
 *      Author: Deo
 */

#ifndef RELE_H_
#define RELE_H_

#include "main.h"

class RELE
{
private:
	GPIO_TypeDef *relePort;
	uint16_t relePin;
	PIN_STATE actualState;
	void setPin(PIN_STATE State);
public:
	RELE(GPIO_TypeDef *RelePort, uint16_t RelePin, PIN_STATE InitialState = LOW);
	void toggleStatus();
	void setState(PIN_STATE NewState);

};

#endif /* RELE_H_ */
