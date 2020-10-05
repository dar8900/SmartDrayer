/*
 * SmartDryer.cpp
 *
 *  Created on: 27 set 2020
 *      Author: dar89
 */

#include <SmartDryer.h>



#define TOLLERANCE_DEGREES	55

SmartDryer::SmartDryer()
{
	display = new NHDST7565_LCD(NHDST7565_LCD::LANDSCAPE_2);
	clock = new DS1307_RTC();
	keyboard = new DryerKey();
	tempSens = new THERMO_SENSOR();
	fanCtrl = new RELE(FanCtrl_GPIO_Port, FanCtrl_Pin, LOW);
	thermoCtrl = new RELE(ThermoCtrl_GPIO_Port, ThermoCtrl_Pin, LOW);

	memory = new STM32_EEPROM();

	getTempTimer = new ChronoTimer(ChronoTimer::SECONDS);

	blinkRedLedTimer = new ChronoTimer(ChronoTimer::MILLIS);
	blinkGreenLedTimer = new ChronoTimer(ChronoTimer::MILLIS);

	testTimer = new ChronoTimer(ChronoTimer::MILLIS);

	dbgDryer = new SerialDebug();

	display->setupLcd();
}


void SmartDryer::blinkLed(uint8_t WichLed, uint16_t BlinkDelay)
{
//	GPIO_TypeDef *LedPort;
//	uint16_t LedPin = 0;
	if(BlinkDelay <= 1)
	{
		BlinkDelay = 2;
	}
	switch(WichLed)
	{
	case RED_LED:
//		LedPort = RedLed_GPIO_Port;
//		LedPin = RedLed_Pin;
		if(blinkRedLedTimer->isFinished(true, BlinkDelay))
		{
			toggleLed(WichLed);
		}
		break;
	case GREEN_LED:
		if(blinkGreenLedTimer->isFinished(true, BlinkDelay))
		{
			toggleLed(WichLed);
		}
//		LedPort = GreenLed_GPIO_Port;
//		LedPin = GreenLed_Pin;
		break;
	default:
		break;
	}

}

void SmartDryer::toggleLed(uint8_t WichLed)
{
	GPIO_TypeDef *LedPort;
	uint16_t LedPin = 0;
	switch(WichLed)
	{
	case RED_LED:
		LedPort = RedLed_GPIO_Port;
		LedPin = RedLed_Pin;
		break;
	case GREEN_LED:
		LedPort = GreenLed_GPIO_Port;
		LedPin = GreenLed_Pin;
		break;
	default:
		break;
	}
	HAL_GPIO_TogglePin(LedPort, LedPin);
}

void SmartDryer::turnOnLed(uint8_t WichLed)
{
	GPIO_TypeDef *LedPort;
	uint16_t LedPin = 0;
	switch(WichLed)
	{
	case RED_LED:
		LedPort = RedLed_GPIO_Port;
		LedPin = RedLed_Pin;
		break;
	case GREEN_LED:
		LedPort = GreenLed_GPIO_Port;
		LedPin = GreenLed_Pin;
		break;
	default:
		break;
	}
	HAL_GPIO_WritePin(LedPort, LedPin, GPIO_PIN_SET);
}

void SmartDryer::turnOffLed(uint8_t WichLed)
{
	GPIO_TypeDef *LedPort;
	uint16_t LedPin = 0;
	switch(WichLed)
	{
	case RED_LED:
		LedPort = RedLed_GPIO_Port;
		LedPin = RedLed_Pin;
		break;
	case GREEN_LED:
		LedPort = GreenLed_GPIO_Port;
		LedPin = GreenLed_Pin;
		break;
	default:
		break;
	}
	HAL_GPIO_WritePin(LedPort, LedPin, GPIO_PIN_RESET);
}

