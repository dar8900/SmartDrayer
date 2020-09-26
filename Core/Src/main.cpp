/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "Display.h"
#include "SerialDebug.h"
#include "Keyboard.h"
#include "RELE.h"
#include "DS1307RTC.h"
#include "ChronoTimer.h"

SerialDebug Dbg;
ST7920_LCD Display;
DryerKey Keyboard;
DS1307_RTC RtcClock;
ChronoTimer GetTimeTimer(ChronoTimer::MILLIS);



/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);




/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();


  /* Configure the system clock */
  SystemClock_Config();


  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();

  RtcClock.setup();
//  Display.setupLcd();



  while (1)
  {
	  uint8_t WichKey = DryerKey::NO_KEY, TestKey = 0;
	  char SerialData[RECEIVE_BUFFER_LEN] = {0};
//	  Display.testDisplay("Test");

	  Dbg.readSerialIT((uint8_t *)SerialData);
	  if(SerialData[5] != 0)
	  {
		  std::string Rec = "";
		  for(int i = 0; i < RECEIVE_BUFFER_LEN; i++)
			  Rec += SerialData[i];
		  Dbg.sendDbgStr("Ricevuto: " + Rec);
	  }

	  WichKey = Keyboard.checkKey();
	  switch(WichKey)
	  {
	  case DryerKey::UP_KEY:
		  TestKey = 1;
		  break;
	  case DryerKey::DOWN_KEY:
		  TestKey = 2;
		  break;
	  case DryerKey::LEFT_KEY:
		  TestKey = 3;
		  break;
	  case DryerKey::OK_KEY:
		  TestKey = 4;
		  break;
	  case DryerKey::LONG_UP_KEY:
		  TestKey = 5;
		  break;
	  case DryerKey::LONG_DOWN_KEY:
		  TestKey = 6;
		  break;
	  case DryerKey::LONG_LEFT_KEY:
		  TestKey = 7;
		  break;
	  case DryerKey::LONG_OK_KEY:
		  TestKey = 8;
		  break;
	  default:
		  break;
	  }
	  if(TestKey != 0)
	  {
		  Dbg.sendDbgStr("Il tasto premuto vale " + std::to_string(TestKey));
	  }

	  if(GetTimeTimer.isFinished(true, 2000))
	  {
		  Dbg.sendDbgStr(RtcClock.getTimeDateStr(DS1307_RTC::ONLY_TIME));
	  }

	  HAL_Delay(1);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
