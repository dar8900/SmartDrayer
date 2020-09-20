/*
 * Keyboard.h
 *
 *  Created on: Sep 20, 2020
 *      Author: Deo
 */

#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_

#include "main.h"

enum
{
	PRESSED = 0,
	LONG_PRESSED,
	NO_PRESS
};

class KEYBOARD
{
private:

	GPIO_TypeDef *buttonPort;
	uint16_t buttonPin;
	uint32_t longPressDelay = 0;
	uint16_t delay = 0;
	bool wasLongPressed = false;
	bool readPin();
public:

	KEYBOARD(GPIO_TypeDef *ButtonPort, uint16_t ButtonPin, uint16_t ButtonDelay);
	uint8_t checkButton();
};

#endif /* INC_KEYBOARD_H_ */
