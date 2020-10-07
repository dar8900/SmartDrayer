/*
 * DS1307RTC.cpp
 *
 *  Created on: 25 set 2020
 *      Author: deo
 */

#include <DS1307RTC.h>
#include "string.h"

const uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30};
const char *monthNames[] =
{
		"Gennaio",
		"Febbraio",
		"Marzo",
		"Aprile",
		"Maggio",
		"Giugno",
		"Luglio",
		"Agosto",
		"Settembre",
		"Ottobre",
		"Novembre",
		"Dicembre",
};

/**************************************************************************/
/*!
    @brief  Convert a binary coded decimal value to binary. RTC stores time/date
   values as BCD.
    @param val BCD value
    @return Binary value
*/
/**************************************************************************/
static uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }

/**************************************************************************/
/*!
    @brief  Convert a binary value to BCD format for the RTC registers
    @param val Binary value
    @return BCD value
*/
/**************************************************************************/
static uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }



/**************************************************************************/
/*!
    @brief  Given a date, return number of days since 2000/01/01,
            valid for 2000--2099
    @param y Year
    @param m Month
    @param d Day
    @return Number of days
*/
/**************************************************************************/
uint16_t DS1307_RTC::date2days(uint16_t y, uint8_t m, uint8_t d)
{
  if (y >= 2000)
    y -= 2000;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += daysInMonth[i - 1];
  if (m > 2 && y % 4 == 0)
    ++days;
  return (days + 365 * y + (y + 3) / 4 - 1);
}

/**************************************************************************/
/*!
    @brief  Given a number of days, hours, minutes, and seconds, return the
   total seconds
    @param days Days
    @param h Hours
    @param m Minutes
    @param s Seconds
    @return Number of seconds total
*/
/**************************************************************************/
uint32_t DS1307_RTC::time2ulong(uint16_t days, uint8_t h, uint8_t m, uint8_t s)
{
  return (((days * 24UL + h) * 60 + m) * 60 + s);
}

/**************************************************************************/
/*!
    @brief  Return the day of the week.
    @return Day of week as an integer from 0 (Sunday) to 6 (Saturday).
*/
/**************************************************************************/
uint8_t DS1307_RTC::getDayOfTheWeek(uint16_t Year, uint8_t Month, uint8_t Day)
{
  uint16_t day = date2days(Year, Month, Day);
  return ((day + 6) % 7); // Jan 1, 2000 is a Saturday, i.e. returns 6
}

/**************************************************************************/
/*!
    @brief  Return Unix time: seconds since 1 Jan 1970.

    @see The `DateTime::DateTime(uint32_t)` constructor is the converse of
        this method.

    @return Number of seconds since 1970-01-01 00:00:00.
*/
/**************************************************************************/
uint32_t DS1307_RTC::getUnixTS(TIME_DATE_T TimeDate)
{
  uint32_t t;
  uint16_t days = date2days(TimeDate.year, TimeDate.month, TimeDate.day);
  t = time2ulong(days, TimeDate.hour, TimeDate.minute, TimeDate.second);
  t += 946684800; // seconds from 1970 to 2000
  return t; // @suppress("Return with parenthesis")
}


uint16_t DS1307_RTC::devAddressLShifted()
{
	return ds1307Address << 1;
}

void DS1307_RTC::clearI2CBuff()
{
	memset(i2cBuffer, 0x00, BUFFER_LEN);
	i2cBufferIndex = 0;
}

DS1307_RTC::DS1307_RTC()
{

}

void DS1307_RTC::setup()
{
	clearI2CBuff();
	HAL_I2C_Master_Transmit(&hi2c1, devAddressLShifted(), i2cBuffer, 0, 1);
}



