/*
 * SerialDebug.h
 *
 *  Created on: Sep 23, 2020
 *      Author: dar89
 */

#ifndef SERIALDEBUG_H_
#define SERIALDEBUG_H_
#include "main.h"
#include "usart.h"

#include "string.h"
#include <string>




#define SEND_TIMEOUT		100
#define DBG_BUFFER			500
#define RECEIVE_BUFFER_LEN	  6

class SerialDebug
{
private:
	std::string serialBuffer;
	uint16_t serialBufferLen;
	uint8_t rxBuffer[RECEIVE_BUFFER_LEN];
	bool rxReady = false;
//	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
	void writeSerial();


public:
	SerialDebug();
	void sendDbgStr(std::string DbgStr);
	bool readSerialIT(uint8_t *Data);

};

#endif /* SERIALDEBUG_H_ */
