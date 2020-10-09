/*
 * Eeprom.h
 *
 *  Created on: 25 set 2020
 *      Author: deo
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#include "main.h"

#define NO_MEM_TRANSFER false
#define MEM_TRANSFER    true

class STM32_EEPROM
{
private:
	uint32_t *eepromArray;
	bool	format(void);
	bool 	singleRead(uint16_t VirtualAddress, uint32_t* Data);
	bool	multiRead(uint16_t StartVirtualAddress,uint16_t HowMuchToRead,uint32_t* Data);
	bool    write(uint32_t Data[]);

	void transferRamToMem(uint32_t *RamVector);
	void tranferMemToRam(uint32_t *RamVector);
	uint32_t eepromArrayIndex = 0;

public:
	STM32_EEPROM();
	void eraseMemory();
	bool memoryEmpty();
	void writeToMemory(uint16_t Address, uint32_t Val, bool Transfer);
	void loadMemory(uint16_t Address, uint32_t *Val);

};


#endif /* INC_EEPROM_H_ */
