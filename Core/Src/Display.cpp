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
	case U8X8_MSG_GPIO_AND_DELAY_INIT:	// called once during init phase of u8g2/u8x8
		break;							// can be used to setup pins
	case U8X8_MSG_DELAY_NANO:			// delay arg_int * 1 nano second
		break;
	case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
		__NOP();
		break;
		//Function which delays 10us
	case U8X8_MSG_DELAY_10MICRO:
		for (uint16_t n = 0; n < 320; n++)
		{
			__NOP();
		}
		break;
	case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
		HAL_Delay(arg_int);
		break;
	case U8X8_MSG_DELAY_I2C:				// arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
		break;							// arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
	case U8X8_MSG_GPIO_D0:				// D0 or SPI clock pin: Output level in arg_int
		//case U8X8_MSG_GPIO_SPI_CLOCK:
		break;
	case U8X8_MSG_GPIO_D1:				// D1 or SPI data pin: Output level in arg_int
		//case U8X8_MSG_GPIO_SPI_DATA:
		break;
	case U8X8_MSG_GPIO_D2:				// D2 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_D3:				// D3 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_D4:				// D4 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_D5:				// D5 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_D6:				// D6 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_D7:				// D7 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_E:				// E/WR pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_CS:				// CS (chip select) pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_DC:				// DC (data/cmd, A0, register select) pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_RESET:			// Reset pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_CS1:				// CS1 (chip select) pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_CS2:				// CS2 (chip select) pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_I2C_CLOCK:		// arg_int=0: Output low at I2C clock pin
		break;							// arg_int=1: Input dir with pullup high for I2C clock pin
	case U8X8_MSG_GPIO_I2C_DATA:			// arg_int=0: Output low at I2C data pin
		break;							// arg_int=1: Input dir with pullup high for I2C data pin
	case U8X8_MSG_GPIO_MENU_SELECT:
		u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
		break;
	case U8X8_MSG_GPIO_MENU_NEXT:
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
		break;
	case U8X8_MSG_GPIO_MENU_PREV:
		u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
		break;
	case U8X8_MSG_GPIO_MENU_HOME:
		u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
		break;
	default:
		u8x8_SetGPIOResult(u8x8, 1);			// default return value
		break;
	}
	return 1; // command processed successfully. // @suppress("Return with parenthesis")
}

uint8_t u8x8_byte_stm32_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_BYTE_SEND:

      HAL_SPI_Transmit(&hspi1, (uint8_t *)arg_ptr, arg_int, 100); // @suppress("C-Style cast instead of C++ cast")

//      while( arg_int > 0 )
//      {
//        SPI.transfer((uint8_t)*data);
//        data++;
//        arg_int--;
//      }
      break;
    case U8X8_MSG_BYTE_INIT:
//      u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
//      SPI.begin();
      break;
    case U8X8_MSG_BYTE_SET_DC:
//      u8x8_gpio_SetDC(u8x8, arg_int);
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
        HAL_GPIO_WritePin(LcdCS_GPIO_Port, LcdCS_Pin, GPIO_PIN_RESET); // @suppress("C-Style cast instead of C++ cast")

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
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
    	HAL_GPIO_WritePin(LcdCS_GPIO_Port, LcdCS_Pin, GPIO_PIN_SET);
//      u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
//      u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
//      SPI.endTransaction();
      break;
    default:
      return 0;
  }
  return 1;
}


ST7920_LCD::ST7920_LCD()
{
//	U8G2_Display = new u8g2_t();
	DispParams.width = 128;
	DispParams.high = 64;
}

void ST7920_LCD::setupLcd()
{
	u8g2_Setup_st7920_s_128x64_f(&U8G2_Display, U8G2_R2, u8x8_byte_stm32_hw_spi, u8g2_gpio_and_delay_stm32);
	u8g2_InitDisplay(&U8G2_Display); // send init sequence to the display, display is in sleep mode after this, // @suppress("C-Style cast instead of C++ cast")
	u8g2_SetPowerSave(&U8G2_Display, 0); // wake up display // @suppress("C-Style cast instead of C++ cast")
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
	textToWrite.textLen = u8g2_GetStrWidth(&U8G2_Display, Text.c_str());
	textToWrite.textHigh = u8g2_GetAscent(&U8G2_Display);
	if(textToWrite.textLen < DispParams.width)
	{
		textToWrite.text = Text;
		u8g2_DrawStr(&U8G2_Display, setTextCenter(), setTextMiddle(), textToWrite.text.c_str());
	}
	else
	{

	}
}

void ST7920_LCD::testDisplay(String Text)
{
	u8g2_SetFont(&U8G2_Display, u8g2_font_6x12_tn);
	u8g2_ClearBuffer(&U8G2_Display);
	u8g2_DrawStr(&U8G2_Display, 20, 20, Text.c_str());
//	drawString(Text, CENTER_POS, MIDDLE_POS);
	u8g2_SendBuffer(&U8G2_Display);
}
