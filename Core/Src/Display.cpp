/*
 * Display.cpp
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo_Garage
 */

#include "Display.h"
#include "tim.h"

u8g2_t U8G2_Display;

void DelayNs(uint32_t NsDelay)
{
	if(NsDelay < 16)
	{
		NsDelay = 16;
	}
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while(__HAL_TIM_GET_COUNTER(&htim2) < NsDelay);
}

uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{

	switch(msg)
	{
	case U8X8_MSG_GPIO_AND_DELAY_INIT:	// called once during init phase of u8g2/u8x8
		break;							// can be used to setup pins
	case U8X8_MSG_DELAY_NANO:			// delay arg_int * 1 nano second
//		DelayNs(arg_int);
		break;
	case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
//		DelayNs(arg_int);
		__NOP();
		break;
		//Function which delays 10us
	case U8X8_MSG_DELAY_10MICRO:
		DelayNs(10000);
//		for (uint16_t n = 0; n < 320; n++)
//		{
//			__NOP();
//		}
		break;
	case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
		HAL_Delay(arg_int);
		break;					// arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
		//	case U8X8_MSG_GPIO_D0:				// D0 or SPI clock pin: Output level in arg_int
//	case U8X8_MSG_GPIO_SPI_CLOCK:
//		HAL_GPIO_WritePin(Sck_GPIO_Port, Sck_Pin, (GPIO_PinState)arg_int);
//		break;
//		//	case U8X8_MSG_GPIO_D1:				// D1 or SPI data pin: Output level in arg_int
//	case U8X8_MSG_GPIO_SPI_DATA:
//		HAL_GPIO_WritePin(Mosi_GPIO_Port, Mosi_Pin, (GPIO_PinState)arg_int);
//		break;
	case U8X8_MSG_GPIO_CS:				// CS (chip select) pin: Output level in arg_int
		HAL_GPIO_WritePin(LcdCS_GPIO_Port, LcdCS_Pin, (GPIO_PinState)arg_int);
		break;
	case U8X8_MSG_GPIO_DC:				// DC (data/cmd, A0, register select) pin: Output level in arg_int
		HAL_GPIO_WritePin(LcdA0_GPIO_Port, LcdA0_Pin, (GPIO_PinState)arg_int);
		break;
	case U8X8_MSG_GPIO_RESET:			// Reset pin: Output level in arg_int
		HAL_GPIO_WritePin(LcdReset_GPIO_Port, LcdReset_Pin, (GPIO_PinState)arg_int);
		break;
	default:
		u8x8_SetGPIOResult(u8x8, 1);			// default return value
		break;
	}
	return 1; // command processed successfully. // @suppress("Return with parenthesis")
}

uint8_t u8x8_byte_stm32_hw_spi(u8x8_t *u8g2, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
//	uint8_t byte;
//	uint8_t *data;
	switch(msg)
	{
	case U8X8_MSG_BYTE_SEND:
//		data = (uint8_t *)arg_ptr;
//		while (arg_int > 0)
//		{
//			byte = *data;
//			data++;
//			arg_int--;
//			HAL_SPI_Transmit(&hspi1, &byte, 1, 100);
//		}
		HAL_SPI_Transmit(&hspi1, (uint8_t *)arg_ptr, arg_int, 10);
		break;
	case U8X8_MSG_BYTE_INIT:
		u8x8_gpio_SetCS(u8g2, u8g2->display_info->chip_disable_level);
		break;
	case U8X8_MSG_BYTE_SET_DC:
		u8x8_gpio_SetDC(u8g2, arg_int);
		break;
	case U8X8_MSG_BYTE_START_TRANSFER:
		u8x8_gpio_SetCS(u8g2, u8g2->display_info->chip_enable_level);
		__NOP();
//		u8g2->gpio_and_delay_cb(u8g2, U8X8_MSG_DELAY_NANO, u8g2->display_info->post_chip_enable_wait_ns, NULL);
		break;
	case U8X8_MSG_BYTE_END_TRANSFER:
//		u8g2->gpio_and_delay_cb(u8g2, U8X8_MSG_DELAY_NANO, u8g2->display_info->pre_chip_disable_wait_ns, NULL);
		__NOP();
		u8x8_gpio_SetCS(u8g2, u8g2->display_info->chip_disable_level);
		break;
	default:
		return 0;
	}
	return 1;
}


NHDST7565_LCD::NHDST7565_LCD(uint8_t Width, uint8_t High)
{
	//	U8G2_Display = new u8g2_t();
	DispParams.width = Width;
	DispParams.high = High;
}

