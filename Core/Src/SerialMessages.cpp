/*
 * SerialDebug.cpp
 *
 *  Created on: Sep 23, 2020
 *      Author: dar89
 */

#include "SerialMessages.h"

typedef std::string String;

#define DELAY_SERIAL_ALIVE_TIMER	5000

volatile bool RxReady = false;
uint8_t RxBuffer[RECEIVE_BUFFER_LEN];
uint8_t RxBufferCopy[RECEIVE_BUFFER_LEN];


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart1)
	{
		RxReady = true;
		memcpy(RxBufferCopy, RxBuffer, RECEIVE_BUFFER_LEN);
		memset(RxBuffer, 0x00, RECEIVE_BUFFER_LEN);
//		HAL_UART_Receive_IT(&huart1, RxBuffer, RECEIVE_BUFFER_LEN);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart1)
	{
		memset(RxBuffer, 0x00, RECEIVE_BUFFER_LEN);
		memset(RxBufferCopy, 0x00, RECEIVE_BUFFER_LEN);
		HAL_UART_Receive_IT(&huart1, RxBuffer, RECEIVE_BUFFER_LEN);
	}
}

void SerialMessage::writeSerial()
{
	uint16_t BufferLen = serialBuffer.size();
	if(!RxReady)
		HAL_UART_Transmit(&huart1, (uint8_t *)serialBuffer.c_str(), BufferLen, SEND_TIMEOUT);
}


SerialMessage::SerialMessage()
{
//	rxBuffer = new uint8_t[RECEIVE_BUFFER_LEN];
	checkSerialDeviceTimer = new ChronoTimer(ChronoTimer::MILLIS);
	HAL_UART_Receive_IT(&huart1, RxBuffer, RECEIVE_BUFFER_LEN);
}

void SerialMessage::clearRxBuff()
{
	memset(RxBufferCopy, 0x00, RECEIVE_BUFFER_LEN);
}


void SerialMessage::sendMessage(String Message, bool NewLine, uint32_t SendDelay)
{
	if(!Message.empty())
	{
		if(NewLine)
			serialBuffer = Message + "\n";
		else
			serialBuffer = Message;
		writeSerial();
		if(SendDelay != 0)
		{
			HAL_Delay(SendDelay);
		}
	}
	else
	{
		serialBuffer = "Errore: messaggio mancante";
	}
}

bool SerialMessage::readSerialPolling()
{
	bool Ready = false;
	String CommandStr = "";

//	HAL_UART_Receive(&huart1, rxBuffer, RECEIVE_BUFFER_LEN, 100);
	if(RxReady)
	{
		if(RxBufferCopy[0] == '$' && (RxBufferCopy[3] == '=' || RxBufferCopy[3] == '$'))
		{
			Ready = true;
		}
		else
		{
			for(int i = 0; i < RECEIVE_BUFFER_LEN; i++)
			{
				CommandStr.push_back(RxBufferCopy[i]);
			}
			commandReceived = CommandStr;
//			clearRxBuff();
			Ready = false;
		}
		RxReady = false;
		HAL_UART_Receive_IT(&huart1, RxBuffer, RECEIVE_BUFFER_LEN);
	}
	return Ready;
}

int16_t SerialMessage::getStartStopReq(char *Req)
{
	int16_t RetVal = INVALID_MESSAGE;
	if(valueSetted == UINT32_MAX)
	{
		if((Req[0] >= '0' && Req[0] <= '9') && (Req[1] >= '0' && Req[1] <= '9'))
		{
			if(Req[0] == '0')
			{
				RetVal = START_STOP_COMMANDS_START + (Req[1] - '0');
			}
			else
			{
				RetVal = START_STOP_COMMANDS_START + (((Req[0] - '0') * 10) + (Req[1] - '0'));
			}
		}
		if((RetVal >= MAX_START_STOP_COMMANDS && RetVal == START_STOP_COMMANDS_START) || RetVal == INVALID_MESSAGE)
		{
			RetVal = INVALID_MESSAGE;
		}
	}
	return RetVal;
}

