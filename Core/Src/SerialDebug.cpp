/*
 * SerialDebug.cpp
 *
 *  Created on: Sep 23, 2020
 *      Author: dar89
 */

#include "SerialDebug.h"

void SerialDebug::writeSerial()
{
	HAL_UART_Transmit(&huart1, serialBuffer, serialBufferLen, SEND_TIMEOUT);
}

SerialDebug::SerialDebug()
{


}

void SerialDebug::sendDbgStr(char *DbgStr)
{
	if(DbgStr != NULL)
	{
		serialBufferLen = strlen(DbgStr);
		memset(serialBuffer, 0x00, DBG_BUFFER);
		if(serialBufferLen > DBG_BUFFER)
		{
			serialBuffer = "Errore: messaggio di debug errato";
			serialBufferLen = strlen(serialBuffer);
		}
		else
		{
			memccpy(serialBuffer, DbgStr, serialBufferLen);
		}
	}
	else
	{
		serialBuffer = "Errore: messaggio mancante";
		serialBufferLen = strlen(serialBuffer);
	}
	writeSerial();
}
