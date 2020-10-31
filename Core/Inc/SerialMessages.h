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
#include "ChronoTimer.h"



#define SEND_TIMEOUT		100
#define DBG_BUFFER			500
#define RECEIVE_BUFFER_LEN	 10

class SerialMessage
{
private:
	enum
	{
		SET_COMMAND = 0,
		START_STOP_COMMAND,
		MAX_COMMAND_TYPE
	};
	static const uint8_t MAX_CONNECTION_RETRIES = 3;
	static const uint8_t MAX_REQ_LEN = 2;
	std::string serialBuffer = "";
	std::string commandReceived = "";
	uint16_t serialBufferLen;
//	volatile uint8_t *rxBuffer;
//	volatile bool rxReady = false;
	bool devConnected = false;
	uint32_t valueSetted = UINT32_MAX;
	ChronoTimer *checkSerialDeviceTimer;
	uint8_t connectionRetries = 0;
	void ENABLE_USART_RX();
	void writeSerial();
	bool readSerialPolling();
	void clearDataBuffer();
	int16_t getStartStopReq(char *Req);
	int16_t getSetReq(char *Req, std::string Command);


public:
	enum
	{
		INVALID_MESSAGE = -2,
		NO_COMMANDS = -1,
		SERIAL_DEVICE_ATTACHED,

		SET_COMMAND_START = 10,
		SET_TEMP,
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

		START_STOP_COMMANDS_START = 200,
		DRYER_ON,
		DRYER_OFF,
		FAN_ON,
		FAN_OFF,
		START_PROG_1,
		STOP_PROG_1,
		START_PROG_2,
		STOP_PROG_2,
		START_PROG_3,
		STOP_PROG_3,
		MAX_START_STOP_COMMANDS
	};
	SerialMessage();
	void sendMessage(std::string DbgStr, bool NewLine = true, uint32_t SendDelay = 0);
	// La richiesta per capire se il device è collegato è nella forma $?$
	// Se la risposta è nella forma $!$ allora il device è collegato
	bool isDeviceConnected();
	// Trama del messaggio: ${numero comando}$ richiesta di start/stop
	// Trama del messaggio: ${numero comando}={valore}$ richiesta di settaggio
	int16_t receiveSerialCommand();
	uint32_t getValueSetted();
	std::string getCommandReceived();
};

#endif /* SERIALDEBUG_H_ */
