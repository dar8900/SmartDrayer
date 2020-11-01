/*
 * Display.cpp
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo_Garage
 */

#include "Display.h"
#include "tim.h"


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


NHDST7565_LCD::NHDST7565_LCD(uint8_t Rotation)
{
	changeDisplayDisposition(Rotation);
}

void NHDST7565_LCD::changeDisplayDisposition(uint8_t NewRotation)
{
	dispParams.rotation = NewRotation;
	if(dispParams.rotation == VERTICAL_1 || dispParams.rotation == VERTICAL_2)
	{
		dispParams.width = 64;
		dispParams.high = 128;
	}
	else
	{
		dispParams.width = 128;
		dispParams.high = 64;
	}
}


void NHDST7565_LCD::assignTextParams(String Text, const uint8_t *Font)
{
	textToWrite.textLen = 0;
	textToWrite.textHigh = 0;
	textToWrite.text = "";
	textToWrite.textFont = Font;
	//	if(textToWrite.textFont != Font)
	//	{
	u8g2_SetFont(&U8G2_Display, textToWrite.textFont);
	textToWrite.textHigh = u8g2_GetAscent(&U8G2_Display);
	//	}
	if(Text != "")
		textToWrite.textLen = u8g2_GetStrWidth(&U8G2_Display, Text.c_str());

	if(textToWrite.textLen < dispParams.width)
	{
		textToWrite.text = Text;
	}
	else
	{
		textToWrite.textFont = displayFonts[W_5_H_8];
		u8g2_SetFont(&U8G2_Display, textToWrite.textFont);
		textToWrite.textLen = u8g2_GetStrWidth(&U8G2_Display, Text.c_str());
		textToWrite.textHigh = u8g2_GetAscent(&U8G2_Display);
	}
}

