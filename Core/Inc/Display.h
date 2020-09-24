/*
 * Display.h
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo_Garage
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_
#include "main.h"
#include "spi.h"
#include <string>

class ST7920_LCD
{
private:
	u8g2_t Display;

public:
	void setupDisplay();
};

#endif /* INC_DISPLAY_H_ */
