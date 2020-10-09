/*
 * SmartDryer.cpp
 *
 *  Created on: 27 set 2020
 *      Author: dar89
 */

#include <SmartDryer.h>

#include <string.h>

#define MENU_LEFT_LIST_XPOS			  5
#define MENU_RIGHT_LIST_XPOS		 60
#define MENU_LIST_YPOS				 20
#define HELP_MESSAGE_MARGIN		     (display->dispParams.width - MENU_RIGHT_LIST_XPOS)
#define HELP_MESSAGE_YPOS		     25
#define MENU_TITLE_YPOS			 	  7


#define TOLLERANCE_DEGREES	55




const char *mainMenuVoices[] =
{
		"Thermo",
		"Ventola",
		"Imposta temp.",
		"Avvia dryer",
		"Imposta ora",
		"Imposta data",
		"Imposta programma",
		"Avvia programma",
		"Mostra info",
};

const char *OnOff[] =
{
		"Off",
		"On",
};

const char *Temps[] =
{
		"30",
		"35",
		"40",
		"45",
		"50",
		"55",
		"60",
		"65",
		"70",
		"75",
		"80",
		"85",
		"90",
};

const char *programsMenuVoices[MAX_PROGRAMS] =
{
		"Programma 1",
		"Programma 2",
		"Programma 3",
};



SmartDryer::SmartDryer()
{
	display = new NHDST7565_LCD(NHDST7565_LCD::LANDSCAPE_2);
	keyboard = new DryerKey();
	tempSens = new THERMO_SENSOR();
	fanCtrl = new RELE(FanCtrl_GPIO_Port, FanCtrl_Pin, LOW);
	thermoCtrl = new RELE(ThermoCtrl_GPIO_Port, ThermoCtrl_Pin, LOW);

	memory = new STM32_EEPROM();

	getTempTimer = new ChronoTimer(ChronoTimer::MILLIS);

	blinkRedLedTimer = new ChronoTimer(ChronoTimer::MILLIS);
	blinkGreenLedTimer = new ChronoTimer(ChronoTimer::MILLIS);
	takeTimeTimer = new ChronoTimer(ChronoTimer::MILLIS);
	showHelpMessageTimer = new ChronoTimer(ChronoTimer::SECONDS);
	programStartedTimer = new ChronoTimer(ChronoTimer::MILLIS);

	testTimer = new ChronoTimer(ChronoTimer::MILLIS);

	dbgDryer = new SerialDebug();

	clock = new DS1307_RTC();

	mainMenu = new MENU_STRUCTURE();
	thermoMenuCtrl = new MENU_STRUCTURE();
	fanMenuCtrl = new MENU_STRUCTURE();
	tempMenuCtrl = new MENU_STRUCTURE();
	startDryerMenu = new MENU_STRUCTURE();
	changeProgramsMenu = new MENU_STRUCTURE();
	startProgramsMenu = new MENU_STRUCTURE();

	statusParam = new DRYER_PARAMS();

	chamberTemperature.readedValueInt = 0;
}

String SmartDryer::floatString(float Number, uint8_t NDecimal)
{
	char CharArray[6];
	char Format[6];
	snprintf(Format, 6, "%%.%df", NDecimal);
	snprintf(CharArray, 6, Format, Number);
	return String(CharArray);
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
			ledStatus = UNKNOWN_STATE;
			break;
		case PROGRAM_INIT:
			for(int i = 0; i < 50*10; i++)
			{
				blinkLed(RED_LED, 50);
				blinkLed(GREEN_LED, 50);
				HAL_Delay(1);
			};
			ledStatus = UNKNOWN_STATE;
			break;
		case PROGRAM_END:
			for(int i = 0; i < 50*20; i++)
			{
				blinkLed(RED_LED, 50);
				blinkLed(GREEN_LED, 50);
				HAL_Delay(1);
			};
			ledStatus = UNKNOWN_STATE;
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
	uint32_t ReadedTemp = 0;
	uint32_t TempTarget = (uint32_t)roundf(WichTemp * 10);
	bool Reading = false;
	if(statusParam->dryerOn)
	{
		if(getTempTimer->isFinished(true, 750))
		{
			chamberTemperature.readedValueFL = tempSens->getTemperature();
			chamberHumity.readedValueFL = tempSens->getHumidity();
			ReadedTemp = (uint32_t)roundf(chamberTemperature.readedValueFL * 10);
			Reading = true;
		}
		if(ReadedTemp != 0 && Reading)
		{
			if(ReadedTemp >= TempTarget - TOLLERANCE_DEGREES)
			{
				statusParam->fanOn = true;
				statusParam->thermoOn = true;
				if(ledStatus != PROGRAM_INIT && ledStatus != PROGRAM_END)
					ledStatus = THERMO_ON_FAN_ON;
			}
			else if(ReadedTemp >= TempTarget)
			{
				statusParam->fanOn = true;
				statusParam->thermoOn = false;
				ledStatus = TEMP_REACHED;
				if(ledStatus != PROGRAM_INIT && ledStatus != PROGRAM_END && ledStatus != TEMP_REACHED)
					ledStatus = THERMO_OFF_FAN_ON;
			}
			else if(ReadedTemp < TempTarget - TOLLERANCE_DEGREES)
			{
				statusParam->fanOn = false;
				statusParam->thermoOn = true;
				if(ledStatus != PROGRAM_INIT && ledStatus != PROGRAM_END && ledStatus != TEMP_REACHED)
					ledStatus = THERMO_ON_FAN_OFF;
			}
		}
//		else
//		{
//			statusParam->fanOn = false;
//			statusParam->thermoOn = false;
//	//		statusParam->dryerOn = false;
//			if(ledStatus != PROGRAM_INIT && ledStatus != PROGRAM_END && ledStatus != TEMP_REACHED)
//				ledStatus = THERMO_OFF_FAN_OFF;
//		}
	}
	else
	{
		statusParam->fanOn = false;
		statusParam->thermoOn = false;
		if(ledStatus != PROGRAM_INIT && ledStatus != PROGRAM_END && ledStatus != TEMP_REACHED)
			ledStatus = THERMO_OFF_FAN_OFF;
		if(getTempTimer->isFinished(true, 750))
		{
			chamberTemperature.readedValueFL = tempSens->getTemperature();
			chamberHumity.readedValueFL = tempSens->getHumidity();
		}
	}
}