void NHDST7565_LCD::setupLcd()
{
	u8g2_Setup_st7565_nhd_c12864_f(&U8G2_Display, U8G2_R2, u8x8_byte_stm32_hw_spi, u8g2_gpio_and_delay_stm32);
	u8g2_InitDisplay(&U8G2_Display); // send init sequence to the display, display is in sleep mode after this, // @suppress("C-Style cast instead of C++ cast")
	u8g2_SetPowerSave(&U8G2_Display, 0); // wake up display // @suppress("C-Style cast instead of C++ cast")
}

void NHDST7565_LCD::assignTextParams(String Text, const uint8_t *Font)
{
	textToWrite.textLen = 0;
	textToWrite.textHigh = 0;
	textToWrite.text = "";
	textToWrite.textFont = Font;
	u8g2_SetFont(&U8G2_Display, textToWrite.textFont);
	textToWrite.textLen = u8g2_GetStrWidth(&U8G2_Display, Text.c_str());
	textToWrite.textHigh = u8g2_GetAscent(&U8G2_Display);

	if(textToWrite.textLen < DispParams.width)
	{
		textToWrite.text = Text;
	}
	else
	{
		textToWrite.textFont = u8g2_font_5x8_mf;
		textToWrite.text = "STRING ERROR!";
	}
}


uint8_t NHDST7565_LCD::setTextLeft()
{
	uint8_t NewPos = 0;
	//	TextLen = u8g2_GetStrWidth(U8G2_Display, textToWrite.c_str());
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t NHDST7565_LCD::setTextCenter()
{
	uint8_t NewPos = 0;
	NewPos = (DispParams.width - textToWrite.textLen) / 2;
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t NHDST7565_LCD::setTextRight()
{
	uint8_t NewPos = 0;
	NewPos = (DispParams.width - textToWrite.textLen);
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t NHDST7565_LCD::setTextTop()
{
	uint8_t NewPos = 0;
	NewPos = 0 + textToWrite.textHigh + 1;
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t NHDST7565_LCD::setTextMiddle()
{
	uint8_t NewPos = 0;
	NewPos = (DispParams.high - textToWrite.textHigh + 1) / 2;
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t NHDST7565_LCD::setTextBottom()
{
	uint8_t NewPos = 0;
	NewPos = (DispParams.high - textToWrite.textHigh - 1);
	return NewPos; // @suppress("Return with parenthesis")
}

void NHDST7565_LCD::drawString(String Text, uint8_t XPos, uint8_t YPos, const uint8_t *u8g2Font)
{
	uint8_t NewXPos = 0, NewYPos = 0;
	assignTextParams(Text, u8g2Font);

	if(XPos <= DispParams.width && YPos <= DispParams.high)
	{
		NewXPos = XPos;
		NewYPos = YPos;
	}
	else if(XPos > DispParams.width && YPos <= DispParams.high)
	{
		NewYPos = YPos;
		switch(XPos)
		{
		case LEFT_POS:
			NewXPos = setTextLeft();
			break;
		case CENTER_POS:
			NewXPos = setTextCenter();
			break;
		case RIGHT_POS:
			NewXPos = setTextRight();
			break;
		default:
			NewXPos = setTextCenter();
			break;
		}
	}
	else if(XPos <= DispParams.width && YPos > DispParams.high)
	{
		NewXPos = XPos;
		switch(YPos)
		{
		case TOP_POS:
			NewYPos = setTextTop();
			break;
		case MIDDLE_POS:
			NewYPos = setTextMiddle();
			break;
		case BOTTOM_POS:
			NewYPos = setTextBottom();
			break;
		default:
			NewYPos = setTextMiddle();
			break;
		}
	}
	else
	{
		switch(XPos)
		{
		case LEFT_POS:
			NewXPos = setTextLeft();
			break;
		case CENTER_POS:
			NewXPos = setTextCenter();
			break;
		case RIGHT_POS:
			NewXPos = setTextRight();
			break;
		default:
			NewXPos = setTextCenter();
			break;
		}
		switch(YPos)
		{
		case TOP_POS:
			NewYPos = setTextTop();
			break;
		case MIDDLE_POS:
			NewYPos = setTextMiddle();
			break;
		case BOTTOM_POS:
			NewYPos = setTextBottom();
			break;
		default:
			NewYPos = setTextMiddle();
			break;
		}
	}

	u8g2_DrawStr(&U8G2_Display, NewXPos, NewYPos, textToWrite.text.c_str());
}


void NHDST7565_LCD::testDisplay(String Text)
{
	//	u8g2_SetFont(&U8G2_Display, u8g2_font_6x12_tn);
	u8g2_ClearBuffer(&U8G2_Display);
	//	u8g2_DrawStr(&U8G2_Display, 20, 20, Text.c_str());
//	drawString(Text, CENTER_POS, MIDDLE_POS, u8g2_font_5x8_mf );
	u8g2_DrawHLine(&U8G2_Display, 5, 32, 100);
	u8g2_SendBuffer(&U8G2_Display);
}
