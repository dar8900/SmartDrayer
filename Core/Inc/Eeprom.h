/*
 * Eeprom.h
 *
 *  Created on: 25 set 2020
 *      Author: deo
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

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



#define   U_INT8_SIZE     1
#define   U_INT16_SIZE    2
#define   U_INT32_SIZE    4
#define   U_INT64_SIZE    8
#define   FLOAT_SIZE      4
#define   DOUBLE_SIZE     8

#define   STR_SIZE  9



//################################################################################################################
bool	EE_Format(void);
bool 	EE_SingleRead(uint16_t VirtualAddress, uint32_t* Data);
bool	EE_MultiRead(uint16_t StartVirtualAddress,uint16_t HowMuchToRead,uint32_t* Data);
bool    EE_Write(uint32_t Data[]);
//################################################################################################################




#endif /* INC_EEPROM_H_ */