void SmartDryer::physicalReleCtrl()
{
	if(statusParam->dryerOn)
	{
		if(statusParam->fanOn)
		{
			fanCtrl->setState(HIGH);
		}
		else
		{
			fanCtrl->setState(LOW);
		}
		if(statusParam->thermoOn)
		{
			thermoCtrl->setState(HIGH);
		}
		else
		{
			thermoCtrl->setState(LOW);
		}
	}
	else
	{
		fanCtrl->setState(LOW);
		thermoCtrl->setState(LOW);
	}
}

void SmartDryer::peripheralsControl()
{
	thermoRegulation(statusParam->temperatureSetted);
	physicalReleCtrl();
	ledControl();
}


void SmartDryer::showTimeDate(String &Time, String &Date)
{
	if(takeTimeTimer->isFinished(true, 500))
	{
		Time = clock->getTimeDateStr(DS1307_RTC::ONLY_TIME_NO_SEC);
		Date = clock->getTimeDateStr(DS1307_RTC::ONLY_DATE_NO_YEAR);
	}
	display->drawTimeDate(Time, Date);
}

void SmartDryer::writeDefaultsDryerProgram(uint8_t WichProgram)
{
	dryerPrograms[WichProgram].programSetted = false;
	dryerPrograms[WichProgram].startTime.second = 0;
	dryerPrograms[WichProgram].startTime.minute = 0;
	dryerPrograms[WichProgram].startTime.hour = 0;
	dryerPrograms[WichProgram].startTime.day = 1;
	dryerPrograms[WichProgram].startTime.month = 1;
	dryerPrograms[WichProgram].startTime.year = 2020;
	dryerPrograms[WichProgram].endTime.second = 0;
	dryerPrograms[WichProgram].endTime.minute = 10;
	dryerPrograms[WichProgram].endTime.hour = 0;
	dryerPrograms[WichProgram].endTime.day = 1;
	dryerPrograms[WichProgram].endTime.month = 1;
	dryerPrograms[WichProgram].endTime.year = 2020;
	dryerPrograms[WichProgram].tempSetted = 30.0;
}

void SmartDryer::saveDryerProgram(uint8_t WichProgram)
{
	const uint8_t MAX_DRYER_VAR = 14;
	uint32_t TempToSave = 0;
	TempToSave = (uint32_t)dryerPrograms[WichProgram].tempSetted;
	memory->writeToMemory(SETTED_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].programSetted, NO_MEM_TRANSFER);
	memory->writeToMemory(START_SECOND_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].startTime.second, NO_MEM_TRANSFER);
	memory->writeToMemory(START_MINUTE_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].startTime.minute, NO_MEM_TRANSFER);
	memory->writeToMemory(START_HOUR_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].startTime.hour, NO_MEM_TRANSFER);
	memory->writeToMemory(START_DAY_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].startTime.day, NO_MEM_TRANSFER);
	memory->writeToMemory(START_MONTH_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].startTime.month, NO_MEM_TRANSFER);
	memory->writeToMemory(START_YEAR_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].startTime.year, NO_MEM_TRANSFER);
	memory->writeToMemory(END_SECOND_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].endTime.second, NO_MEM_TRANSFER);
	memory->writeToMemory(END_MINUTE_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].endTime.minute, NO_MEM_TRANSFER);
	memory->writeToMemory(END_HOUR_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].endTime.hour, NO_MEM_TRANSFER);
	memory->writeToMemory(END_DAY_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].endTime.day, NO_MEM_TRANSFER);
	memory->writeToMemory(END_MONTH_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].endTime.month, NO_MEM_TRANSFER);
	memory->writeToMemory(END_YEAR_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), dryerPrograms[WichProgram].endTime.year, NO_MEM_TRANSFER);
	memory->writeToMemory(TEMP_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), TempToSave, MEM_TRANSFER);

}

void SmartDryer::loadDryerProgram(uint8_t WichProgram)
{
	const uint8_t MAX_DRYER_VAR = 14;
	uint32_t TempSaved = 0;
	memory->loadMemory(SETTED_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].programSetted);
	memory->loadMemory(START_SECOND_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].startTime.second);
	memory->loadMemory(START_MINUTE_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].startTime.minute);
	memory->loadMemory(START_HOUR_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].startTime.hour);
	memory->loadMemory(START_DAY_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].startTime.day);
	memory->loadMemory(START_MONTH_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].startTime.month);
	memory->loadMemory(START_YEAR_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].startTime.year);
	memory->loadMemory(END_SECOND_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].endTime.second);
	memory->loadMemory(END_MINUTE_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].endTime.minute);
	memory->loadMemory(END_HOUR_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].endTime.hour);
	memory->loadMemory(END_DAY_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].endTime.day);
	memory->loadMemory(END_MONTH_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].endTime.month);
	memory->loadMemory(END_YEAR_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&dryerPrograms[WichProgram].endTime.year);
	memory->loadMemory(TEMP_PROGRAM_1 + (MAX_DRYER_VAR * WichProgram), (uint32_t *)&TempSaved);
	dryerPrograms[WichProgram].tempSetted = (float)TempSaved;
}


