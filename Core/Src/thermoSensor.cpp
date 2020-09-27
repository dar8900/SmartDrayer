/*
 * thermoCouple.cpp
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo
 */

#include <thermoSensor.h>
#include "i2c.h"
#include "string.h"

#define SERIAL_NUMBER_LEN	8

//==============================================================================
#define TRIGGER_T_MEASUREMENT_HM  	0XE3   	// command trig. temp meas. hold master
#define TRIGGER_RH_MEASUREMENT_HM 	0XE5  	// command trig. hum. meas. hold master
#define TRIGGER_T_MEASUREMENT_NHM 	0XF3  	// command trig. temp meas. no hold master
#define TRIGGER_RH_MEASUREMENT_NHM 	0XF5 	// command trig. hum. meas. no hold master
#define USER_REGISTER_W 			0XE6	// command writing user register
#define USER_REGISTER_R 			0XE7    // command reading user register
#define SOFT_RESET 					0XFE    // command soft reset
//==============================================================================
// HOLD MASTER - SCL line is blocked (controlled by sensor) during measurement
// NO HOLD MASTER - allows other I2C communication tasks while sensor performing
// measurements.

void THERMO_SENSOR::clearI2CBuff()
{
	memset(i2cBuffer, 0x00, BUFFER_LEN);
	i2cBufferIndex = 0;
}

uint16_t THERMO_SENSOR::readSensor_hm(uint8_t command)
{
	uint8_t checksum;
	uint8_t data[2];
	uint16_t result;
//	uint8_t n = 0;
	uint8_t d;

	if(command == TRIGGER_RH_MEASUREMENT_HM || command == TRIGGER_RH_MEASUREMENT_NHM)
		d = 30;
	if(command == TRIGGER_T_MEASUREMENT_HM || command == TRIGGER_T_MEASUREMENT_NHM)
		d = 85;

	clearI2CBuff();
	i2cBuffer[0] = command;
	HAL_I2C_Master_Transmit(&hi2c1, I2CTEMPADDR, i2cBuffer, 1, 10);
	HAL_Delay(d);
	clearI2CBuff();
	HAL_I2C_Master_Receive(&hi2c1, I2CTEMPADDR, i2cBuffer, 3, 10);
	data[0] = i2cBuffer[i2cBufferIndex++]; 	// read data (MSB)
	data[1] = i2cBuffer[i2cBufferIndex++]; 	// read data (LSB)
	checksum = i2cBuffer[i2cBufferIndex++];	// read checksum
	result = (data[0] << 8);
	result += data[1];
	if(CRC_Checksum (data, 2, checksum))
	{
		reset();
		result = 1;
	}
	clearI2CBuff();
	return result; // @suppress("Return with parenthesis")
}

float THERMO_SENSOR::CalcRH(uint16_t rh)
{
	rh &= ~0x0003;	// clean last two bits
  	return (-6.0 + 125.0/65536 * (float)rh); // return relative humidity
}

float THERMO_SENSOR::CalcT(uint16_t t)
{
	t &= ~0x0003;	// clean last two bits
	return (-46.85 + 175.72/65536 * (float)t);
}

uint8_t THERMO_SENSOR::CRC_Checksum(uint8_t data[], uint8_t no_of_bytes, uint8_t checksum)
{
	uint8_t crc = 0;
  	uint8_t byteCtr;
  	uint8_t ret = 0;

 	 //calculates 8-Bit checksum with given polynomial
  	for (byteCtr = 0; byteCtr < no_of_bytes; ++byteCtr)
 	 {
		crc ^= (data[byteCtr]);
		for (uint8_t bit = 8; bit > 0; --bit)
		{
		   if (crc & 0x80)
			   crc = (crc << 1) ^ POLYNOMIAL;
		   else
			   crc = (crc << 1);
		}
 	 }
 	 if (crc != checksum)
 		 ret = 1;
 	 else
 		 ret = 0;
 	 return ret;
}

String THERMO_SENSOR::getSerialNumber()
{

	uint8_t SerialNumber[SERIAL_NUMBER_LEN];
	String SN = "";

	clearI2CBuff();
	i2cBuffer[i2cBufferIndex++] = 0xFA;
	i2cBuffer[i2cBufferIndex++] = 0x0F;
	HAL_I2C_Master_Transmit(&hi2c1, I2CTEMPADDR, i2cBuffer, i2cBufferIndex, 10);
	clearI2CBuff();
	HAL_I2C_Master_Receive(&hi2c1, I2CTEMPADDR, i2cBuffer, 8, 10);
	SerialNumber[5] = i2cBuffer[i2cBufferIndex++];
	i2cBufferIndex++;
	SerialNumber[4] = i2cBuffer[i2cBufferIndex++];
	i2cBufferIndex++;
	SerialNumber[3] = i2cBuffer[i2cBufferIndex++];
	i2cBufferIndex++;
	SerialNumber[2] = i2cBuffer[i2cBufferIndex++];
	i2cBufferIndex++;
	clearI2CBuff();
	i2cBuffer[i2cBufferIndex++] = 0xFC;
	i2cBuffer[i2cBufferIndex++] = 0xC9;
	HAL_I2C_Master_Transmit(&hi2c1, I2CTEMPADDR, i2cBuffer, i2cBufferIndex, 10);
	clearI2CBuff();
	HAL_I2C_Master_Receive(&hi2c1, I2CTEMPADDR, i2cBuffer, 6, 10);
	SerialNumber[1] = i2cBuffer[i2cBufferIndex++];
	SerialNumber[0] = i2cBuffer[i2cBufferIndex++];
	i2cBufferIndex++;
	SerialNumber[7] = i2cBuffer[i2cBufferIndex++];
	SerialNumber[6] = i2cBuffer[i2cBufferIndex++];
	i2cBufferIndex++;
	clearI2CBuff();

	for(int i = 0; i < SERIAL_NUMBER_LEN; i++)
		SN += std::to_string(SerialNumber[i]);

	return SN; // @suppress("Return with parenthesis")

}

float THERMO_SENSOR::getHumidity(void)
{
	uint16_t result; 	// return variable

	result = readSensor_hm(TRIGGER_RH_MEASUREMENT_NHM);

	return CalcRH(result);
}

float THERMO_SENSOR::getTemperature(void)
{
	uint16_t result; 	// return variable

	result = readSensor_hm(TRIGGER_T_MEASUREMENT_NHM);

	return CalcT(result);
}

void THERMO_SENSOR::reset()
{
	clearI2CBuff();
	i2cBuffer[0] = SOFT_RESET;
	HAL_I2C_Master_Transmit(&hi2c1, I2CTEMPADDR, i2cBuffer, 1, 10);
	HAL_Delay(15);
	clearI2CBuff();
}
