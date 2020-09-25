/*
 * ChronoTimer.cpp
 *
 *  Created on: 25 set 2020
 *      Author: deo
 */

#include <ChronoTimer.h>

ChronoTimer::ChronoTimer(uint8_t Resolution)
{
	if(Resolution <= HOURS)
		resolution = Resolution;
}

void ChronoTimer::setTimer(uint32_t SetDelay)
{
	if(oldSettedDelay != SetDelay)
	{
		switch(resolution)
		{
		case MILLIS:
			timeDelay = SetDelay;
			break;
		case SECONDS:
			timeDelay = SetDelay * 1000;
			break;
		case MINUTE:
			timeDelay = SetDelay * 1000 * 60;
			break;
		case HOURS:
			timeDelay = SetDelay * 1000 * 3600;
			break;
		default:
			break;
		}
		isSetted = true;
		oldSettedDelay = SetDelay;
	}

}

void ChronoTimer::startTimer()
{
	if(isStopped && isSetted)
	{
		isStarted = true;
		isStopped = false;
		getTick = HAL_GetTick();
	}
}

void ChronoTimer::stopTimer()
{
	if(isStarted)
	{
		isStarted = false;
		isStopped = true;
		getTick = 0;
	}
}

void ChronoTimer::restart()
{
	stopTimer();
	startTimer();
}

bool ChronoTimer::isFinished(bool Restart, uint32_t DelayReset)
{
	bool Finish = false;
	if(DelayReset > 0)
	{
		setTimer(DelayReset);
	}
	startTimer();
	if(HAL_GetTick() - getTick >= timeDelay)
	{
		Finish = true;
	}
	if(Restart)
	{
		restart();
	}
	return Finish;
}

uint32_t ChronoTimer::elapsedTime()
{
	uint32_t Elapsed = 0;
	if(isStarted)
	{
		Elapsed = HAL_GetTick() - getTick;
		switch(resolution)
		{
		case MILLIS:
			break;
		case SECONDS:
			timeDelay = Elapsed / 1000;
			break;
		case MINUTE:
			Elapsed = Elapsed / 60000;
			break;
		case HOURS:
			Elapsed = Elapsed / 3600000;
			break;
		default:
			break;
		}
	}
	return Elapsed;
}