void SmartDryer::navMenu()
{
	bool ExitNavMenu = false;
	MENU_STRUCTURE *ActualMenu = mainMenu;
	uint8_t MenuSel = MAIN_MENU;
	showHelpMessageTimer->restart();
	String Time = "", Date = "";
	while(!ExitNavMenu)
	{
		uint8_t WichKey = DryerKey::NO_KEY;
		display->clearFrameBuffer();
		showTimeDate(Time, Date);
		display->drawString(ActualMenu->menuTitle, NHDST7565_LCD::CENTER_POS, MENU_TITLE_YPOS, display->displayFonts[NHDST7565_LCD::W_6_H_13_B]);
		ActualMenu->maxMenuLines = display->drawMenuList(ActualMenu->XPos, ActualMenu->YPos, ActualMenu->topItemPos,
				ActualMenu->itemSelected, ActualMenu->menuVoices, ActualMenu->maxMenuItems,
				ActualMenu->withChebox, ActualMenu->menuSelected, ActualMenu->itemsChecked, ActualMenu->menuFont);
//		if(showHelpMessageTimer->isFinished(false, 3))
//		{
//			if(MenuSel == MAIN_MENU)
//			{
//				u8g2_DrawFrame(&display->U8G2_Display, MENU_RIGHT_LIST_XPOS - 1, HELP_MESSAGE_YPOS - 1, display->dispParams.width - MENU_RIGHT_LIST_XPOS - 1
//						, display->dispParams.high - HELP_MESSAGE_YPOS - 1);
//				display->drawBox(MENU_RIGHT_LIST_XPOS, HELP_MESSAGE_YPOS, display->dispParams.width - MENU_RIGHT_LIST_XPOS,
//						display->dispParams.high - HELP_MESSAGE_YPOS, WHITE_COLOR);
//				//				  u8g2_SetDrawColor(&display->U8G2_Display, 0);
//				//				  u8g2_DrawBox(&display->U8G2_Display, MENU_RIGHT_LIST_XPOS, HELP_MESSAGE_YPOS, display->dispParams.width - MENU_RIGHT_LIST_XPOS, display->dispParams.high - HELP_MESSAGE_YPOS);
//				//				  u8g2_SetDrawColor(&display->U8G2_Display, 1);
//				display->drawText(mainMenuHelpMsgs.at(ActualMenu->itemSelected), MENU_RIGHT_LIST_XPOS, HELP_MESSAGE_YPOS, HELP_MESSAGE_MARGIN);
//			}
//			else
//			{
//				showHelpMessageTimer->restart();
//			}
//		}

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
							case TEMP_CTRL:
								MenuSel = TEMP_CTRL;
								ActualMenu = tempMenuCtrl;
								break;
							case START_DRYER_CTRL:
								MenuSel = START_DRYER_CTRL;
								ActualMenu = startDryerMenu;
								break;
							case CHANGE_TIME_MENU:
								screen = CHANGE_TIME;
								ExitNavMenu = true;
								break;
							case CHANGE_DATE_MENU:
								screen = CHANGE_DATE;
								ExitNavMenu = true;
								break;
							case CHANGE_PROGRAMS_LIST:
								MenuSel = CHANGE_PROGRAMS_LIST;
								ActualMenu = changeProgramsMenu;
								break;
							case START_PROGRAMS_LIST:
								MenuSel = START_PROGRAMS_LIST;
								ActualMenu = startProgramsMenu;
								break;
							case SHOW_INFO_PAGE:
								screen = SHOW_INFO;
								ExitNavMenu = true;
								break;
							default:
								break;
						}
						if(ActualMenu->withChebox)
						{
							for(int i = 0; i < ActualMenu->maxMenuItems; i++)
							{
								ActualMenu->itemsChecked[i] = false;
							}
							switch(ActualMenu->paramType)
							{
								case PARAM_BOOL_TYPE:
									if(*(bool *)ActualMenu->paramAssociated)
									{
										ActualMenu->itemsChecked[0] = false;
										ActualMenu->itemsChecked[1] = true;
									}
									else
									{
										ActualMenu->itemsChecked[0] = true;
										ActualMenu->itemsChecked[1] = false;
									}
									break;
								case PARAM_FLOAT_TYPE:
									switch(MenuSel)
									{
										case TEMP_CTRL:
											for(int i = 0; i < ActualMenu->maxMenuItems; i++)
											{
												uint32_t TempSetted = (uint32_t)*(float *)ActualMenu->paramAssociated;
												if(TempSetted == (uint32_t)paramTemperatures[i])
												{
													ActualMenu->itemsChecked[i] = true;
												}
												else
													ActualMenu->itemsChecked[i] = false;
											}
											break;
										default:
											break;
									}
									break;
								default:
									break;
							}
						}
						break; // case MAIN_MENU
					case CHANGE_PROGRAMS_LIST:
						screen = ActualMenu->itemSelected + CHANGE_PROGRAM_1;
						ExitNavMenu = true;
						break;
					case START_PROGRAMS_LIST:
						if(dryerPrograms[ ActualMenu->itemSelected].programSetted)
						{
							screen = ActualMenu->itemSelected + START_PROGRAM_1;
							ExitNavMenu = true;
						}
						break;
					default:
						break;
				}
			break;// case LEFT_KEY
			case DryerKey::OK_KEY:
				if(ActualMenu->withChebox)
				{
					for(int i = 0; i < ActualMenu->maxMenuItems; i++)
						ActualMenu->itemsChecked[i] = false;
					ActualMenu->itemsChecked[ActualMenu->itemSelected] = true;
					switch(ActualMenu->paramType)
					{
						case PARAM_BOOL_TYPE:
							if(ActualMenu->itemSelected == 0)
								*(bool *)ActualMenu->paramAssociated = false;
							else
								*(bool *)ActualMenu->paramAssociated = true;
							break;
						case PARAM_FLOAT_TYPE:
							switch(MenuSel)
							{
								case TEMP_CTRL:
									*(float *)ActualMenu->paramAssociated = (float)paramTemperatures[ActualMenu->itemSelected];
									break;
								default:
									break;
							}
							break;
						default:
							break;
					}
					MenuSel = MAIN_MENU;
					ActualMenu = mainMenu;
				}
				break;
			case DryerKey::LONG_LEFT_KEY:
				switch(MenuSel)
				{
					case THERMO_CTRL:
					case FAN_CTRL:
					case TEMP_CTRL:
					case START_DRYER_CTRL:
					case CHANGE_PROGRAMS_LIST:
					case START_PROGRAMS_LIST:
						MenuSel = MAIN_MENU;
						ActualMenu = mainMenu;
						break;
					default:
						break;
				}
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
					ActualMenu->topItemPos = ActualMenu->itemSelected - (ActualMenu->maxMenuLines - 2);
				else
					ActualMenu->topItemPos = 0;
				if(ActualMenu->itemSelected >= ActualMenu->maxMenuItems - ActualMenu->maxMenuLines)
				{
					ActualMenu->topItemPos = ActualMenu->maxMenuItems - ActualMenu->maxMenuLines;
				}
			}
			else
				ActualMenu->topItemPos = 0;

		}
		else if(WichKey != DryerKey::NO_KEY)
		{
			showHelpMessageTimer->restart();
		}
		peripheralsControl();
	}

}



