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
uint8_t RxData[RECEIVE_BUFFER_LEN];


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart1)
	{
		RxReady = true;
		memcpy(RxData, RxBuffer, RECEIVE_BUFFER_LEN);
		memset(RxBuffer, 0x00, RECEIVE_BUFFER_LEN);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart1)
	{
		memset(RxBuffer, 0x00, RECEIVE_BUFFER_LEN);
		memset(RxData, 0x00, RECEIVE_BUFFER_LEN);
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
	checkSerialDeviceTimer = new ChronoTimer(ChronoTimer::MILLIS);
	HAL_UART_Receive_IT(&huart1, RxBuffer, RECEIVE_BUFFER_LEN);
}

void SerialMessage::clearDataBuffer()
{
	memset(RxData, 0x00, RECEIVE_BUFFER_LEN);
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
		if(RxData[0] == '$' && (RxData[3] == '=' || RxData[3] == '$'))
		{
			Ready = true;
		}
		else
		{
			for(int i = 0; i < RECEIVE_BUFFER_LEN; i++)
			{
				CommandStr.push_back(RxData[i]);
			}
			commandReceived = CommandStr;
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
		if(RetVal >= MAX_SET_COMMANDS && RetVal == SET_COMMAND_START)
		{
			RetVal = INVALID_MESSAGE;
		}
		if(RetVal != INVALID_MESSAGE)
		{
			String ValStr = "";
			bool ZeroChecked = false;
			int ValueLen = Command.length() - 5;
			for(uint16_t i = 4; i < Command.length() - 1; i++)
			{
				if(ValueLen > 1  && !ZeroChecked)
				{
					while(i < Command.length() - 1)
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
				else if(ValueLen == 1 && Command[i] == '0')
				{
					ZeroChecked = true;
				}
				if(ZeroChecked)
				{
					ValStr.push_back(Command[i]);
				}
			}
			if(!ValStr.empty())
			{
				valueSetted = std::stoul(ValStr);
			}
			else
			{
				RetVal = INVALID_MESSAGE;
			}
		}
	}
	return RetVal;
}


bool SerialMessage::isDeviceConnected()
{
	if(checkSerialDeviceTimer->isFinished(true, DELAY_SERIAL_ALIVE_TIMER))
	{
		sendMessage("$?$", true, 10);
		if(connectionRetries < MAX_CONNECTION_RETRIES)
			connectionRetries++;
	}
	if(RxReady)
	{
		if(RxData[0] == '$' && RxData[1] == '!' && RxData[2] == '$')
		{
			devConnected = true;
			RxReady = false;
			sendMessage("Thanks!", true, 10);
			HAL_UART_Receive_IT(&huart1, RxBuffer, RECEIVE_BUFFER_LEN);
			clearDataBuffer();
			connectionRetries = 0;
		}
		else if(RxData[0] == '$' && (RxData[3] == '=' || RxData[3] == '$'))
		{
			devConnected = true;
			connectionRetries = 0;
		}
		else
		{
			RxReady = false;
			sendMessage("Who are you??", true, 10);
			HAL_UART_Receive_IT(&huart1, RxBuffer, RECEIVE_BUFFER_LEN);
			clearDataBuffer();
		}
	}
	if(connectionRetries == MAX_CONNECTION_RETRIES)
	{
		devConnected = false;
	}
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
			if(RxData[i] == '$' && i > 0)
			{
				EndMsg = true;
			}
			else
			{
				InvalidMsg = true;
			}
			CommandStr.push_back(RxData[i]);
			if(EndMsg)
			{
				InvalidMsg = false;
				break;
			}
		}
		commandReceived = CommandStr;
		clearDataBuffer();
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
