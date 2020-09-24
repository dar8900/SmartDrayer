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
#include <string>




#define SEND_TIMEOUT	100
#define DBG_BUFFER		500

class SerialDebug
{
private:
	std::string serialBuffer;
	uint16_t serialBufferLen;
	void writeSerial();

public:
	SerialDebug();
	void sendDbgStr(std::string DbgStr);
};

#endif /* SERIALDEBUG_H_ */
