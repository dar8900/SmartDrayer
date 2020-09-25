/*
 * ChronoTimer.h
 *
 *  Created on: 25 set 2020
 *      Author: deo
 */

#ifndef INC_CHRONOTIMER_H_
#define INC_CHRONOTIMER_H_
#include "main.h"

class ChronoTimer
{
private:
	uint8_t resolution = MILLIS;
	uint32_t timeDelay = 0;
	uint32_t oldSettedDelay = 0;
	uint32_t getTick = 0;
	bool isSetted = false;
	bool isStarted = false;
	bool isStopped = true;
public:
	enum
	{
		MILLIS = 0,
		SECONDS,
		MINUTE,
		HOURS
	};
	ChronoTimer(uint8_t Resolution = MILLIS);
	void setTimer(uint32_t SetDelay = 1);
	void startTimer();
	void stopTimer();
	void restart();
	bool isFinished(bool Restart = true, uint32_t DelayReset = 0);
	uint32_t elapsedTime();

};

#endif /* INC_CHRONOTIMER_H_ */
