#pragma once

#ifndef _GAMETIMER_H_
#define _GAMETIMER_H_

class GameTimer
{
public:
	GameTimer();

	float TotalTime() const; // in seconds
	float DeltaTime() const;

	void Reset(); //call before message loop
	void Start(); // call when unpaused
	void Stop(); //call when paused
	void Tick(); //call every frame

private:
	double m_secondsPerCount;
	double m_deltaTime;

	__int64 m_baseTime;
	__int64 m_pausedTime;
	__int64 m_stopTime;
	__int64 m_prevTime;
	__int64 m_currTime;

	bool m_stopped;
};

#endif