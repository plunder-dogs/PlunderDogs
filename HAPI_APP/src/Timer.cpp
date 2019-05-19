#include "Timer.h"
#include <algorithm>	//abs()

Timer::Timer(float expirationTime, bool active)
	: m_expirationTime(expirationTime),
	m_elaspedTime(0.0f),
	m_active(active)
{
}

bool Timer::isExpired() const
{
	return m_elaspedTime >= m_expirationTime;
}

float Timer::getElaspedTime() const
{
	return m_elaspedTime;
}

void Timer::update(float deltaTime)
{
	if (m_active)
	{
		m_elaspedTime += std::abs(deltaTime);
	}
}

void Timer::reset()
{
	m_elaspedTime = 0.0f;
}

void Timer::setActive(bool active)
{
	m_active = active;
}

void Timer::setNewExpirationTime(float newExpirationTime)
{
	m_expirationTime = newExpirationTime;
}