void SmartDryer::ledControl()
{

	switch(ledStatus)
	{
	case THERMO_ON_FAN_OFF:
		blinkLed(RED_LED, 10);
		turnOffLed(GREEN_LED);
		break;
	case THERMO_OFF_FAN_ON:
		turnOffLed(RED_LED);
		blinkLed(GREEN_LED, 500);
		break;
	case THERMO_OFF_FAN_OFF:
		turnOffLed(GREEN_LED);
		turnOffLed(RED_LED);
		break;
	case THERMO_ON_FAN_ON:
		blinkLed(RED_LED, 10);
		blinkLed(GREEN_LED, 500);
		break;
	case TEMP_REACHED:
		turnOnLed(RED_LED);
		HAL_Delay(250);
		break;
	case PROGRAM_INIT:
		for(int i = 0; i < 50*10; i++)
		{
			blinkLed(RED_LED, 50);
			blinkLed(GREEN_LED, 50);
			HAL_Delay(1);
		};
		break;
	case PROGRAM_END:
		for(int i = 0; i < 50*20; i++)
		{
			blinkLed(RED_LED, 50);
			blinkLed(GREEN_LED, 50);
			HAL_Delay(1);
		};
		break;
	case UNKNOWN_STATE:
	default:
		blinkLed(RED_LED, 1000);
		blinkLed(GREEN_LED, 1000);
		break;
	}
}

void SmartDryer::thermoRegulation(float WichTemp)
{
	float ReadedTemp = 0.0;
	uint32_t TempTarget = (uint32_t)roundf(WichTemp * 10);
	if(getTempTimer->isFinished(true, 2))
	{
		ReadedTemp = tempSens->getTemperature();
		readedTemperature = (uint32_t)roundf(ReadedTemp * 10);
	}
	if(readedTemperature != 0)
	{
		if(readedTemperature >= TempTarget - TOLLERANCE_DEGREES)
		{
			statusFlags.fanOn = true;
			statusFlags.thermoOn = true;
			ledStatus = THERMO_ON_FAN_ON;
		}
		else if(readedTemperature >= TempTarget)
		{
			statusFlags.fanOn = true;
			statusFlags.thermoOn = false;
			ledStatus = THERMO_OFF_FAN_ON;
		}
		else if(readedTemperature < TempTarget - TOLLERANCE_DEGREES)
		{
			statusFlags.fanOn = false;
			statusFlags.thermoOn = true;
			ledStatus = THERMO_ON_FAN_OFF;
		}
	}
	else
	{
		statusFlags.fanOn = false;
		statusFlags.thermoOn = false;
		ledStatus = THERMO_OFF_FAN_OFF;
	}
}

void SmartDryer::physicalReleCtrl()
{
	if(statusFlags.fanOn)
	{
		fanCtrl->setState(HIGH);
	}
	else
	{
		fanCtrl->setState(LOW);
	}
	if(statusFlags.thermoOn)
	{
		thermoCtrl->setState(HIGH);
	}
	else
	{
		thermoCtrl->setState(LOW);
	}
}