int16_t SerialMessage::getSetReq(char *Req, String Command)
{
	int16_t RetVal = INVALID_MESSAGE;
	if(valueSetted == UINT32_MAX)
	{
		if((Req[0] >= '0' && Req[0] <= '9') && (Req[1] >= '0' && Req[1] <= '9'))
		{
			if(Req[0] == '0')
			{
				RetVal = SET_COMMAND_START + (Req[1] - '0');
			}
			else
			{
				RetVal = SET_COMMAND_START + (((Req[0] - '0') * 10) + (Req[1] - '0'));
			}
		}
		if((RetVal >= MAX_SET_COMMANDS && RetVal == SET_COMMAND_START) || RetVal == INVALID_MESSAGE)
		{
			RetVal = INVALID_MESSAGE;
		}
		if(RetVal != INVALID_MESSAGE)
		{
			String ValStr = "";
			bool ZeroChecked = false;
			for(int i = 4; i < Command.length() - 1; i++)
			{
				int ValLen = Command.length() - 5;
				if(ValLen > 1)
				{
					while(i < Command.length() && !ZeroChecked)
					{
						if(Command[i] == '0')
							i++;
						else
						{
							ZeroChecked = true;
							break;
						}
					}
				}
				ValStr.push_back(Command[i]);
			}
			valueSetted = std::stoul(ValStr);
		}
	}
	return RetVal;
}


bool SerialMessage::isDeviceConnected()
{
//	bool Connected = false, GetSomething = false;
//	uint8_t Timeout = SEARCH_DEVICE_TIMEOUT_MS;
//	clearRxBuff();
	if(checkSerialDeviceTimer->isFinished(true, DELAY_SERIAL_ALIVE_TIMER))
	{
		sendMessage("$?$", true, 10);
		if(connectionRetries < MAX_CONNECTION_RETRIES)
			connectionRetries++;
	}


//	while(Timeout > 0)
//	{
//		HAL_UART_Receive(&huart1, rxBuffer, RECEIVE_BUFFER_LEN, 200);
//		if(rxBuffer[0] != 0x00)
//		{
//			GetSomething = true;
//			break;
//		}
//		Timeout--;
//		HAL_Delay(1);
//	}
	if(RxReady)
	{
		if(RxBufferCopy[0] == '$' && RxBufferCopy[1] == '!' && RxBufferCopy[2] == '$')
		{
//			GetSomething = true;
			devConnected = true;
			RxReady = false;
			sendMessage("Thanks!", true, 10);
			HAL_UART_Receive_IT(&huart1, RxBuffer, RECEIVE_BUFFER_LEN);
			clearRxBuff();
			connectionRetries = 0;
		}
		else if(RxBufferCopy[0] == '$' && (RxBufferCopy[3] == '=' || RxBufferCopy[3] == '$'))
		{
			devConnected = true;
			connectionRetries = 0;
		}
		else
		{
			RxReady = false;
			sendMessage("Who are you??", true, 10);
			HAL_UART_Receive_IT(&huart1, RxBuffer, RECEIVE_BUFFER_LEN);
			clearRxBuff();
		}
	}
	if(connectionRetries == MAX_CONNECTION_RETRIES)
	{
		devConnected = false;
	}
//	if(GetSomething)
//	{
//		if(RxBufferCopy[0] == '$' && RxBufferCopy[1] == '!' && RxBufferCopy[2] == '$')
//		{
//			devConnected = true;
//			clearRxBuff();
//		}
//		else
//		{
//			devConnected = false;
//		}
//	}

	return devConnected;
}


int16_t SerialMessage::receiveSerialCommand()
{
	String CommandStr = "";
	int16_t Command = NO_COMMANDS;
	bool InvalidMsg = false;
	char Req[MAX_REQ_LEN];
	if(readSerialPolling())
	{
		for(int i = 0; i < RECEIVE_BUFFER_LEN; i++)
		{
			bool EndMsg = false;
			if(RxBufferCopy[i] == '$' && i > 0)
			{
				EndMsg = true;
			}
			else
			{
				InvalidMsg = true;
			}
			CommandStr.push_back(RxBufferCopy[i]);
			if(EndMsg)
			{
				InvalidMsg = false;
				break;
			}
		}
		commandReceived = CommandStr;
//		memset(RxBuffer, 0x00, RECEIVE_BUFFER_LEN);
		clearRxBuff();
		if(InvalidMsg)
		{
			Command = INVALID_MESSAGE;
		}
		else
		{
			Req[0] = CommandStr[1];
			Req[1] = CommandStr[2];
			if(CommandStr[3] == '$')
			{
				Command = getStartStopReq(Req);
			}
			else
			{
				Command = getSetReq(Req, CommandStr);
			}

		}
		if(!InvalidMsg && Command != INVALID_MESSAGE)
		{
			sendMessage("$!$", true);
		}
	}
	return Command;
}

uint32_t SerialMessage::getValueSetted()
{
	uint32_t RetVal = valueSetted;
	valueSetted = UINT32_MAX;
	return RetVal;
}



std::string SerialMessage::getCommandReceived()
{
	String RetCommand = commandReceived;
	commandReceived = "";
	return RetCommand;
}
