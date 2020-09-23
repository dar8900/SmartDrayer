/*
 * SerialDebug.h
 *
 *  Created on: Sep 23, 2020
 *      Author: dar89
 */

#ifndef SERIALDEBUG_H_
#define SERIALDEBUG_H_
#include "main.h"

#define SEND_TIMEOUT	100
#define DBG_BUFFER		500

class SerialDebug
{
private:
	char serialBuffer[DBG_BUFFER];
	uint16_t serialBufferLen;
	void writeSerial();

public:
	SerialDebug();
	void sendDbgStr(char *DbgStr);
};

#endif /* SERIALDEBUG_H_ */
