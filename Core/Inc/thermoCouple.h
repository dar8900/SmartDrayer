/*
 * TermoCouple.h
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo
 */

#ifndef INC_THERMOCOUPLE_H_
#define INC_THERMOCOUPLE_H_

#include "main.h"
#include "spi.h"

class THERMO_COUPLE
{
private:

	uint16_t readData();
	float temperature;
public:
	float readTemp();
};



#endif /* INC_THERMOCOUPLE_H_ */