void SmartDryer::test()
{

}

void SmartDryer::setup()
{
	uint32_t FirstBootVal = 0;
	display->setupLcd();
	clock->setup();
	rtcRunning = clock->isRunning();

	const uint8_t NTemps =  sizeof(Temps)/sizeof(Temps[0]);
	paramTemperatures = new uint8_t[NTemps];
	for(int i = 0; i < NTemps; i++)
	{
		paramTemperatures[i] = 30 + (i * 5);
	}

	physicalReleCtrl();
	ledControl();


	mainMenuHelpMsgs.push_back("Accende o spegne la resistenza");
	mainMenuHelpMsgs.push_back("Accende o spegne la ventola");
	mainMenuHelpMsgs.push_back("Imposta la temperatura di servizio");
	mainMenuHelpMsgs.push_back("Avvia il dryer manualmente");
	mainMenuHelpMsgs.push_back("Modifica l'ora di sistema");
	mainMenuHelpMsgs.push_back("Modifica la data di sistema");
	mainMenuHelpMsgs.push_back("Imposta i programmi");
	mainMenuHelpMsgs.push_back("Seleziona il programma da avviare");
	mainMenuHelpMsgs.push_back("Mostra info sistema");
	if(mainMenuHelpMsgs.size() < MAX_MENU_ITEMS)
	{
		mainMenuHelpMsgs.clear();
		for(int i = 0; i < MAX_MENU_ITEMS; i++)
		{
			mainMenuHelpMsgs.push_back("");
		}
	}


	mainMenu->menuTitle = "Menu principale";
	mainMenu->menuVoices = mainMenuVoices;
	mainMenu->XPos = MENU_LEFT_LIST_XPOS;
	mainMenu->YPos = MENU_LIST_YPOS;
	mainMenu->menuFont = display->displayFonts[NHDST7565_LCD::W_5_H_8];
	mainMenu->topItemPos = 0;
	mainMenu->itemSelected = 0;
	mainMenu->maxMenuLines = 0;
	mainMenu->maxMenuItems = sizeof(mainMenuVoices)/sizeof(mainMenuVoices[0]);
	mainMenu->withChebox = false;
	mainMenu->itemsChecked = NULL;
	mainMenu->paramAssociated = NULL;
	mainMenu->paramType = NO_TYPE;
	mainMenu->menuSelected = true;

	thermoMenuCtrl->menuTitle = "Ctrl. thermo";
	thermoMenuCtrl->menuVoices = OnOff;
	thermoMenuCtrl->XPos = MENU_RIGHT_LIST_XPOS;
	thermoMenuCtrl->YPos = MENU_LIST_YPOS;
	thermoMenuCtrl->menuFont = display->displayFonts[NHDST7565_LCD::W_5_H_8];
	thermoMenuCtrl->topItemPos = 0;
	thermoMenuCtrl->itemSelected = 0;
	thermoMenuCtrl->maxMenuLines = 0;
	thermoMenuCtrl->maxMenuItems = sizeof(OnOff)/sizeof(OnOff[0]);
	thermoMenuCtrl->withChebox = true;
	thermoMenuCtrl->itemsChecked = new bool(thermoMenuCtrl->maxMenuItems);
	thermoMenuCtrl->paramAssociated = (bool *)&statusParam->thermoOn;
	thermoMenuCtrl->paramType = PARAM_BOOL_TYPE;
	thermoMenuCtrl->menuSelected = true;

	fanMenuCtrl->menuTitle = "Ctrl. ventola";
	fanMenuCtrl->menuVoices = OnOff;
	fanMenuCtrl->XPos = MENU_RIGHT_LIST_XPOS;
	fanMenuCtrl->YPos = MENU_LIST_YPOS;
	fanMenuCtrl->menuFont = display->displayFonts[NHDST7565_LCD::W_5_H_8];
	fanMenuCtrl->topItemPos = 0;
	fanMenuCtrl->itemSelected = 0;
	fanMenuCtrl->maxMenuLines = 0;
	fanMenuCtrl->maxMenuItems = sizeof(OnOff)/sizeof(OnOff[0]);
	fanMenuCtrl->withChebox = true;
	fanMenuCtrl->itemsChecked = new bool(fanMenuCtrl->maxMenuItems);
	fanMenuCtrl->paramAssociated = (bool *)&statusParam->fanOn;
	fanMenuCtrl->paramType = PARAM_BOOL_TYPE;
	fanMenuCtrl->menuSelected = true;

	startDryerMenu->menuTitle = "Avvia dryer";
	startDryerMenu->menuVoices = OnOff;
	startDryerMenu->XPos = MENU_RIGHT_LIST_XPOS;
	startDryerMenu->YPos = MENU_LIST_YPOS;
	startDryerMenu->menuFont = display->displayFonts[NHDST7565_LCD::W_5_H_8];
	startDryerMenu->topItemPos = 0;
	startDryerMenu->itemSelected = 0;
	startDryerMenu->maxMenuLines = 0;
	startDryerMenu->maxMenuItems = sizeof(OnOff)/sizeof(OnOff[0]);
	startDryerMenu->withChebox = true;
	startDryerMenu->itemsChecked = new bool(startDryerMenu->maxMenuItems);
	startDryerMenu->paramAssociated = (bool *)&statusParam->dryerOn;
	startDryerMenu->paramType = PARAM_BOOL_TYPE;
	startDryerMenu->menuSelected = true;

	tempMenuCtrl->menuTitle = "Imposta temp.";
	tempMenuCtrl->menuVoices = Temps;
	tempMenuCtrl->XPos = MENU_RIGHT_LIST_XPOS;
	tempMenuCtrl->YPos = MENU_LIST_YPOS;
	tempMenuCtrl->menuFont = display->displayFonts[NHDST7565_LCD::W_5_H_8];
	tempMenuCtrl->topItemPos = 0;
	tempMenuCtrl->itemSelected = 0;
	tempMenuCtrl->maxMenuLines = 0;
	tempMenuCtrl->maxMenuItems = sizeof(Temps)/sizeof(Temps[0]);;
	tempMenuCtrl->withChebox = true;
	tempMenuCtrl->itemsChecked = new bool(tempMenuCtrl->maxMenuItems);
	tempMenuCtrl->paramAssociated = (float *)&statusParam->temperatureSetted;
	tempMenuCtrl->paramType = PARAM_FLOAT_TYPE;
	tempMenuCtrl->menuSelected = true;

	changeProgramsMenu->menuTitle = "Modifica programma";
	changeProgramsMenu->menuVoices = programsMenuVoices;
	changeProgramsMenu->XPos = MENU_LEFT_LIST_XPOS;
	changeProgramsMenu->YPos = MENU_LIST_YPOS;
	changeProgramsMenu->menuFont = display->displayFonts[NHDST7565_LCD::W_5_H_8];
	changeProgramsMenu->topItemPos = 0;
	changeProgramsMenu->itemSelected = 0;
	changeProgramsMenu->maxMenuLines = 0;
	changeProgramsMenu->maxMenuItems = sizeof(programsMenuVoices)/sizeof(programsMenuVoices[0]);
	changeProgramsMenu->withChebox = false;
	changeProgramsMenu->itemsChecked = NULL;
	changeProgramsMenu->paramAssociated = NULL;
	changeProgramsMenu->paramType = NO_TYPE;
	changeProgramsMenu->menuSelected = true;

	startProgramsMenu->menuTitle = "Avvia programma";
	startProgramsMenu->menuVoices = programsMenuVoices;
	startProgramsMenu->XPos = MENU_LEFT_LIST_XPOS;
	startProgramsMenu->YPos = MENU_LIST_YPOS;
	startProgramsMenu->menuFont = display->displayFonts[NHDST7565_LCD::W_5_H_8];
	startProgramsMenu->topItemPos = 0;
	startProgramsMenu->itemSelected = 0;
	startProgramsMenu->maxMenuLines = 0;
	startProgramsMenu->maxMenuItems = sizeof(programsMenuVoices)/sizeof(programsMenuVoices[0]);
	startProgramsMenu->withChebox = false;
	startProgramsMenu->itemsChecked = NULL;
	startProgramsMenu->paramAssociated = NULL;
	startProgramsMenu->paramType = NO_TYPE;
	startProgramsMenu->menuSelected = true;

	if(eepromEnabled)
	{
		if(resetMemory)
		{
			FirstBootVal = 500;
			memory->writeToMemory(FIRST_BOOT, FirstBootVal, MEM_TRANSFER);
			ResetSystem;
		}
		if(!memory->memoryEmpty())
		{
			memory->loadMemory(FIRST_BOOT, &FirstBootVal);
			if(FirstBootVal == 200)
			{
				for(int i = 0; i < MAX_DRYER_PROGRAMS; i++)
				{
					loadDryerProgram(i);
				}
			}
			else
			{
				FirstBootVal = 200;
				for(int i = 0; i < MAX_DRYER_PROGRAMS; i++)
				{
					writeDefaultsDryerProgram(i);
				}
				memory->eraseMemory();
				memory->writeToMemory(FIRST_BOOT, FirstBootVal, NO_MEM_TRANSFER);
				for(int i = 0; i < MAX_DRYER_PROGRAMS; i++)
				{
					saveDryerProgram(i);
					HAL_Delay(500);
				}
				ResetSystem;
			}
		}
		else
		{
			FirstBootVal = 200;
			memory->writeToMemory(FIRST_BOOT, FirstBootVal, NO_MEM_TRANSFER);
			for(int i = 0; i < MAX_DRYER_PROGRAMS; i++)
			{
				writeDefaultsDryerProgram(i);
			}
			for(int i = 0; i < MAX_DRYER_PROGRAMS; i++)
			{
				saveDryerProgram(i);
				HAL_Delay(500);
			}
			ResetSystem;
		}
	}

}


