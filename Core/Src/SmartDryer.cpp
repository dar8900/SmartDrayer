/*
 * SmartDryer.cpp
 *
 *  Created on: 27 set 2020
 *      Author: dar89
 */

#include <SmartDryer.h>

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

const char *programsMenuVoices[] =
{
		"Programma 1",
};



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
	takeTimeTimer = new ChronoTimer(ChronoTimer::MILLIS);
	showHelpMessageTimer = new ChronoTimer(ChronoTimer::SECONDS);

	testTimer = new ChronoTimer(ChronoTimer::MILLIS);

	dbgDryer = new SerialDebug();

	statusParam = new DRYER_PARAMS();

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
			statusParam->fanOn = true;
			statusParam->thermoOn = true;
			if(ledStatus != PROGRAM_INIT && ledStatus != PROGRAM_END)
				ledStatus = THERMO_ON_FAN_ON;
		}
		else if(readedTemperature >= TempTarget)
		{
			statusParam->fanOn = true;
			statusParam->thermoOn = false;
			ledStatus = TEMP_REACHED;
			if(ledStatus != PROGRAM_INIT && ledStatus != PROGRAM_END && ledStatus != TEMP_REACHED)
				ledStatus = THERMO_OFF_FAN_ON;
		}
		else if(readedTemperature < TempTarget - TOLLERANCE_DEGREES)
		{
			statusParam->fanOn = false;
			statusParam->thermoOn = true;
			if(ledStatus != PROGRAM_INIT && ledStatus != PROGRAM_END && ledStatus != TEMP_REACHED)
				ledStatus = THERMO_ON_FAN_OFF;
		}
	}
	else
	{
		statusParam->fanOn = false;
		statusParam->thermoOn = false;
//		statusParam->dryerOn = false;
		if(ledStatus != PROGRAM_INIT && ledStatus != PROGRAM_END && ledStatus != TEMP_REACHED)
			ledStatus = THERMO_OFF_FAN_OFF;
	}
}

void SmartDryer::physicalReleCtrl()
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

void SmartDryer::peripheralsControl()
{
	if(statusParam->dryerOn)
	{
		thermoRegulation(statusParam->temperatureSetted);
		physicalReleCtrl();
	}
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
						screen = ActualMenu->itemSelected + START_PROGRAM_1;
						ExitNavMenu = true;
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

void SmartDryer::setup()
{

	clock = new DS1307_RTC();
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

	mainMenu = new MENU_STRUCTURE();
	thermoMenuCtrl = new MENU_STRUCTURE();
	fanMenuCtrl = new MENU_STRUCTURE();
	tempMenuCtrl = new MENU_STRUCTURE();
	startDryerMenu = new MENU_STRUCTURE();
	changeProgramsMenu = new MENU_STRUCTURE();
	startProgramsMenu = new MENU_STRUCTURE();

	statusParam = new DRYER_PARAMS();


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

	// TODO Caricare dalla eeprom se abilitata, i programmi nelle rispettive variabili in ram
	if(eepromEnabled)
	{

	}

}


void SmartDryer::run()
{
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
				//TODO aggiungere schermata con info su: temperatura camera, ora, data, stato dryer, numero versione
				showInfo();
				screen = NAV_MENU;
				break;
			case CHANGE_PROGRAM_1:
				//TODO aggiungere altri possibili programmi in cascata
				changeProgram(screen - CHANGE_PROGRAM_1);
				screen = NAV_MENU;
				break;
			case START_PROGRAM_1:
				//TODO aggiungere altri possibili programmi in cascata
				startProgram(screen - START_PROGRAM_1);
				screen = NAV_MENU;
				break;
			default:
				break;
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
		// TODO scrivere che si preme ok per andare avanti o long ok per tornare indietro
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
		// TODO scrivere che si preme ok per andare avanti o long ok per tornare indietro
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
}

void SmartDryer::changeProgram(uint8_t WichProgram)
{
	bool ExitChangeProgram = false;
	uint8_t WichSetting = 0;
	const uint8_t MAX_SETTINGS = 5;
	uint8_t StartHour = 0, StartMinute = 0;
	uint8_t EndHour = 0, EndMinute = 0;
	uint32_t TemperatureSetted = 0;
	String Time = "", Date = "";
	while(!ExitChangeProgram)
	{
		display->clearFrameBuffer();
		showTimeDate(Time, Date);

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
							StartMinute--;
						else
							StartMinute = 59;
						break;
					case 2:
						if(EndHour > 0)
							EndHour--;
						else
							EndHour = 23;
						break;
					case 3:
						if(EndMinute > 0)
							EndMinute--;
						else
							EndMinute = 59;
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
						if(StartMinute < 59)
							StartMinute++;
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
						if(EndMinute < 59)
							EndMinute++;
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
					// TODO Salvare in ram il programma appena modificato, se la eeprom è abilitata salvarlo anche li
					clock->getTimeDate(dryerPrograms[WichProgram].startTime);
					clock->getTimeDate(dryerPrograms[WichProgram].endTime);
					dryerPrograms[WichProgram].tempSetted = (float)TemperatureSetted;
					dryerPrograms[WichProgram].startTime.minute = StartMinute;
					dryerPrograms[WichProgram].endTime.minute = EndMinute;
					dryerPrograms[WichProgram].startTime.hour = StartHour;
					dryerPrograms[WichProgram].endTime.hour = EndHour;
					if(EndHour < StartHour)
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
							dryerPrograms[WichProgram].endTime.minute = StartMinute + 1;
						}
					}

					if(eepromEnabled)
					{

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
	}
}

void SmartDryer::startProgram(uint8_t WichProgram)
{
	bool ExitStartProgram = false;
	while(!ExitStartProgram)
	{

		uint8_t WichKey = DryerKey::NO_KEY;
		WichKey = keyboard->checkKey();
		switch(WichKey)
		{
			case DryerKey::UP_KEY:
			case DryerKey::LONG_UP_KEY:
				break;
			case DryerKey::DOWN_KEY:
			case DryerKey::LONG_DOWN_KEY:

				break;
			case DryerKey::OK_KEY:

				break;
			case DryerKey::LONG_OK_KEY:

				break;
			case DryerKey::LONG_LEFT_KEY:
				ExitStartProgram = true;
				break;
			default:
				break;
		}
	}
}
