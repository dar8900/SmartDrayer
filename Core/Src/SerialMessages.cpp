/*
 * SerialDebug.cpp
 *
 *  Created on: Sep 23, 2020
 *      Author: dar89
 */

#include "SerialMessages.h"

typedef std::string String;

bool RxReady = false;
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//	RxReady = true;
//}

void SerialMessage::writeSerial()
{
	uint16_t BufferLen = serialBuffer.size();
	HAL_UART_Transmit(&huart1, (uint8_t *)serialBuffer.c_str(), BufferLen, SEND_TIMEOUT);
}


SerialMessage::SerialMessage()
{
//	HAL_UART_Receive_IT(&huart1, rxBuffer, RECEIVE_BUFFER_LEN);
	rxBuffer = new uint8_t[RECEIVE_BUFFER_LEN];
}

void SerialMessage::clearRxBuff()
{
	memset(rxBuffer, 0x00, RECEIVE_BUFFER_LEN);
}


void SerialMessage::sendMessage(String Message)
{
	if(!Message.empty())
	{
		serialBuffer = Message + "\n";
		writeSerial();
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
	HAL_UART_Receive(&huart1, rxBuffer, RECEIVE_BUFFER_LEN, 100);
	if(rxBuffer[0] == '$' && (rxBuffer[3] == '=' || rxBuffer[3] == '$'))
	{
		Ready = true;
	}
	else
	{
		for(int i = 0; i < RECEIVE_BUFFER_LEN; i++)
		{
			CommandStr.push_back(rxBuffer[i]);
		}
		commandReceived = CommandStr;
		clearRxBuff();
		Ready = false;
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
		if(RetVal >= MAX_START_STOP_COMMANDS && RetVal == START_STOP_COMMANDS_START)
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
	return RetVal;
}


bool SerialMessage::isDeviceConnected()
{
	bool Connected = false, GetSomething = false;
	uint8_t Timeout = SEARCH_DEVICE_TIMEOUT_MS;
	clearRxBuff();
	sendMessage("$?$");
	while(Timeout > 0)
	{
		HAL_UART_Receive(&huart1, rxBuffer, RECEIVE_BUFFER_LEN, 10);
		if(rxBuffer[0] != 0x00)
		{
			GetSomething = true;
			break;
		}
		Timeout--;
		HAL_Delay(1);
	}
	if(GetSomething)
	{
		if(rxBuffer[0] == '$' && rxBuffer[1] == '!' && rxBuffer[2] == '$')
		{
			Connected = true;
		}
	}
	return Connected;
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
			if(rxBuffer[i] == '$' && i > 0)
			{
				EndMsg = true;
			}
			else
			{
				InvalidMsg = true;
			}
			CommandStr.push_back(rxBuffer[i]);
			if(EndMsg)
			{
				break;
			}
		}
		commandReceived = CommandStr;
		memset(rxBuffer, 0x00, RECEIVE_BUFFER_LEN);
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
