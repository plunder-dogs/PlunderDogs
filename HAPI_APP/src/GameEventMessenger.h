#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <assert.h>
#include <utility>
#include <algorithm>

enum GameEvent
{
	eResetBattle = 0,
	eNewTurn,
	eYellowShipDestroyed,
	eBlueShipDestroyed,
	eGreenShipDestroyed,
	eRedShipDestroyed,
	eRedWin,
	eYellowWin,
	eGreenWin,
	eBlueWin,
	eEnteringMovementPhase,
	eEnteringAttackPhase,
	eEndMovementPhaseEarly,
	eEndAttackPhaseEarly,
	eUnableToSkipPhase,
	eEnteredAITurn,
	eLeftAITurn
};

class Listener
{
public:
	Listener(const std::function<void(GameEvent)>& fp, std::string&& listenerName)
		: m_listener(fp),
		m_name(std::move(listenerName))
	{}

	std::function<void(GameEvent)> m_listener;
	std::string m_name;
};

class GameEventMessenger
{
public:
	static GameEventMessenger& getInstance()
	{
		static GameEventMessenger instance;
		return instance;
	}

	static void subscribe(const std::function<void(GameEvent)>& fp, std::string&& listenerName, GameEvent message)
	{
		auto iter = m_listeners.find(message);
		if (iter != m_listeners.cend())
		{
			iter->second.emplace_back(fp, std::move(listenerName));
		}
		else
		{
			m_listeners.emplace(message, std::vector<Listener>{Listener(fp, std::move(listenerName))});
		}
	}

	static void broadcast(GameEvent message)
	{
		auto iter = m_listeners.find(message);
		assert(iter != m_listeners.cend());

		for (const auto& listener : iter->second)
		{
			listener.m_listener(message);
		}
	}

	static void unsubscribe(const std::string& listenerName, GameEvent message)
	{
		auto iter = m_listeners.find(message);
		assert(iter != m_listeners.cend());

		auto listener = std::find_if(iter->second.begin(), iter->second.end(),
			[&listenerName](const auto& listener) { return listener.m_name == listenerName; });
		assert(listener != iter->second.cend());
		iter->second.erase(listener);

		if (iter->second.empty())
		{
			m_listeners.erase(iter);
		}
	}

private:
	static std::unordered_map<GameEvent, std::vector<Listener>> m_listeners;
};