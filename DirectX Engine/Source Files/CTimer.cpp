//***************************************************************************************
// GameTimer.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "CTimer.h"
#include <windows.h>
#include <string>

CTimer::CTimer()
	: SecondsPerCount(0.0), DeltaTime(-1.0), BaseTime(0),
	PausedTime(0), PreviousTime(0), CurrentTime(0), bIsStopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	SecondsPerCount = 1.0 / (double)countsPerSec;
}

// Returns the total time elapsed since Reset() was called, NOT counting any
// time when the clock is stopped.
float CTimer::GetTotalTime()const
{
	// If we are stopped, do not count the time that has passed since we stopped.
	// Moreover, if we previously already had a pause, the distance 
	// mStopTime - mBaseTime includes paused time, which we do not want to count.
	// To correct this, we can subtract the paused time from mStopTime:  
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    CurrentTime

	if (bIsStopped)
	{
		return (float)(((StopTime - PausedTime) - BaseTime) * SecondsPerCount);
	}

	// The distance CurrentTime - mBaseTime includes paused time,
	// which we do not want to count.  To correct this, we can subtract 
	// the paused time from CurrentTime:  
	//
	//  (CurrentTime - mPausedTime) - mBaseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     CurrentTime

	else
	{
		return (float)(((CurrentTime - PausedTime) - BaseTime) * SecondsPerCount);
	}
}

float CTimer::GetDeltaTime()const
{
	return (float)DeltaTime;
}



void CTimer::Reset()
{
	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	BaseTime = currentTime;
	PreviousTime = currentTime;
	StopTime = 0;
	bIsStopped = false;
}

void CTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);


	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	if (bIsStopped)
	{
		PausedTime += (startTime - StopTime);

		PreviousTime = startTime;
		StopTime = 0;
		bIsStopped = false;
	}
}

void CTimer::Stop()
{
	if (!bIsStopped)
	{
		__int64 currentTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

		StopTime = currentTime;
		bIsStopped = true;
	}
}

void CTimer::Tick()
{
	if (bIsStopped)
	{
		DeltaTime = 0.0;
		return;
	}

	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	CurrentTime = currentTime;

	// Time difference between this frame and the previous.
	DeltaTime = (CurrentTime - PreviousTime) * SecondsPerCount;

	// Prepare for next frame.
	PreviousTime = CurrentTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if (DeltaTime < 0.0)
	{
		DeltaTime = 0.0;
	}
}


