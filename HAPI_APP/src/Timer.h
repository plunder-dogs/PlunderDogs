#pragma once

class Timer
{
public:
	Timer(float expirationTime, bool active = true);

	bool isExpired() const;
	float getElaspedTime() const;

	void update(float deltaTime);
	void reset();
	void setActive(bool active);
	void setNewExpirationTime(float newExpirationTime);

private:
	float m_expirationTime;
	float m_elaspedTime;
	bool m_active;
};