void SmartDryer::changeTime()
{
	bool ExitChangeTime = false;
	uint8_t Hour = 0, Minute = 0;
	bool IsHours = true;
	String Title = "", Number = "";
	String Time = "", Date = "";
	while(!ExitChangeTime)
	{
		display->clearFrameBuffer();
		showTimeDate(Time, Date);
		if(IsHours)
		{
			Title = "Imposta ora";
			Number = std::to_string(Hour);
		}
		else
		{
			Title = "Imposta minuto";
			Number = std::to_string(Minute);
		}
		display->drawString(Title, NHDST7565_LCD::CENTER_POS, 5, display->displayFonts[NHDST7565_LCD::W_6_H_13_B]);
		display->drawString(Number, NHDST7565_LCD::CENTER_POS, 30, display->displayFonts[NHDST7565_LCD::W_9_H_17_B]);
		display->drawSymbol(60, 30, display->displayFonts[NHDST7565_LCD::W_8_H_8_ICON], 0x0070); // triangolo alto
		display->drawSymbol(60, 55, display->displayFonts[NHDST7565_LCD::W_8_H_8_ICON], 0x006D); // triangolo basso
		display->sendFrameBuffer();
		uint8_t WichKey = DryerKey::NO_KEY;
		WichKey = keyboard->checkKey();
		switch(WichKey)
		{
			case DryerKey::UP_KEY:
			case DryerKey::LONG_UP_KEY:
				if(IsHours)
				{
					if(Hour > 0)
						Hour--;
					else
						Hour = 23;
				}
				else
				{
					if(Minute > 0)
						Minute--;
					else
						Minute = 59;
				}
				break;
			case DryerKey::DOWN_KEY:
			case DryerKey::LONG_DOWN_KEY:
				if(IsHours)
				{
					if(Hour < 23)
						Hour++;
					else
						Hour = 0;
				}
				else
				{
					if(Minute < 59)
						Minute++;
					else
						Minute = 0;
				}
				break;
			case DryerKey::OK_KEY:
				if(IsHours)
				{
					IsHours = false;
				}
				else
				{
					DS1307_RTC::TIME_DATE_T NewTime;
					clock->getTimeDate(NewTime);
					NewTime.hour = Hour;
					NewTime.minute = Minute;
					NewTime.second = 0;
					clock->adjustTimeDate(NewTime);
					ExitChangeTime = true;
				}
				break;
			case DryerKey::LONG_OK_KEY:
				if(!IsHours)
				{
					IsHours = true;
				}
				break;
			case DryerKey::LONG_LEFT_KEY:
				ExitChangeTime = true;
				break;
			default:
				break;
		}
		peripheralsControl();
	}
}

