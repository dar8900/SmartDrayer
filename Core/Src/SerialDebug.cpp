/*
 * SerialDebug.cpp
 *
 *  Created on: Sep 23, 2020
 *      Author: dar89
 */

#include "SerialDebug.h"

void SerialDebug::writeSerial()
{
	uint16_t BufferLen = serialBuffer.size();
	HAL_UART_Transmit(&huart1, (uint8_t *)serialBuffer.c_str(), BufferLen, SEND_TIMEOUT);
}

SerialDebug::SerialDebug()
{


}

void SerialDebug::sendDbgStr(std::string DbgStr)
{
	if(!DbgStr.empty())
	{
		serialBuffer = DbgStr;
	}
	else
	{
		serialBuffer = "Errore: messaggio mancante";
	}
	writeSerial();
}