void SmartDryer::navMenu()
{
	bool ExitNavMenu = false;
	const char *mainMenuVoices[] =
	{
			"Thermo",
			"Ventola",
	};

	const char *OnOff[] =
	{
			"Off",
			"On",
	};

	mainMenu->menuTitle = "Menu principale";
	mainMenu->menuTitleFont = display->displayFonts[NHDST7565_LCD::W_6_H_13_B];
	mainMenu->menuVoices = mainMenuVoices;
	mainMenu->XPos = 5;
	mainMenu->YPos = 14;
	mainMenu->menuFont = display->displayFonts[NHDST7565_LCD::W_5_H_8];
	mainMenu->topItemPos = 0;
	mainMenu->itemSelected = 0;
	mainMenu->maxMenuLines = 0;
	mainMenu->maxMenuItems = sizeof(mainMenuVoices)/sizeof(mainMenuVoices[0]);
	mainMenu->withChebox = false;
	mainMenu->itemsChecked = new bool(2);
	mainMenu->paramAssociated = NULL;
	mainMenu->paramType = NO_TYPE;
	mainMenu->menuSelected = true;

	thermoMenuCtrl->menuTitle = "Ctrl. thermo";
	thermoMenuCtrl->menuTitleFont = display->displayFonts[NHDST7565_LCD::W_6_H_13_B];
	thermoMenuCtrl->menuVoices = OnOff;
	thermoMenuCtrl->XPos = 60;
	thermoMenuCtrl->YPos = 14;
	thermoMenuCtrl->menuFont = display->displayFonts[NHDST7565_LCD::W_5_H_8];
	thermoMenuCtrl->topItemPos = 0;
	thermoMenuCtrl->itemSelected = 0;
	thermoMenuCtrl->maxMenuLines = 0;
	thermoMenuCtrl->maxMenuItems = 2;
	thermoMenuCtrl->withChebox = false;
	thermoMenuCtrl->itemsChecked = new bool(2);
	thermoMenuCtrl->paramAssociated = (bool *)&statusFlags.thermoOn;
	mainMenu->paramType = PARAM_BOOL_TYPE;
	thermoMenuCtrl->menuSelected = true;

	fanMenuCtrl->menuTitle = "Menu principale";
	fanMenuCtrl->menuTitleFont = display->displayFonts[NHDST7565_LCD::W_6_H_13_B];
	fanMenuCtrl->menuVoices = OnOff;
	fanMenuCtrl->XPos = 60;
	fanMenuCtrl->YPos = 14;
	fanMenuCtrl->menuFont = display->displayFonts[NHDST7565_LCD::W_5_H_8];
	fanMenuCtrl->topItemPos = 0;
	fanMenuCtrl->itemSelected = 0;
	fanMenuCtrl->maxMenuLines = 0;
	fanMenuCtrl->maxMenuItems = 2;
	fanMenuCtrl->withChebox = false;
	fanMenuCtrl->itemsChecked = new bool(2);
	fanMenuCtrl->paramAssociated = (bool *)&statusFlags.fanOn;
	mainMenu->paramType = PARAM_BOOL_TYPE;
	fanMenuCtrl->menuSelected = true;

	MENU_STRUCTURE *ActualMenu = mainMenu;
	uint8_t MenuSel = MAIN_MENU;
	while(!ExitNavMenu)
	{
		  uint8_t WichKey = DryerKey::NO_KEY;
		  display->clearFrameBuffer();
		  ActualMenu->maxMenuLines = display->drawMenuList(ActualMenu->XPos, ActualMenu->YPos, ActualMenu->topItemPos,
				  ActualMenu->itemSelected, ActualMenu->menuVoices, ActualMenu->maxMenuItems,
				  ActualMenu->withChebox, ActualMenu->menuSelected, ActualMenu->itemsChecked, ActualMenu->menuFont);
		  display->sendFrameBuffer();
		  WichKey = keyboard->checkKey();
		  switch(WichKey)
		  {
		  case DryerKey::UP_KEY:
		  case DryerKey::LONG_UP_KEY:
			  if(ActualMenu->itemSelected > 0)
				  ActualMenu->itemSelected--;
			  else
				  ActualMenu->itemSelected = ActualMenu->maxMenuItems - 1;
			  break;
		  case DryerKey::DOWN_KEY:
		  case DryerKey::LONG_DOWN_KEY:
			  if(ActualMenu->itemSelected < ActualMenu->maxMenuItems - 1)
				  ActualMenu->itemSelected++;
			  else
				  ActualMenu->itemSelected = 0;
			  break;
		  case DryerKey::LEFT_KEY:
			  switch(MenuSel)
			  {
			  case MAIN_MENU:
				  switch(ActualMenu->itemSelected + 1)
				  {
				  case THERMO_CTRL:
					  MenuSel = THERMO_CTRL;
					  ActualMenu = thermoMenuCtrl;
					  break;
				  case FAN_CTRL:
					  MenuSel = FAN_CTRL;
					  ActualMenu = fanMenuCtrl;
					  break;
				  default:
					  break;
				  }
				  break;
			  case THERMO_CTRL:
				  MenuSel = MAIN_MENU;
				  ActualMenu = mainMenu;
				  break;
			  case FAN_CTRL:
				  MenuSel = MAIN_MENU;
				  ActualMenu = mainMenu;
				  break;
			  default:
				  break;
			  }
			  break;
		  case DryerKey::OK_KEY:
			  if(ActualMenu->withChebox)
			  {
				  for(int i = 0; i < ActualMenu->maxMenuItems; i++)
					  ActualMenu->itemsChecked[i] = false;
				  switch(ActualMenu->paramType)
				  {
				  case PARAM_BOOL_TYPE:
					  *(bool *)ActualMenu->paramAssociated = !*(bool *)ActualMenu->paramAssociated;
					  ActualMenu->itemsChecked[ActualMenu->itemSelected] = true;
					  break;
				  case PARAM_VALUE_UINT_TYPE:
					  break;
				  case PARAM_VALUE_INT_TYPE:
					  break;
				  default:
					  break;
				  }
			  }
			  break;
		  case DryerKey::LONG_LEFT_KEY:
			  break;
		  case DryerKey::LONG_OK_KEY:
			  break;
		  default:
			  break;
		  }
		  if(WichKey != DryerKey::NO_KEY && WichKey != DryerKey::LEFT_KEY)
		  {
			  if(ActualMenu->itemSelected > ActualMenu->maxMenuLines - 2)
			  {
				  if(ActualMenu->itemSelected - (ActualMenu->maxMenuLines - 2) < ActualMenu->maxMenuItems - 1)
					  ActualMenu->itemSelected = ActualMenu->itemSelected - (ActualMenu->maxMenuLines - 2);
				  else
					  ActualMenu->topItemPos = 0;
			  }
			  else
				  ActualMenu->topItemPos = 0;
			  if(ActualMenu->itemSelected >= ActualMenu->maxMenuItems - ActualMenu->maxMenuLines)
			  {
				  ActualMenu->topItemPos = ActualMenu->maxMenuItems - ActualMenu->maxMenuLines;
			  }
		  }
	}

}