void SmartDryer::changeDate()
{
	bool ExitChangeDate = false;
	uint8_t Day = 1, Month = 1, Year = 20;
	uint8_t WichDateVar = 0;
	String Title = "", Number = "";
	String Time = "", Date = "";
	while(!ExitChangeDate)
	{
		display->clearFrameBuffer();
		showTimeDate(Time, Date);
		if(WichDateVar == 0)
		{
			Title = "Imposta mese";
			Number = String(monthNames[Month - 1]);
		}
		else if(WichDateVar == 1)
		{
			Title = "Imposta giorno";
			Number = std::to_string(Day);
		}
		else
		{
			Title = "Imposta anno";
			Number = std::to_string(Year);
		}
		display->drawString(Title, NHDST7565_LCD::CENTER_POS, 5, display->displayFonts[NHDST7565_LCD::W_6_H_13_B]);
		display->drawString(Number, NHDST7565_LCD::CENTER_POS, 30, display->displayFonts[NHDST7565_LCD::W_9_H_17_B]);
		display->drawSymbol(60, 30, display->displayFonts[NHDST7565_LCD::W_8_H_8_ICON], 0x0070); // triangolo alto
		display->drawSymbol(60, 55, display->displayFonts[NHDST7565_LCD::W_8_H_8_ICON], 0x006D); // triangolo basso
		display->sendFrameBuffer();
		uint8_t WichKey = DryerKey::NO_KEY;
		WichKey = keyboard->checkKey();
		switch(WichKey)
		{
			case DryerKey::UP_KEY:
			case DryerKey::LONG_UP_KEY:
				if(WichDateVar == 1)
				{
					if(Day > 1)
						Day--;
					else
						Day = daysInMonth[Month - 1];
				}
				else if(WichDateVar == 0)
				{
					if(Month > 1)
						Month--;
					else
						Month = 12;
				}
				else
				{
					if(Year > 20)
						Year--;
					else
						Year = 99;
				}
				break;
			case DryerKey::DOWN_KEY:
			case DryerKey::LONG_DOWN_KEY:
				if(WichDateVar == 1)
				{
					if(Day < daysInMonth[Month - 1])
						Day++;
					else
						Day = 1;
				}
				else if(WichDateVar == 0)
				{
					if(Month < 12)
						Month++;
					else
						Month = 1;
				}
				else
				{
					if(Year < 99)
						Year++;
					else
						Year = 20;
				}
				break;
			case DryerKey::OK_KEY:
				if(WichDateVar < 2)
				{
					WichDateVar++;
				}
				else
				{
					DS1307_RTC::TIME_DATE_T NewDate;
					clock->getTimeDate(NewDate);
					NewDate.day = Day;
					NewDate.month = Month;
					NewDate.year = Year + 2000;
					clock->adjustTimeDate(NewDate);
					ExitChangeDate = true;
				}
				break;
			case DryerKey::LONG_OK_KEY:
				if(WichDateVar > 0)
				{
					WichDateVar--;
				}
				break;
			case DryerKey::LONG_LEFT_KEY:
				ExitChangeDate = true;
				break;
			default:
				break;
		}
		peripheralsControl();
	}
}

void SmartDryer::showInfo()
{
	bool ExitShowInfo = false;
	String Time = "", Date = "";
	while(!ExitShowInfo)
	{
		String TempReaded = floatString(chamberTemperature.readedValueFL, 1) + "C";
		String HumidityReaded = floatString(chamberHumity.readedValueFL, 1) + "%";
		display->clearFrameBuffer();
		showTimeDate(Time, Date);
		display->drawString("Condizioni camera", NHDST7565_LCD::CENTER_POS, 8, display->displayFonts[NHDST7565_LCD::W_5_H_8]);
		display->drawString(TempReaded + "    " + HumidityReaded, NHDST7565_LCD::CENTER_POS, 20, display->displayFonts[NHDST7565_LCD::W_6_H_13_B]);
		display->drawString("Versione SW", NHDST7565_LCD::CENTER_POS, 40, display->displayFonts[NHDST7565_LCD::W_5_H_8]);
		display->drawString(String(SW_VERSION), NHDST7565_LCD::CENTER_POS, 50, display->displayFonts[NHDST7565_LCD::W_6_H_13_B]);
		display->sendFrameBuffer();
		uint8_t WichKey = DryerKey::NO_KEY;
		WichKey = keyboard->checkKey();
		switch(WichKey)
		{
			case DryerKey::LONG_LEFT_KEY:
				ExitShowInfo = true;
				break;
			default:
				break;
		}
		peripheralsControl();
	}
}

