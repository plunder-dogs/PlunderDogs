#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <assert.h>

enum GameEvent
{
	eEnteredNewBattlePhase,
	eOnFactionShipDestroyed,
	eEndBattlePhaseEarly,
	eUnableToSkipPhase,
	eEnteredAITurn,
	eLeftAITurn
};

class Listener
{
public:
	Listener(const std::function<void(GameEvent)>& fp)
		: m_listener(fp)
	{}

	std::function<void(GameEvent)> m_listener;
};

class GameEventMessenger
{
public:
	static GameEventMessenger& getInstance()
	{
		static GameEventMessenger instance;
		return instance;
	}

	void subscribe(const std::function<void(GameEvent)>& fp, GameEvent message)
	{
		auto iter = m_listeners.find(message);
		if (iter != m_listeners.cend())
		{
			iter->second.emplace_back(fp);
		}
		else
		{
			m_listeners.emplace(message, std::vector<Listener>{Listener(fp)});
		}
	}

	void broadcast(GameEvent message)
	{
		auto iter = m_listeners.find(message);
		assert(iter != m_listeners.cend());

		for (const auto& listener : iter->second)
		{
			listener.m_listener(message);
		}
	}

	void unsubscribe(GameEvent message)
	{
		auto iter = m_listeners.find(message);
		assert(iter != m_listeners.cend());

		iter->second.pop_back();

		if (iter->second.empty())
		{
			m_listeners.erase(iter);
		}
	}

private:
	std::unordered_map<GameEvent, std::vector<Listener>> m_listeners;
};