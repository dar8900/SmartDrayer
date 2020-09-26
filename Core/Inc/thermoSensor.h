/*
 * TermoCouple.h
 *
 *  Created on: Sep 19, 2020
 *      Author: Deo
 */

#ifndef INC_THERMOSENSOR_H_
#define INC_THERMOSENSOR_H_

#include "main.h"

class THERMO_SENSOR
{
private:

	uint16_t readData();
	float temperature;
public:
	float readTemp();
};



#endif /* INC_THERMOSENSOR_H_ */