void SmartDryer::changeProgram(uint8_t WichProgram)
{
	bool ExitChangeProgram = false;
	uint8_t WichSetting = 0;
	const uint8_t MAX_SETTINGS = 5;
	uint8_t StartHour = dryerPrograms[WichProgram].startTime.hour, StartMinute = dryerPrograms[WichProgram].startTime.minute;
	uint8_t EndHour = dryerPrograms[WichProgram].endTime.hour, EndMinute = dryerPrograms[WichProgram].endTime.minute;
	uint32_t TemperatureSetted = (uint32_t)dryerPrograms[WichProgram].tempSetted;
	String Time = "", Date = "";
	String Title = "", SubTitle = "", Number = "";
	while(!ExitChangeProgram)
	{
		Title = "Programma " + std::to_string(WichProgram + 1);
		switch(WichSetting)
		{
			case 0:
				SubTitle = "Ora di inizio";
				Number = std::to_string(StartHour);
				break;
			case 1:
				SubTitle = "Minuto di inizio";
				Number = std::to_string(StartMinute);
				break;
			case 2:
				SubTitle = "Ora di fine";
				Number = std::to_string(EndHour);
				break;
			case 3:
				SubTitle = "Minuto di fine";
				Number = std::to_string(EndMinute);
				break;
			case 4:
				SubTitle = "Temperatura";
				Number = std::to_string(TemperatureSetted);
				break;
			default:
				break;
		}
		display->clearFrameBuffer();
		showTimeDate(Time, Date);
		display->drawString(Title, NHDST7565_LCD::CENTER_POS, 7, display->displayFonts[NHDST7565_LCD::W_6_H_13_B]);
		display->drawString(SubTitle, NHDST7565_LCD::CENTER_POS, 22, display->displayFonts[NHDST7565_LCD::W_6_H_10]);
		display->drawString(Number, NHDST7565_LCD::CENTER_POS, 41, display->displayFonts[NHDST7565_LCD::W_6_H_13_B]);
		display->drawSymbol(60, 40, display->displayFonts[NHDST7565_LCD::W_8_H_8_ICON], 0x0070); // triangolo alto
		display->drawSymbol(60, 60, display->displayFonts[NHDST7565_LCD::W_8_H_8_ICON], 0x006D); // triangolo basso
		display->sendFrameBuffer();
		uint8_t WichKey = DryerKey::NO_KEY;
		WichKey = keyboard->checkKey();
		switch(WichKey)
		{
			case DryerKey::UP_KEY:
			case DryerKey::LONG_UP_KEY:
				switch(WichSetting)
				{
					case 0:
						if(StartHour > 0)
							StartHour--;
						else
							StartHour = 23;
						break;
					case 1:
						if(StartMinute > 0)
							StartMinute -= 10;
						else
							StartMinute = 50;
						break;
					case 2:
						if(EndHour > 0)
							EndHour--;
						else
							EndHour = 23;
						break;
					case 3:
						if(EndMinute > 0)
							EndMinute -= 10;
						else
							EndMinute = 50;
						break;
					case 4:
						if(TemperatureSetted > 30)
							TemperatureSetted -= 5;
						else
							TemperatureSetted = 90;
						break;
					default:
						break;
				}
				break;
			case DryerKey::DOWN_KEY:
			case DryerKey::LONG_DOWN_KEY:
				switch(WichSetting)
				{
					case 0:
						if(StartHour < 23)
							StartHour++;
						else
							StartHour = 0;
						break;
					case 1:
						if(StartMinute < 50)
							StartMinute += 10;
						else
							StartMinute = 0;
						break;
					case 2:
						if(EndHour < 23)
							EndHour++;
						else
							EndHour = 0;
						break;
					case 3:
						if(EndMinute < 50)
							EndMinute += 10;
						else
							EndMinute = 0;
						break;
					case 4:
						if(TemperatureSetted < 90)
							TemperatureSetted += 5;
						else
							TemperatureSetted = 30;
						break;
					default:
						break;
				}
				break;
			case DryerKey::OK_KEY:
				if(WichSetting < MAX_SETTINGS - 1)
					WichSetting++;
				else
				{
					clock->getTimeDate(dryerPrograms[WichProgram].startTime);
					clock->getTimeDate(dryerPrograms[WichProgram].endTime);
					dryerPrograms[WichProgram].tempSetted = (float)TemperatureSetted;
					dryerPrograms[WichProgram].startTime.minute = StartMinute;
					dryerPrograms[WichProgram].endTime.minute = EndMinute;
					dryerPrograms[WichProgram].startTime.hour = StartHour;
					dryerPrograms[WichProgram].endTime.hour = EndHour;
					if(StartHour > EndHour)
					{
						if(dryerPrograms[WichProgram].endTime.day + 1 < daysInMonth[dryerPrograms[WichProgram].endTime.month - 1])
							dryerPrograms[WichProgram].endTime.day += 1;
						else
						{
							dryerPrograms[WichProgram].endTime.day = 1;
							dryerPrograms[WichProgram].endTime.month += 1;
						}
					}
					else if(EndHour == StartHour)
					{
						if(StartMinute > EndMinute)
						{
							if(EndMinute + (StartMinute  - EndMinute) < 50)
							{
								dryerPrograms[WichProgram].endTime.minute = EndMinute + (StartMinute  - EndMinute);
							}
							else
							{
								dryerPrograms[WichProgram].endTime.minute = EndMinute;
								dryerPrograms[WichProgram].endTime.hour = EndHour + 1;
								if(StartHour > EndHour)
								{
									if(dryerPrograms[WichProgram].endTime.day + 1 < daysInMonth[dryerPrograms[WichProgram].endTime.month - 1])
										dryerPrograms[WichProgram].endTime.day += 1;
									else
									{
										dryerPrograms[WichProgram].endTime.day = 1;
										dryerPrograms[WichProgram].endTime.month += 1;
									}
								}
							}
						}
					}
					dryerPrograms[WichProgram].programSetted = true;
					if(eepromEnabled)
					{
						saveDryerProgram(WichProgram);
					}
					ExitChangeProgram = true;
				}
				break;
			case DryerKey::LEFT_KEY:
				if(WichSetting > 0)
					WichSetting--;
				break;
			case DryerKey::LONG_OK_KEY:

				break;
			case DryerKey::LONG_LEFT_KEY:
				ExitChangeProgram = true;
				break;
			default:
				break;
		}
		peripheralsControl();
	}
}

