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

class BUTTON
{
private:

	GPIO_TypeDef *buttonPort;
	uint16_t buttonPin;
	uint32_t longPressDelay = 0;
	uint16_t delay = 0;
	bool wasLongPressed = false;
	bool readPin();
public:

	BUTTON(GPIO_TypeDef *ButtonPort, uint16_t ButtonPin, uint16_t ButtonDelay);
	uint8_t checkButton();
};

class DryerKey
{
private:
	static const uint8_t MAX_KEYS = 4;
	BUTTON *keys[MAX_KEYS];

public:
	enum
	{
		UP_KEY = 0,
		DOWN_KEY,
		LEFT_KEY,
		OK_KEY,
		LONG_UP_KEY,
		LONG_DOWN_KEY,
		LONG_LEFT_KEY,
		LONG_OK_KEY,
		NO_KEY
	};
	DryerKey();
	uint8_t checkKey();

};
#endif /* INC_KEYBOARD_H_ */
