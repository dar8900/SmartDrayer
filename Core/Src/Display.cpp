/*
 * Display.cpp
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo_Garage
 */

#include "Display.h"

uint8_t ST7920_LCD::u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{
	switch(msg)
	{
		//Initialize SPI peripheral
		case U8X8_MSG_GPIO_AND_DELAY_INIT:
			/* HAL initialization contains all what we need so we can skip this part. */

		break;

		//Function which implements a delay, arg_int contains the amount of ms
		case U8X8_MSG_DELAY_MILLI:
		HAL_Delay(arg_int);

		break;
		//Function which delays 10us
		case U8X8_MSG_DELAY_10MICRO:
		for (uint16_t n = 0; n < 320; n++)
		{
			__NOP();
		}

		break;
		//Function which delays 100ns
		case U8X8_MSG_DELAY_100NANO:
		__NOP();

		break;
		//Function to define the logic level of the clockline
		case U8X8_MSG_GPIO_SPI_CLOCK:
			if (arg_int) HAL_GPIO_WritePin(Sck_GPIO_Port, Sck_Pin, RESET);
			else HAL_GPIO_WritePin(Sck_GPIO_Port, Sck_Pin, SET);

		break;
		//Function to define the logic level of the data line to the display
		case U8X8_MSG_GPIO_SPI_DATA:
			if (arg_int) HAL_GPIO_WritePin(Mosi_GPIO_Port, Mosi_Pin, SET);
			else HAL_GPIO_WritePin(Mosi_GPIO_Port, Mosi_Pin, RESET);

		break;
		// Function to define the logic level of the CS line
		case U8X8_MSG_GPIO_CS:
			if (arg_int) HAL_GPIO_WritePin(LcdCS_GPIO_Port, LcdCS_Pin, RESET);
			else HAL_GPIO_WritePin(LcdCS_GPIO_Port, LcdCS_Pin, SET);

		break;
		//Function to define the logic level of the Data/ Command line
		case U8X8_MSG_GPIO_DC:
//			if (arg_int) HAL_GPIO_WritePin(CD_LCD_PORT, CD_LCD_PIN, SET);
//			else HAL_GPIO_WritePin(CD_LCD_PORT, CD_LCD_PIN, RESET);

		break;
		//Function to define the logic level of the RESET line
//		case U8X8_MSG_GPIO_RESET:
//			if (arg_int) HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, SET);
//			else HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, RESET);
//
//		break;
		default:
			return 0; //A message was received which is not implemented, return 0 to indicate an error
	}

	return 1; // command processed successfully.
}


void ST7920_LCD::setupDisplay()
{
	u8g2_Setup_st7920_p_128x64_f(&Display, U8G2_R0, u8x8_byte_3wire_sw_spi, u8g2_gpio_and_delay_stm32);
	u8g2_InitDisplay(&Display); // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&Display, 0); // wake up display
}