void SmartDryer::startProgram(uint8_t WichProgram)
{
	bool ExitStartProgram = false;
	bool ProgramEnd = false;
	String Time = "", Date = "";
	String StartHour = "", StartMinute = "", EndHour = "", EndMinute = "";
	String TempRead = "0C", TempSet = "0";
	DS1307_RTC::TIME_DATE_T ActualTime;
	StartHour = dryerPrograms[WichProgram].startTime.hour > 9 ? std::to_string(dryerPrograms[WichProgram].startTime.hour) : "0" + std::to_string(dryerPrograms[WichProgram].startTime.hour);
	StartMinute = dryerPrograms[WichProgram].startTime.minute > 9 ? std::to_string(dryerPrograms[WichProgram].startTime.minute) : "0" + std::to_string(dryerPrograms[WichProgram].startTime.minute);
	EndHour = dryerPrograms[WichProgram].endTime.hour > 9 ? std::to_string(dryerPrograms[WichProgram].endTime.hour) : "0" + std::to_string(dryerPrograms[WichProgram].endTime.hour);
	EndMinute = dryerPrograms[WichProgram].endTime.minute > 9 ? std::to_string(dryerPrograms[WichProgram].endTime.minute) : "0" + std::to_string(dryerPrograms[WichProgram].endTime.minute);
	TempSet = floatString(dryerPrograms[WichProgram].tempSetted, 1) + "C";
	while(!ExitStartProgram)
	{
		if(programStartedTimer->isFinished(true, 500))
		{
			clock->getTimeDate(ActualTime);
		}
		TempRead = floatString(chamberTemperature.readedValueFL, 1) + "C";
		display->clearFrameBuffer();
		showTimeDate(Time, Date);
		display->drawString("Temp. letta", NHDST7565_LCD::LEFT_POS, 10, display->displayFonts[NHDST7565_LCD::W_3_H_6]);
		display->drawString("Temp. target", NHDST7565_LCD::RIGHT_POS, 10, display->displayFonts[NHDST7565_LCD::W_3_H_6]);
		display->drawString(TempRead, NHDST7565_LCD::LEFT_POS, 20, display->displayFonts[NHDST7565_LCD::W_6_H_13_B]);
		display->drawString(TempSet, NHDST7565_LCD::RIGHT_POS, 20, display->displayFonts[NHDST7565_LCD::W_6_H_13_B]);
		display->drawString("Inizio programma", NHDST7565_LCD::LEFT_POS, 40, display->displayFonts[NHDST7565_LCD::W_3_H_6]);
		display->drawString("Fine programma", NHDST7565_LCD::RIGHT_POS, 40, display->displayFonts[NHDST7565_LCD::W_3_H_6]);
		display->drawString(StartHour + ":" + StartMinute, NHDST7565_LCD::LEFT_POS, 50, display->displayFonts[NHDST7565_LCD::W_6_H_13_B]);
		display->drawString(EndHour + ":" + EndMinute, NHDST7565_LCD::RIGHT_POS, 50, display->displayFonts[NHDST7565_LCD::W_6_H_13_B]);
		display->sendFrameBuffer();
		uint8_t WichKey = DryerKey::NO_KEY;
		WichKey = keyboard->checkKey();
		switch(WichKey)
		{
			case DryerKey::OK_KEY:
				break;
			case DryerKey::LONG_LEFT_KEY:
				statusParam->dryerOn = false;
				statusParam->programStarted = false;
				ledStatus = PROGRAM_END;
				display->drawFullScreenPopUp("Programma terminato", 2000);
				programStartedTimer->stopTimer();
				ProgramEnd = true;
				if(eepromEnabled)
				{
					writeDefaultsDryerProgram(WichProgram);
					saveDryerProgram(WichProgram);
				}
				ExitStartProgram = true;
				break;
			default:
				break;
		}

		if(!statusParam->programStarted && !ProgramEnd)
		{
			if(ActualTime.hour >= dryerPrograms[WichProgram].startTime.hour &&
					ActualTime.minute >= dryerPrograms[WichProgram].startTime.minute &&
					ActualTime.day >= dryerPrograms[WichProgram].startTime.day &&
					ActualTime.month >= dryerPrograms[WichProgram].startTime.month &&
					ActualTime.year >= dryerPrograms[WichProgram].startTime.year &&
					ActualTime.day <= dryerPrograms[WichProgram].endTime.day &&
					ActualTime.hour <= dryerPrograms[WichProgram].endTime.hour &&
					ActualTime.minute <= dryerPrograms[WichProgram].endTime.minute &&
					ActualTime.month <= dryerPrograms[WichProgram].endTime.month &&
					ActualTime.year <= dryerPrograms[WichProgram].endTime.year)
			{
				statusParam->programStarted = true;
				statusParam->dryerOn = true;
				ledStatus = PROGRAM_INIT;
				display->drawFullScreenPopUp("Programma iniziato", 2000);
			}
//			else
//			{
//				statusParam->programStarted = false;
//				statusParam->dryerOn = false;
//				if(eepromEnabled)
//				{
//					writeDefaultsDryerProgram(WichProgram);
//					saveDryerProgram(WichProgram);
//				}
//				ExitStartProgram = true;
//			}
		}
		else
		{
			if(!ProgramEnd)
			{
				if(ActualTime.hour == dryerPrograms[WichProgram].endTime.hour &&
						ActualTime.minute == dryerPrograms[WichProgram].endTime.minute &&
						ActualTime.day == dryerPrograms[WichProgram].endTime.day &&
						ActualTime.month == dryerPrograms[WichProgram].endTime.month &&
						ActualTime.year == dryerPrograms[WichProgram].endTime.year)
				{
					statusParam->programStarted = false;
					statusParam->dryerOn = false;
					ledStatus = PROGRAM_END;
					ProgramEnd = true;
					display->drawFullScreenPopUp("Programma terminato", 2000);
					if(eepromEnabled)
					{
						writeDefaultsDryerProgram(WichProgram);
						saveDryerProgram(WichProgram);
					}
					ExitStartProgram = true;
				}
			}
		}
		peripheralsControl();
	}
}

void SmartDryer::run()
{
	display->drawFullScreenPopUp("Smart Dryer", 1000);
	while(1)
	{
		switch(screen)
		{
			case NAV_MENU:
				navMenu();
				break;
			case CHANGE_TIME:
				changeTime();
				screen = NAV_MENU;
				break;
			case CHANGE_DATE:
				changeDate();
				screen = NAV_MENU;
				break;
			case SHOW_INFO:
				showInfo();
				screen = NAV_MENU;
				break;
			case CHANGE_PROGRAM_1:
			case CHANGE_PROGRAM_2:
			case CHANGE_PROGRAM_3:
				//TODO aggiungere altri possibili programmi in cascata
				changeProgram(screen - CHANGE_PROGRAM_1);
				screen = NAV_MENU;
				break;
			case START_PROGRAM_1:
			case START_PROGRAM_2:
			case START_PROGRAM_3:
				//TODO aggiungere altri possibili programmi in cascata
				startProgram(screen - START_PROGRAM_1);
				screen = NAV_MENU;
				break;
			default:
				break;
		}
	}
}
