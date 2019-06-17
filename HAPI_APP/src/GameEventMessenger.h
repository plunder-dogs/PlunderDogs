#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <assert.h>

enum eGameEvent
{
	eEnteredNewBattlePhase,
	eFactionShipDestroyed,
	eEndBattlePhaseEarly,
	eUnableToSkipPhase,
	eEnteredAITurn,
	eLeftAITurn
};

struct GameEvent
{
	GameEvent(const void* data = nullptr)
		: data(data)
	{}

	const void* data;
};

struct FactionShipDestroyedEvent
{
	FactionShipDestroyedEvent(FactionName factionName, int shipID)
		: factionName(factionName),
		shipID(shipID)
	{}

	const FactionName factionName;
	const int shipID;
};

class GameEventMessenger
{
	struct Listener
	{
		Listener(const std::function<void(GameEvent)>& fp)
			: m_listener(fp)
		{}

		std::function<void(GameEvent)> m_listener;
	};

public:
	static GameEventMessenger& getInstance()
	{
		static GameEventMessenger instance;
		return instance;
	}

	void subscribe(const std::function<void(GameEvent)>& fp, eGameEvent message)
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

	void broadcast(GameEvent message, eGameEvent gameEvent)
	{
		auto iter = m_listeners.find(gameEvent);
		assert(iter != m_listeners.cend());

		for (const auto& listener : iter->second)
		{
			listener.m_listener(message);
		}
	}

	void unsubscribe(eGameEvent message)
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
	std::unordered_map<eGameEvent, std::vector<Listener>> m_listeners;
};