/*
 * Display.h
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo_Garage
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_
#include "main.h"

class ST7920_LCD
{
private:
	uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr);
	u8g2_t Display;
public:
	void setupDisplay();
};

#endif /* INC_DISPLAY_H_ */
