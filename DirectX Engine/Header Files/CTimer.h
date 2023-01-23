//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#ifndef TIMER_H
#define TIMER_H

class CTimer
{
public:
	CTimer();

	float GetTotalTime()const; // in seconds
	float GetDeltaTime()const; // in seconds

	void Reset(); // Call before message loop.
	void Start(); // Call when unpaused.
	void Stop();  // Call when paused.
	void Tick();  // Call every frame.

private:
	double SecondsPerCount;
	double DeltaTime;

	__int64 BaseTime;
	__int64 PausedTime;
	__int64 StopTime;
	__int64 PreviousTime;
	__int64 CurrentTime;

	bool bIsStopped;
};

#endif // GAMETIMER_H