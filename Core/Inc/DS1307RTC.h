/*
 * DS1307RTC.h
 *
 *  Created on: 25 set 2020
 *      Author: deo
 */

#ifndef INC_DS1307RTC_H_
#define INC_DS1307RTC_H_
#include "main.h"
#include "i2c.h"
#include <string>

typedef std::string String;

extern const uint8_t daysInMonth[];
extern const char *monthNames[];

class DS1307_RTC
{
private:
	static const uint8_t BUFFER_LEN = 32;
	static const uint8_t ds1307Address = 0x68;
	static const uint8_t ds1307Control = 0x07;
	static const uint8_t ds1307NVRam = 0x08;
	uint8_t i2cBuffer[BUFFER_LEN];
	uint8_t i2cBufferIndex = 0;
	uint16_t devAddressLShifted();
	void clearI2CBuff();
	uint16_t date2days(uint16_t y, uint8_t m, uint8_t d);
	uint32_t time2ulong(uint16_t days, uint8_t h, uint8_t m, uint8_t s);

public:
	typedef struct
	{
		uint8_t		second;
		uint8_t		minute;
		uint8_t		hour;
		uint8_t		day;
		uint8_t		month;
		uint16_t	year;

	}TIME_DATE_T;

	enum
	{
		ONLY_TIME = 0,
		ONLY_TIME_NO_SEC,
		ONLY_DATE,
		ONLY_DATE_NO_YEAR,
		TIME_DATE
	};

	DS1307_RTC();
	void setup();
	bool isRunning();
	void adjustTimeDate(TIME_DATE_T AdjustVar);
	void getTimeDate(TIME_DATE_T &RetTimeDate);
	uint8_t getDayOfTheWeek(uint16_t Year, uint8_t Month, uint8_t Day);
	uint32_t getUnixTS(TIME_DATE_T TimeDate);
	String getTimeDateStr(uint8_t FormatType);
};

#endif /* INC_DS1307RTC_H_ */

