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

	u8g2_t U8G2_Display;
	DISPLAY_PARAMS DispParams;
	TEXT_4_WRITE textToWrite;
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
	NHDST7565_LCD(uint8_t Rotation);
	void setupLcd();
	void drawString(String Text, uint8_t XPos, uint8_t YPos, const uint8_t *u8g2Font);
	void testDisplay(String Text);
};

#endif /* INC_DISPLAY_H_ */