void SmartDryer::test()
{
	testTimer->setTimer(5000);
	std::string Time = "";
//	display->setupLcd();

	uint8_t TopPos1 = 0, ItemSel1 = 0, TopPos2 = 0, ItemSel2 = 0, MaxLines1 = 0, MaxLines2 = 0, MaxItemsMenu1 = 8, MaxItemsMenu2 = 8;
	uint8_t *TopPosPtr = &TopPos1;
	uint8_t *ItemSelPtr = &ItemSel1;
	uint8_t *MaxLinesPtr = &MaxLines1;
	uint8_t MaxItemsMenu = MaxItemsMenu1;
	bool SwitchMenu = true;

	while(1)
	{
		  uint8_t WichKey = DryerKey::NO_KEY;
		  display->clearFrameBuffer();
		  if(SwitchMenu)
		  {
//			  MaxLines1 = display->drawMenuList(1, 12, TopPos1, ItemSel1, MenuList1, MaxItemsMenu1, false, SwitchMenu, display->displayFonts[NHDST7565_LCD::W_5_H_8]);
		  }
		  else
		  {
//			  MaxLines2 = display->drawMenuList(40, 12, TopPos2, ItemSel2, MenuList2, MaxItemsMenu2, true, !SwitchMenu, display->displayFonts[NHDST7565_LCD::W_5_H_8]);
		  }
		  display->sendFrameBuffer();
		  WichKey = keyboard->checkKey();
		  switch(WichKey)
		  {
		  case DryerKey::UP_KEY:
		  case DryerKey::LONG_UP_KEY:
			  if(*ItemSelPtr > 0)
				  (*ItemSelPtr)--;
			  else
				  (*ItemSelPtr) = MaxItemsMenu - 1;
			  break;
		  case DryerKey::DOWN_KEY:
		  case DryerKey::LONG_DOWN_KEY:
			  if(*ItemSelPtr < MaxItemsMenu - 1)
				  (*ItemSelPtr)++;
			  else
				  (*ItemSelPtr) = 0;
			  break;
		  case DryerKey::LEFT_KEY:
			  SwitchMenu = !SwitchMenu;
			  if(!SwitchMenu)
			  {
				  TopPosPtr = &TopPos2;
				  ItemSelPtr = &ItemSel2;
				  MaxLinesPtr = &MaxLines2;
				  MaxItemsMenu = MaxItemsMenu2;
			  }
			  else
			  {
				  TopPos2 = 0;
				  ItemSel2  =0;
				  MaxLines2 = 0;
				  TopPosPtr = &TopPos1;
				  ItemSelPtr = &ItemSel1;
				  MaxLinesPtr = &MaxLines1;
				  MaxItemsMenu = MaxItemsMenu1;

			  }
			  break;
		  case DryerKey::OK_KEY:
			  break;
		  case DryerKey::LONG_LEFT_KEY:

			  break;
		  case DryerKey::LONG_OK_KEY:
			  break;
		  default:
			  break;
		  }
		  if(WichKey != DryerKey::NO_KEY && WichKey != DryerKey::LEFT_KEY)
		  {
			  if(*ItemSelPtr > *MaxLinesPtr - 2)
			  {
				  if(*ItemSelPtr - ((*MaxLinesPtr) - 2) < MaxItemsMenu - 1)
					  *TopPosPtr = (*ItemSelPtr) - ((*MaxLinesPtr) - 2);
				  else
					  *TopPosPtr = 0;
			  }
			  else
				  *TopPosPtr = 0;
			  if(*ItemSelPtr >= MaxItemsMenu - *MaxLinesPtr)
			  {
				  *TopPosPtr = MaxItemsMenu - *MaxLinesPtr;
			  }
		  }
	}
}

void SmartDryer::run()
{
	while(1)
	{

	}
}

