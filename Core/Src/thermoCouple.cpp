/*
 * thermoCouple.cpp
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo
 */

#include "thermoCouple.h"


uint16_t THERMO_COUPLE::readData()
{
	uint8_t Bytes[2] = {0};
	uint16_t DataReceived = 0;
	HAL_GPIO_WritePin(ThermoCoupleCS_GPIO_Port, ThermoCoupleCS_Pin, GPIO_PIN_SET);
	HAL_SPI_Receive(&hspi1, Bytes, 2, 250);
	HAL_GPIO_WritePin(ThermoCoupleCS_GPIO_Port, ThermoCoupleCS_Pin, GPIO_PIN_RESET);
	DataReceived = Bytes[0];
	DataReceived <<= 8;
	DataReceived |= Bytes[1];
	if(DataReceived & 0x4)
	{
		DataReceived = UINT16_MAX;
	}
	else
	{
		DataReceived >>= 3;
	}
	return DataReceived;
}


float THERMO_COUPLE::readTemp()
{
	float Temp = 0.0;
	uint16_t SpiRead = readData();
	if(SpiRead != UINT16_MAX)
	{
		Temp = SpiRead * 0.25;
	}
	else
	{
		Temp = NAN;
	}
	return Temp;
}