void NHDST7565_LCD::assignNewDrawCoord(uint8_t OldXPos, uint8_t OldYPos,
		uint8_t &NewXPos, uint8_t &NewYPos)
{
	if(OldXPos <= dispParams.width && OldYPos <= dispParams.high)
	{
		NewXPos = OldXPos;
		NewYPos = OldYPos;
	}
	else if(OldXPos > dispParams.width && OldYPos <= dispParams.high)
	{
		NewYPos = OldYPos;
		switch(OldXPos)
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
	else if(OldXPos <= dispParams.width && OldYPos > dispParams.high)
	{
		NewXPos = OldXPos;
		switch(OldYPos)
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
		switch(OldXPos)
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
		switch(OldYPos)
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
}

void NHDST7565_LCD::setupLcd()
{
	const u8g2_cb_t *DisplayRotation;
	switch(dispParams.rotation)
	{
		case LANDSCAPE_1:
			DisplayRotation = &u8g2_cb_r2;
			break;
		case LANDSCAPE_2:
			DisplayRotation = &u8g2_cb_r0;
			break;
		case VERTICAL_1:
			DisplayRotation = &u8g2_cb_r1;
			break;
		case VERTICAL_2:
			DisplayRotation = &u8g2_cb_r3;
			break;
		default:
			DisplayRotation = &u8g2_cb_r0;
			break;
	}
	u8g2_Setup_st7565_nhd_c12864_f(&U8G2_Display, DisplayRotation, u8x8_byte_stm32_hw_spi, u8g2_gpio_and_delay_stm32);
	u8g2_InitDisplay(&U8G2_Display); // send init sequence to the display, display is in sleep mode after this, // @suppress("C-Style cast instead of C++ cast")
	u8g2_SetPowerSave(&U8G2_Display, 0); // wake up display // @suppress("C-Style cast instead of C++ cast")
	u8g2_SetFontMode(&U8G2_Display, 1);
	u8g2_SetDrawColor(&U8G2_Display, figureColorSetted);
	clearScreen();
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
	NewPos = (dispParams.width - textToWrite.textLen) / 2;
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t NHDST7565_LCD::setTextRight()
{
	uint8_t NewPos = 0;
	NewPos = (dispParams.width - textToWrite.textLen);
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t NHDST7565_LCD::setTextTop()
{
	uint8_t NewPos = 0;
	NewPos = 1;
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t NHDST7565_LCD::setTextMiddle()
{
	uint8_t NewPos = 0;
	NewPos = ((dispParams.high - textToWrite.textHigh + 1) / 2) + (textToWrite.textHigh / 2) - textToWrite.textHigh;
	return NewPos; // @suppress("Return with parenthesis")
}

uint8_t NHDST7565_LCD::setTextBottom()
{
	uint8_t NewPos = 0;
	//	NewPos = (DispParams.high - textToWrite.textHigh - 1);
	NewPos = dispParams.high - 1 - textToWrite.textHigh;
	return NewPos; // @suppress("Return with parenthesis")
}



void NHDST7565_LCD::clearFrameBuffer()
{
	u8g2_ClearBuffer(&U8G2_Display);
}
void NHDST7565_LCD::sendFrameBuffer()
{
	u8g2_SendBuffer(&U8G2_Display);
}

void NHDST7565_LCD::clearScreen()
{
	clearFrameBuffer();
	sendFrameBuffer();
}

void NHDST7565_LCD::drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	u8g2_DrawBox(&U8G2_Display, x, y, w, h);
}

void NHDST7565_LCD::drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t Color)
{
	u8g2_SetDrawColor(&U8G2_Display, Color);
	u8g2_DrawBox(&U8G2_Display, x, y, w, h);
	if(Color != figureColorSetted)
		u8g2_SetDrawColor(&U8G2_Display, figureColorSetted);

}

void NHDST7565_LCD::drawCircle(uint8_t x, uint8_t y, uint8_t r, bool Empty, uint8_t Color)
{
	u8g2_SetDrawColor(&U8G2_Display, Color);
	if(Empty)
		u8g2_DrawCircle(&U8G2_Display, x, y, r, U8G2_DRAW_ALL);
	else
		u8g2_DrawDisc(&U8G2_Display, x, y, r, U8G2_DRAW_ALL);
	if(Color != figureColorSetted)
		u8g2_SetDrawColor(&U8G2_Display, figureColorSetted);
}

void NHDST7565_LCD::drawString(String Text, uint8_t XPos, uint8_t YPos, const uint8_t *u8g2Font)
{
	uint8_t NewXPos = 0, NewYPos = 0;
	assignTextParams(Text, u8g2Font);
	assignNewDrawCoord(XPos, YPos, NewXPos, NewYPos);
//	if(XPos <= dispParams.width && YPos <= dispParams.high)
//	{
//		NewXPos = XPos;
//		NewYPos = YPos;
//	}
//	else if(XPos > dispParams.width && YPos <= dispParams.high)
//	{
//		NewYPos = YPos;
//		switch(XPos)
//		{
//			case LEFT_POS:
//				NewXPos = setTextLeft();
//				break;
//			case CENTER_POS:
//				NewXPos = setTextCenter();
//				break;
//			case RIGHT_POS:
//				NewXPos = setTextRight();
//				break;
//			default:
//				NewXPos = setTextCenter();
//				break;
//		}
//	}
//	else if(XPos <= dispParams.width && YPos > dispParams.high)
//	{
//		NewXPos = XPos;
//		switch(YPos)
//		{
//			case TOP_POS:
//				NewYPos = setTextTop();
//				break;
//			case MIDDLE_POS:
//				NewYPos = setTextMiddle();
//				break;
//			case BOTTOM_POS:
//				NewYPos = setTextBottom();
//				break;
//			default:
//				NewYPos = setTextMiddle();
//				break;
//		}
//	}
//	else
//	{
//		switch(XPos)
//		{
//			case LEFT_POS:
//				NewXPos = setTextLeft();
//				break;
//			case CENTER_POS:
//				NewXPos = setTextCenter();
//				break;
//			case RIGHT_POS:
//				NewXPos = setTextRight();
//				break;
//			default:
//				NewXPos = setTextCenter();
//				break;
//		}
//		switch(YPos)
//		{
//			case TOP_POS:
//				NewYPos = setTextTop();
//				break;
//			case MIDDLE_POS:
//				NewYPos = setTextMiddle();
//				break;
//			case BOTTOM_POS:
//				NewYPos = setTextBottom();
//				break;
//			default:
//				NewYPos = setTextMiddle();
//				break;
//		}
//	}

	u8g2_DrawStr(&U8G2_Display, NewXPos, NewYPos + textToWrite.textHigh, textToWrite.text.c_str());
}

void NHDST7565_LCD::drawText(String Text, uint8_t XPos, uint8_t YPos,
		uint8_t MarginLen)
{
	assignTextParams(Text, displayFonts[W_3_H_6]);
	if(textToWrite.textLen < MarginLen)
	{
		drawString(Text, XPos, YPos, displayFonts[W_3_H_6]);
	}
	else
	{
		int LastChar = 0;
		int NLines = (Text.length() / MarginLen);
		for(uint8_t NSpace = 0; NSpace < Text.length(); NSpace++)
		{
			if(Text[NSpace] == ' ')
				NLines++;
		}
		if((NLines * textToWrite.textHigh) < dispParams.high)
		{
			for(int i = 0; i < NLines; i++)
			{
				String NewLine = "";
				uint8_t j = 0;
				for(j = LastChar; j < MarginLen + LastChar; j++)
				{
					if(j < Text.length() && Text[j] != ' ')
					{
						NewLine += Text[j];
					}
					else
					{
						if(Text[j] == ' ')
							j++;
						break;
					}
				}
				u8g2_DrawStr(&U8G2_Display, XPos, YPos + (i * textToWrite.textHigh), NewLine.c_str());
				LastChar = j;
			}
		}
		else
		{
			drawString("To Long", XPos, YPos, displayFonts[W_3_H_6]);
		}
	}
}


uint8_t NHDST7565_LCD::drawMenuList(uint8_t FirstItemXPos, uint8_t FirstItemYPos, uint8_t FirsListItem, uint8_t ItemSel, const char **MenuItems, uint8_t MaxItems,
		bool WithCheckBox, bool MenuSelected, bool *ItemsChecked, const uint8_t *u8g2Font)
{
	assignTextParams("", u8g2Font);
	uint8_t MaxLines = (dispParams.high - FirstItemYPos) / (textToWrite.textHigh + MENU_ITEM_INTERLINE);
	uint8_t NextItem = 0;
	uint8_t MaxTexLen = 0;
	uint8_t FrameHigh = 0;
	for(int Item = 0; Item < MaxLines; Item++)
	{
		NextItem = FirsListItem + Item;
		if(NextItem >= MaxItems)
			break;
		if(NextItem == ItemSel && MenuSelected)
		{
			assignTextParams(MenuItems[NextItem], u8g2Font);
			u8g2_SetFontMode(&U8G2_Display, 0);
			u8g2_SetDrawColor(&U8G2_Display, 0);
			drawString(MenuItems[NextItem], FirstItemXPos, FirstItemYPos + (Item * (textToWrite.textHigh + MENU_ITEM_INTERLINE)), u8g2Font);
			u8g2_SetFontMode(&U8G2_Display, 1);
			u8g2_SetDrawColor(&U8G2_Display, 1);
			if(!WithCheckBox)
			{
				u8g2_SetFont(&U8G2_Display, displayFonts[W_8_H_8_ICON]);
				u8g2_DrawGlyph(&U8G2_Display, FirstItemXPos + textToWrite.textLen + 1, FirstItemYPos + textToWrite.textHigh + (Item * (textToWrite.textHigh + MENU_ITEM_INTERLINE)) + 1, 0x006F);
				u8g2_SetFont(&U8G2_Display, textToWrite.textFont);
			}
			else
			{
				if(ItemsChecked[NextItem])
				{
					u8g2_DrawFrame(&U8G2_Display, dispParams.width - 11, FirstItemYPos  + (Item * (textToWrite.textHigh + MENU_ITEM_INTERLINE)), 8, 8);
					u8g2_DrawBox(&U8G2_Display, dispParams.width - 9, FirstItemYPos  + (Item * (textToWrite.textHigh + MENU_ITEM_INTERLINE)) + 2, 4, 4);
					//					drawBox(dispParams.width - 9, FirstItemYPos  + (Item * (textToWrite.textHigh + MENU_ITEM_INTERLINE)) + 2, 4, 4, BLACK_COLOR);
				}
				else
				{
					u8g2_DrawFrame(&U8G2_Display, dispParams.width - 11, FirstItemYPos  + (Item * (textToWrite.textHigh + MENU_ITEM_INTERLINE)), 8, 8);
				}
			}
		}
		else
		{
			drawString(MenuItems[NextItem], FirstItemXPos, FirstItemYPos + (Item * (textToWrite.textHigh + MENU_ITEM_INTERLINE)), u8g2Font);
			if(WithCheckBox)
			{
				if(ItemsChecked[NextItem])
				{
					u8g2_DrawFrame(&U8G2_Display, dispParams.width - 11, FirstItemYPos  + (Item * (textToWrite.textHigh + MENU_ITEM_INTERLINE)), 8, 8);
					u8g2_DrawBox(&U8G2_Display, dispParams.width - 9, FirstItemYPos  + (Item * (textToWrite.textHigh + MENU_ITEM_INTERLINE)) + 2, 4, 4);
					//					drawBox(dispParams.width - 9, FirstItemYPos  + (Item * (textToWrite.textHigh + MENU_ITEM_INTERLINE)) + 2, 4, 4, BLACK_COLOR);
				}
				else
				{
					u8g2_DrawFrame(&U8G2_Display, dispParams.width - 11, FirstItemYPos  + (Item * (textToWrite.textHigh + MENU_ITEM_INTERLINE)), 8, 8);
				}
			}
		}
		if(MaxTexLen < textToWrite.textLen)
			MaxTexLen = textToWrite.textLen;
	}
	if((textToWrite.textHigh * (MaxItems + 1)) + 2 < dispParams.high - FirstItemYPos - 1 )
		FrameHigh = (textToWrite.textHigh * (MaxItems + 1)) + 2;
	else
		FrameHigh = dispParams.high - FirstItemYPos - 1;
	if(!WithCheckBox)
	{
		u8g2_DrawRFrame(&U8G2_Display, FirstItemXPos - 1, FirstItemYPos - 1, MaxTexLen + 10, FrameHigh, 3);
	}
	else
	{
		u8g2_DrawRFrame(&U8G2_Display, FirstItemXPos - 1, FirstItemYPos - 1, dispParams.width - FirstItemXPos - 1, FrameHigh, 3);
	}
	return MaxLines;
}

void NHDST7565_LCD::drawSymbol(uint8_t XPos, uint8_t YPos, const uint8_t *SymbolFont, uint16_t SymbolCode)
{
	uint8_t NewXPos = 0, NewYPos = 0;
	assignTextParams("", SymbolFont);
	assignNewDrawCoord(XPos, YPos, NewXPos, NewYPos);
	u8g2_DrawGlyph(&U8G2_Display, NewXPos, NewYPos, SymbolCode);
}


void NHDST7565_LCD::drawTimeDate(String Time, String Date)
{
	drawString(Time, LEFT_POS, TOP_POS, displayFonts[W_3_H_6]);
	drawString(Date, RIGHT_POS, TOP_POS, displayFonts[W_3_H_6]);
}

void NHDST7565_LCD::drawFullScreenPopUp(String Text, uint16_t Delay)
{
	clearFrameBuffer();
	u8g2_DrawRFrame(&U8G2_Display, 1, 1, dispParams.width - 1, dispParams.high - 1, 2);
	drawString(Text, CENTER_POS, MIDDLE_POS, displayFonts[W_6_H_13_B]);
	sendFrameBuffer();
	HAL_Delay(Delay);
}

void NHDST7565_LCD::drawIcon(uint8_t XPos, uint8_t YPos, uint8_t IconW,
		uint8_t IconH, const uint8_t *IconBits)
{
	u8g2_DrawXBM(&U8G2_Display, XPos, YPos, IconW, IconH, IconBits);
}
