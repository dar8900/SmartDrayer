/*
 * TermoCouple.h
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo
 */

#ifndef INC_THERMOSENSOR_H_
#define INC_THERMOSENSOR_H_

#include "main.h"

#include <string>

typedef std::string String;


class THERMO_SENSOR
{
private:
	static const uint8_t BUFFER_LEN = 32;
	static const uint8_t I2CTEMPADDR = (0x40 << 1);
	static const uint16_t POLYNOMIAL = 0x131; // P(x)=x^8+x^5+x^4+1 = 100110001
	uint8_t i2cBuffer[BUFFER_LEN];
	uint8_t i2cBufferIndex = 0;
	void clearI2CBuff();

	//==============================================================================
	uint16_t readSensor_hm(uint8_t command);
	//==============================================================================
	// reads SHT21 with hold master operation mode
	// input:	temp/hum command
	// return:	temp/hum raw data (16bit scaled)


	//==============================================================================
	float CalcRH(uint16_t rh);
	//==============================================================================
	// calculates the relative humidity
	// input:  rh:	 humidity raw value (16bit scaled)
	// return:		 relative humidity [%RH] (float)

	//==============================================================================
	float CalcT(uint16_t t);
	//==============================================================================
	// calculates the temperature
	// input:  t: 	temperature raw value (16bit scaled)
	// return:		relative temperature [°C] (float)

	//==============================================================================
	uint8_t CRC_Checksum(uint8_t data[], uint8_t no_of_bytes, uint8_t checksum);
	//==============================================================================
	// CRC-8 checksum for error detection
	// input:  data[]       checksum is built based on this data
	//         no_of_bytes  checksum is built for n bytes of data
	//         checksum     expected checksum
	// return:              1 			   = checksum does not match
	//                      0              = checksum matches
public:
	//==============================================================================
	float getHumidity(void);
	//==============================================================================
	// calls humidity measurement with hold master mode

	//==============================================================================
	float getTemperature(void);
	//==============================================================================
	// calls temperature measurement with hold master mode

	//==============================================================================
	void reset();
	//==============================================================================
	// performs a soft reset, delays 15ms

	//==============================================================================
	String getSerialNumber();
	//==============================================================================
	// returns electronical identification code depending of selected memory
	// location
};



#endif /* INC_THERMOSENSOR_H_ */
