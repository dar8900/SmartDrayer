/*
 * SerialDebug.cpp
 *
 *  Created on: Sep 23, 2020
 *      Author: dar89
 */

#include "SerialDebug.h"

bool RxReady = false;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	RxReady = true;
}

void SerialDebug::writeSerial()
{
	uint16_t BufferLen = serialBuffer.size();
	HAL_UART_Transmit(&huart1, (uint8_t *)serialBuffer.c_str(), BufferLen, SEND_TIMEOUT);
}


SerialDebug::SerialDebug()
{
	HAL_UART_Receive_IT(&huart1, rxBuffer, RECEIVE_BUFFER_LEN);
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

bool SerialDebug::readSerialIT(uint8_t *Data)
{
	bool Ready = false;
	if(RxReady)
	{
		Ready = true;
		memcpy(Data, rxBuffer, RECEIVE_BUFFER_LEN);
		memset(rxBuffer, 0x00, RECEIVE_BUFFER_LEN);
		RxReady = false;
	}
	return Ready;
}
