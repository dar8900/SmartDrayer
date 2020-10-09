/*
 * Eeprom.cpp
 *
 *  Created on: 25 set 2020
 *      Author: deo
 */




#include "main.h"
#include "Eeprom.h"
#include "string.h"


//#define   _EEPROM_F1_LOW_DESTINY
#define   _EEPROM_F1_MEDIUM_DESTINY
//#define   _EEPROM_F1_HIGH_DESTINY

#define		_EEPROM_USE_FLASH_PAGE				                 125

#ifdef  _EEPROM_F1_MEDIUM_DESTINY
#define		_EEPROM_FLASH_PAGE_SIZE								1024
/* Base address of the Flash sectors */
#define ADDR_FLASH_PAGE_0     ((uint32_t)0x08000000) /* Base @ of Page 0, 1 Kbytes */
#define _EEPROM_FLASH_PAGE_ADDRESS    (ADDR_FLASH_PAGE_0|(_EEPROM_FLASH_PAGE_SIZE*_EEPROM_USE_FLASH_PAGE))
#if (_EEPROM_USE_FLASH_PAGE>127)
#error  "Please Enter currect value _EEPROM_USE_FLASH_PAGE  (0 to 127)"
#endif
#endif

#define MAX_DIM_EEPROM_ARRAY    (_EEPROM_FLASH_PAGE_SIZE/4)
#define BLANK_VALUE             0xFFFFFFFF

#define EEPROM_VIRTUAL_ADDR_BEGIN   0
#define MAX_EEPROM_ADDRESS        255

#define   U_INT8_SIZE     1
#define   U_INT16_SIZE    2
#define   U_INT32_SIZE    4
#define   U_INT64_SIZE    8
#define   FLOAT_SIZE      4
#define   DOUBLE_SIZE     8


//##########################################################################################################

// ERASE

//##########################################################################################################
bool STM32_EEPROM::format(void)
{
	bool RetVal = false;
	uint32_t	error;
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef	flashErase;
	flashErase.NbPages=1;
	flashErase.Banks = FLASH_BANK_1;
	flashErase.PageAddress = _EEPROM_FLASH_PAGE_ADDRESS;
	flashErase.TypeErase = FLASH_TYPEERASE_PAGES;
	if(HAL_FLASHEx_Erase(&flashErase, &error) == HAL_OK)
	{
		HAL_FLASH_Lock();
		if(error != 0xFFFFFFFF)
			RetVal = false;
		else
			RetVal = true;
	}
	HAL_FLASH_Lock();
	return RetVal;
}

//##########################################################################################################

// LETTURA

//##########################################################################################################



bool STM32_EEPROM::singleRead(uint16_t VirtualAddress, uint32_t* Data)
{
	if(VirtualAddress >= (_EEPROM_FLASH_PAGE_SIZE/4))
		return false;
	*Data =  (*(uint32_t*)((VirtualAddress*4) + _EEPROM_FLASH_PAGE_ADDRESS));
	return true;
}


bool STM32_EEPROM::multiRead(uint16_t StartVirtualAddress,uint16_t HowMuchToRead, uint32_t* Data)
{
	bool RetVal = false;
	if((StartVirtualAddress+HowMuchToRead) <= (_EEPROM_FLASH_PAGE_SIZE/4))
	{
		for(uint16_t i = StartVirtualAddress ; i < (HowMuchToRead + StartVirtualAddress); i++)
		{
			*Data =  (*(uint32_t*)((i*4) + _EEPROM_FLASH_PAGE_ADDRESS));
			Data++;
		}
		RetVal = true;
	}
	return RetVal;
}

//##########################################################################################################

// SCRITTURA

//##########################################################################################################

bool STM32_EEPROM::write(uint32_t Data[])
{
    uint32_t OldValue = 0;
    bool RetVal = false;
	if(format())
	{
		HAL_FLASH_Unlock();
		for(uint16_t i = 0 ; i < (_EEPROM_FLASH_PAGE_SIZE/4); i++)
		{
			singleRead(i, &OldValue);
			if(Data[i] != OldValue)
			{
				if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (i*4) + _EEPROM_FLASH_PAGE_ADDRESS, (uint64_t)Data[i]) != HAL_OK)
				{
					HAL_FLASH_Lock();
					RetVal = false;
					break;
				}
				RetVal = true;
			}
		}
		HAL_FLASH_Lock();
	}
	return RetVal;
}

//##########################################################################################################


//##########################################################################################################

//                                      TOOLS

//##########################################################################################################


void STM32_EEPROM::transferRamToMem(uint32_t *RamVector)
{
    write(RamVector);
}

void STM32_EEPROM::tranferMemToRam(uint32_t *RamVector)
{
    multiRead(EEPROM_VIRTUAL_ADDR_BEGIN, MAX_DIM_EEPROM_ARRAY, RamVector);
}

void STM32_EEPROM::eraseMemory()
{
	format();
	tranferMemToRam(eepromArray);
    return;
}


bool STM32_EEPROM::memoryEmpty()
{
    for(uint16_t i = 0; i < MAX_DIM_EEPROM_ARRAY; i++)
    {
        if(eepromArray[i] != BLANK_VALUE)
            return false;
    }
    return true;
}


STM32_EEPROM::STM32_EEPROM()
{
	eepromArray = new uint32_t[MAX_DIM_EEPROM_ARRAY];
	tranferMemToRam(eepromArray);
}

void STM32_EEPROM::writeToMemory(uint16_t Address, uint32_t Val, bool Transfer)
{
	eepromArray[Address] = Val;
	if(Transfer)
	{
		transferRamToMem(eepromArray);
	}
}

void STM32_EEPROM::loadMemory(uint16_t Address, uint32_t *Val)
{
	tranferMemToRam(eepromArray);
	*Val = eepromArray[Address];
}
