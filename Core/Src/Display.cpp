/*
 * Display.cpp
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo_Garage
 */

#include "Display.h"

uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
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
			if (arg_int) HAL_GPIO_WritePin(Sck_GPIO_Port, Sck_Pin, GPIO_PIN_RESET);
			else HAL_GPIO_WritePin(Sck_GPIO_Port, Sck_Pin, GPIO_PIN_SET);

		break;
		//Function to define the logic level of the data line to the display
		case U8X8_MSG_GPIO_SPI_DATA:
			if (arg_int) HAL_GPIO_WritePin(Mosi_GPIO_Port, Mosi_Pin, GPIO_PIN_SET);
			else HAL_GPIO_WritePin(Mosi_GPIO_Port, Mosi_Pin, GPIO_PIN_RESET);

		break;
		// Function to define the logic level of the CS line
		case U8X8_MSG_GPIO_CS:
			if (arg_int) HAL_GPIO_WritePin(LcdCS_GPIO_Port, LcdCS_Pin, GPIO_PIN_RESET);
			else HAL_GPIO_WritePin(LcdCS_GPIO_Port, LcdCS_Pin, GPIO_PIN_SET);

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

uint8_t u8x8_byte_stm32_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t *data;
  switch(msg)
  {
    case U8X8_MSG_BYTE_SEND:
      data = (uint8_t *)arg_ptr;
      HAL_GPIO_WritePin(LcdCS_GPIO_Port, LcdCS_Pin, GPIO_PIN_RESET);
      HAL_SPI_Transmit(&hspi1, data, arg_int, 100);
      HAL_GPIO_WritePin(LcdCS_GPIO_Port, LcdCS_Pin, GPIO_PIN_SET);
//      while( arg_int > 0 )
//      {
//        SPI.transfer((uint8_t)*data);
//        data++;
//        arg_int--;
//      }
      break;
//    case U8X8_MSG_BYTE_INIT:
//      u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
//      SPI.begin();
//      break;
//    case U8X8_MSG_BYTE_SET_DC:
//      u8x8_gpio_SetDC(u8x8, arg_int);
//      break;
//    case U8X8_MSG_BYTE_START_TRANSFER:
//      /* SPI mode has to be mapped to the mode of the current controller, at least Uno, Due, 101 have different SPI_MODEx values */
//      internal_spi_mode =  0;
//      switch(u8x8->display_info->spi_mode)
//      {
//        case 0: internal_spi_mode = SPI_MODE0; break;
//        case 1: internal_spi_mode = SPI_MODE1; break;
//        case 2: internal_spi_mode = SPI_MODE2; break;
//        case 3: internal_spi_mode = SPI_MODE3; break;
//      }
//      SPI.beginTransaction(SPISettings(u8x8->display_info->sck_clock_hz, MSBFIRST, internal_spi_mode));
//      u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
//      u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
//      break;
//    case U8X8_MSG_BYTE_END_TRANSFER:
//      u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
//      u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
//      SPI.endTransaction();
//      break;
    default:
      return 0;
  }
  return 1;
}

void ST7920_LCD::setupDisplay()
{
	u8g2_Setup_st7920_p_128x64_f(U8G2_Display, U8G2_R0, u8x8_byte_stm32_hw_spi, u8g2_gpio_and_delay_stm32);
	u8g2_InitDisplay(U8G2_Display); // send init sequence to the display, display is in sleep mode after this, // @suppress("C-Style cast instead of C++ cast")
	u8g2_SetPowerSave(U8G2_Display, 0); // wake up display // @suppress("C-Style cast instead of C++ cast")
}

ST7920_LCD::ST7920_LCD()
{
	U8G2_Display = new u8g2_t();
	DispParams.width = 128;
	DispParams.high = 64;
}

uint8_t ST7920_LCD::setTextLeft()
{
	uint8_t NewPos = 0;
//	TextLen = u8g2_GetStrWidth(U8G2_Display, textToWrite.c_str());
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t ST7920_LCD::setTextCenter()
{
	uint8_t NewPos = 0;
	NewPos = (DispParams.width - textToWrite.textLen) / 2;
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t ST7920_LCD::setTextRight()
{
	uint8_t NewPos = 0;
	NewPos = (DispParams.width - textToWrite.textLen);
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t ST7920_LCD::setTextTop()
{
	uint8_t NewPos = 0;
	NewPos = 0 + textToWrite.textHigh + 1;
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t ST7920_LCD::setTextMiddle()
{
	uint8_t NewPos = 0;
	NewPos = (DispParams.high - textToWrite.textHigh + 1) / 2;
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t ST7920_LCD::setTextBottom()
{
	uint8_t NewPos = 0;
	NewPos = (DispParams.high - textToWrite.textHigh - 1);
	return NewPos; // @suppress("Return with parenthesis")
}

void ST7920_LCD::drawString(String Text, uint8_t XPos, uint8_t YPos)
{
	textToWrite.textLen = 0;
	textToWrite.textHigh = 0;
	textToWrite.text = "";
	textToWrite.textLen = u8g2_GetStrWidth(U8G2_Display, Text.c_str());
	textToWrite.textHigh = u8g2_GetAscent(U8G2_Display);
	if(textToWrite.textLen < DispParams.width)
	{
		textToWrite.text = Text;
		u8g2_DrawStr(U8G2_Display, setTextCenter(), setTextMiddle(), textToWrite.text.c_str());
	}
	else
	{

	}
}

void ST7920_LCD::testDisplay(String Text)
{
	u8g2_ClearBuffer(U8G2_Display);
	drawString(Text, CENTER_POS, MIDDLE_POS);
	u8g2_SendBuffer(U8G2_Display);
}
