/*
 * thermoCouple.cpp
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo
 */

#include <thermoSensor.h>
#include "i2c.h"
#include "string.h"

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
	uint8_t n = 0;
	uint8_t d;

	if(command == TRIGGER_RH_MEASUREMENT_HM || command == TRIGGER_RH_MEASUREMENT_NHM) d = 30;
	if(command == TRIGGER_T_MEASUREMENT_HM || command == TRIGGER_T_MEASUREMENT_NHM) d = 85;

//	Wire.beginTransmission(I2C_ADD);
//	Wire.write(command);
//	Wire.endTransmission();
//	delay(d);
//	Wire.requestFrom(I2C_ADD,3);
	clearI2CBuff();
	HAL_I2C_Master_Transmit(&hi2c1, I2CTEMPADDR, &command, 1, 10);
	HAL_Delay(d);
	HAL_I2C_Master_Receive(&hi2c1, I2CTEMPADDR, i2cBuffer, 3, 10);

//	while(Wire.available() < 3)
//	{
//		delay(10);
//		n++;
//		if(n>10)
//		{
//			return 0;
//		}
//	}

//	data[0] = Wire.read(); 	// read data (MSB)
//	data[1] = Wire.read(); 	// read data (LSB)
//	checksum = Wire.read();	// read checksum

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
//	Wire.beginTransmission(I2C_ADD);
//	Wire.write(SOFT_RESET);
//	Wire.endTransmission();
//
//	delay(15);	// wait for SHT to reset
}

uint8_t THERMO_SENSOR::getSerialNumber(uint8_t return_sn)
{

	uint8_t serialNumber[8];

	// read memory location 1
//	Wire.beginTransmission(I2C_ADD);
//	Wire.write(0xFA);
//	Wire.write(0x0F);
//	Wire.endTransmission();
//
//	Wire.requestFrom(I2C_ADD,8);
//	while(Wire.available() < 8) {}
//
//	serialNumber[5] = Wire.read();	// read SNB_3
//	Wire.read();					// CRC SNB_3 not used
//	serialNumber[4] = Wire.read();  // read SNB_2
//	Wire.read();					// CRC SNB_2 not used
//	serialNumber[3] = Wire.read();	// read SNB_1
//	Wire.read();					// CRC SNB_1 not used
//	serialNumber[2] = Wire.read();	// read SNB_0
//	Wire.read();					// CRC SNB_0 not used
//
//	// read memory location 2
//	Wire.beginTransmission(I2C_ADD);
//	Wire.write(0xFC);
//	Wire.write(0xC9);
//	Wire.endTransmission();
//
//	Wire.requestFrom(I2C_ADD,6);
//	while(Wire.available() < 6) {}
//
//	serialNumber[1] = Wire.read();	// read SNC_1
//	serialNumber[0] = Wire.read();  // read SNC_0
//	Wire.read();					// CRC SNC_1/SNC_0 not used
//	serialNumber[7] = Wire.read();	// read SNA_1
//	serialNumber[6] = Wire.read();	// read SNA_0
//	Wire.read();					// CRC SNA_1/SNA_0 not used

	return serialNumber[return_sn];
}
