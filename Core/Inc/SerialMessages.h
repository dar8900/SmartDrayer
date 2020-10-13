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
#define RECEIVE_BUFFER_LEN	 25

class SerialMessage
{
private:
	enum
	{
		SET_COMMAND = 0,
		START_STOP_COMMAND,
		MAX_COMMAND_TYPE
	};
	static const uint8_t MAX_REQ_LEN = 2;
	std::string serialBuffer = "";
	std::string commandReceived = "";
	uint16_t serialBufferLen;
	uint8_t *rxBuffer;
	bool rxReady = false;
	uint32_t valueSetted = UINT32_MAX;
	void writeSerial();
	bool readSerialPolling();
	int16_t getStartStopReq(char *Req);
	int16_t getSetReq(char *Req, std::string Command);

public:
	enum
	{
		INVALID_MESSAGE = -2,
		NO_COMMANDS = -1,

		SET_COMMAND_START = 0,
		SET_TEMP,
		SET_FAN,
		SET_START_MINUTE_PROG_1,
		SET_START_HOUR_PROG_1,
		SET_END_MINUTE_PROG_1,
		SET_END_HOUR_PROG_1,
		SET_TEMP_PROG_1,
		SET_START_MINUTE_PROG_2,
		SET_START_HOUR_PROG_2,
		SET_END_MINUTE_PROG_2,
		SET_END_HOUR_PROG_2,
		SET_TEMP_PROG_2,
		SET_START_MINUTE_PROG_3,
		SET_START_HOUR_PROG_3,
		SET_END_MINUTE_PROG_3,
		SET_END_HOUR_PROG_3,
		SET_TEMP_PROG_3,
		MAX_SET_COMMANDS,

		START_STOP_COMMANDS_START = 100,
		DRYER_ON,
		DRYER_OFF,
		START_PROG_1,
		STOP_PROG_1,
		START_PROG_2,
		STOP_PROG_2,
		START_PROG_3,
		STOP_PROG_3,
		MAX_START_STOP_COMMANDS
	};
	SerialMessage();
	void sendMessage(std::string DbgStr);
	int16_t receiveSerialCommand();
	uint32_t getValueSetted();
	std::string getCommandReceived();
};

#endif /* SERIALDEBUG_H_ */