bool DS1307_RTC::isRunning()
{
	bool Live = false;
	uint8_t RxData = 0;
	HAL_I2C_Master_Transmit(&hi2c1, devAddressLShifted(), 0, 1, 1);
	HAL_I2C_Master_Receive(&hi2c1, devAddressLShifted(), i2cBuffer, 1, 1);
	RxData = i2cBuffer[0];
	clearI2CBuff();
	if(RxData >> 7 == 0x00)
	{
		Live = true;
	}
	return Live;
}



void DS1307_RTC::adjustTimeDate(TIME_DATE_T AdjustVar)
{
	clearI2CBuff();
	i2cBuffer[i2cBufferIndex++] = 0;
	i2cBuffer[i2cBufferIndex++] = bin2bcd(AdjustVar.second);
	i2cBuffer[i2cBufferIndex++] = bin2bcd(AdjustVar.minute);
	i2cBuffer[i2cBufferIndex++] = bin2bcd(AdjustVar.hour);
	i2cBuffer[i2cBufferIndex++] = bin2bcd(0);
	i2cBuffer[i2cBufferIndex++] = bin2bcd(AdjustVar.day);
	i2cBuffer[i2cBufferIndex++] = bin2bcd(AdjustVar.month);
	i2cBuffer[i2cBufferIndex++] = bin2bcd(AdjustVar.year - 2000);
	HAL_I2C_Master_Transmit(&hi2c1, devAddressLShifted(), i2cBuffer, i2cBufferIndex, 1);
	clearI2CBuff();
}

void DS1307_RTC::getTimeDate(TIME_DATE_T &RetTimeDate)
{
	clearI2CBuff();
	HAL_I2C_Master_Transmit(&hi2c1, devAddressLShifted(), 0, 1, 1);
	HAL_I2C_Master_Receive(&hi2c1, devAddressLShifted(), i2cBuffer, 7, 1);
	RetTimeDate.second = bcd2bin(i2cBuffer[i2cBufferIndex++] & 0x7F);
	RetTimeDate.minute = bcd2bin(i2cBuffer[i2cBufferIndex++]);
	RetTimeDate.hour = bcd2bin(i2cBuffer[i2cBufferIndex++]);
	i2cBufferIndex++;
	RetTimeDate.day = bcd2bin(i2cBuffer[i2cBufferIndex++]);
	RetTimeDate.month = bcd2bin(i2cBuffer[i2cBufferIndex++]);
	RetTimeDate.year = bcd2bin(i2cBuffer[i2cBufferIndex++]) + 2000;
	clearI2CBuff();
}

String DS1307_RTC::getTimeDateStr(uint8_t FormatType)
{
	String second, minute, hour, day, month, year, RetStr = "Wrong type";
	TIME_DATE_T TimeDate;
	getTimeDate(TimeDate);
	second = TimeDate.second > 9 ? std::to_string(TimeDate.second) : "0" + std::to_string(TimeDate.second);
	minute = TimeDate.minute > 9 ? std::to_string(TimeDate.minute) : "0" + std::to_string(TimeDate.minute);
	hour = TimeDate.hour > 9 ? std::to_string(TimeDate.hour) : "0" + std::to_string(TimeDate.hour);
	day = TimeDate.day > 9 ? std::to_string(TimeDate.day) : "0" + std::to_string(TimeDate.day);
	month = TimeDate.month > 9 ? std::to_string(TimeDate.month) : "0" + std::to_string(TimeDate.month);
	year = std::to_string(TimeDate.year % 100);
	switch(FormatType)
	{
	case ONLY_TIME:
		RetStr = hour + ":" + minute + ":" + second;
		break;
	case ONLY_TIME_NO_SEC:
		RetStr = hour + ":" + minute;
		break;
	case ONLY_DATE:
		RetStr = day + "/" + month + "/" + year;
		break;
	case ONLY_DATE_NO_YEAR:
		RetStr = day + "/" + month;
		break;
	case TIME_DATE:
		RetStr = hour + ":" + minute + ":" + second + " " + day + "/" + month + "/" + year;
		break;
	default:
		break;
	}
	return RetStr;
}
