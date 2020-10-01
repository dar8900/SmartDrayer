/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

#include "../LcdLib/u8g2.h"
#include "math.h"
#include "stdint.h"
#include "stdbool.h"


typedef enum
{
	LOW = 0,
	HIGH
}PIN_STATE;


/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);


/* Private defines -----------------------------------------------------------*/
#define CurrentSensor_Pin GPIO_PIN_0
#define CurrentSensor_GPIO_Port GPIOA
#define ThermoSensor_Pin GPIO_PIN_2
#define ThermoSensor_GPIO_Port GPIOA
#define LcdCS_Pin GPIO_PIN_3
#define LcdCS_GPIO_Port GPIOA
#define LcdA0_Pin GPIO_PIN_4
#define LcdA0_GPIO_Port GPIOA
#define Sck_Pin GPIO_PIN_5
#define Sck_GPIO_Port GPIOA
#define LcdReset_Pin GPIO_PIN_6
#define LcdReset_GPIO_Port GPIOA
#define Mosi_Pin GPIO_PIN_7
#define Mosi_GPIO_Port GPIOA
#define UpButton_Pin GPIO_PIN_12
#define UpButton_GPIO_Port GPIOB
#define DownButton_Pin GPIO_PIN_13
#define DownButton_GPIO_Port GPIOB
#define LeftButton_Pin GPIO_PIN_14
#define LeftButton_GPIO_Port GPIOB
#define OkButton_Pin GPIO_PIN_15
#define OkButton_GPIO_Port GPIOB
#define ThermoCtrl_Pin GPIO_PIN_9
#define ThermoCtrl_GPIO_Port GPIOA
#define FanCtrl_Pin GPIO_PIN_10
#define FanCtrl_GPIO_Port GPIOA
#define RedLed_Pin GPIO_PIN_11
#define RedLed_GPIO_Port GPIOA
#define GreenLed_Pin GPIO_PIN_12
#define GreenLed_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define TX_Pin GPIO_PIN_6
#define TX_GPIO_Port GPIOB
#define RX_Pin GPIO_PIN_7
#define RX_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_8
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_9
#define I2C_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
