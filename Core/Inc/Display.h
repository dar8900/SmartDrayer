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
#include <vector>

//typedef std::vector<bool> CheckVector;
typedef std::string String;

class NHDST7565_LCD
{
private:
	typedef struct
	{
		uint8_t width;
		uint8_t high;
		uint8_t rotation;
	}DISPLAY_PARAMS;

	typedef struct
	{
		String text;
		uint8_t textLen;
		uint8_t textHigh;
		const uint8_t *textFont;
	}TEXT_4_WRITE;


	DISPLAY_PARAMS dispParams;
	TEXT_4_WRITE textToWrite;
	static const uint8_t MENU_ITEM_INTERLINE = 3;


	void changeDisplayDisposition(uint8_t NewRotation);
	void assignTextParams(String Text, const uint8_t *Font);
	uint8_t setTextLeft();
	uint8_t setTextCenter();
	uint8_t setTextRight();
	uint8_t setTextTop();
	uint8_t setTextMiddle();
	uint8_t setTextBottom();

public:
	enum
	{
		LANDSCAPE_1 = 0,
		LANDSCAPE_2,
		VERTICAL_1,
		VERTICAL_2
	};
	enum
	{
		LEFT_POS = 130,
		CENTER_POS,
		RIGHT_POS
	};
	enum
	{
		TOP_POS = 70,
		MIDDLE_POS,
		BOTTOM_POS
	};
	enum
	{
		W_3_H_6 = 0,
		W_5_H_8,
		W_6_H_10,
		W_6_H_13_B,
		W_10_H_20,
		W_9_H_17_B,
//		W_4_H_6_T,
//		W_5_H_8_T,
//		W_6_H_10_T,
//		W_6_H_13_BT,
//		W_10_H_20_BT,
		W_8_H_8_ICON,
		MAX_FONTS
	};

	u8g2_t U8G2_Display;

	const uint8_t *displayFonts[MAX_FONTS] =
	{
			u8g2_font_tom_thumb_4x6_mr,
			u8g2_font_5x8_mr,
			u8g2_font_6x10_mr,
			u8g2_font_6x13B_mr,
			u8g2_font_10x20_mr,
			u8g2_font_t0_17_mr,
//			u8g2_font_4x6_tf,
//			u8g2_font_5x8_tf,
//			u8g2_font_6x10_tf,
//			u8g2_font_6x13B_tf,
//			u8g2_font_10x20_tf,
			u8g2_font_open_iconic_all_1x_t,
	};
	NHDST7565_LCD(uint8_t Rotation);
	void setupLcd();
	void clearFrameBuffer();
	void sendFrameBuffer();
	void clearScreen();
	void drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
	void drawCircle(uint8_t x, uint8_t y, uint8_t r, bool Empty);
	void drawString(String Text, uint8_t XPos, uint8_t YPos, const uint8_t *u8g2Font);
	void drawText(String Text, uint8_t XPos, uint8_t YPos, uint8_t MarginLen);
	uint8_t drawMenuList(uint8_t FirstItemXPos, uint8_t FirstItemYPos, uint8_t FirsListItem, uint8_t ItemSel, const char **MenuItems, uint8_t MaxItems,
			bool WithCheckBox, bool MenuSelected, bool *ItemsChecked, const uint8_t *u8g2Font);
};

#endif /* INC_DISPLAY_H_ */
