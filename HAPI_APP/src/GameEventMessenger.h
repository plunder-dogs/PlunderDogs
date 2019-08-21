#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <assert.h>

enum eGameEvent
{
	eEnteredNewBattlePhase,
	eFactionShipDestroyed,
	eUnableToEndMovementPhase,
	eHideEndPhaseButton,
	eShowEndPhaseButton,
	//New Events
	eAllFactionsFinishedDeployment,
	eEnteredNewFactionTurn
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
	FactionShipDestroyedEvent(eFactionName factionName, int shipID)
		: factionName(factionName),
		shipID(shipID)
	{}

	const eFactionName factionName;
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

	void addGameEventToQueue(eGameEvent gameEvent)
	{
		gameEventQueue.push_back(gameEvent);
	}

	void update()
	{
		if (!gameEventQueue.empty())
		{
			for (eGameEvent gameEvent : gameEventQueue)
			{
				broadcast(GameEvent(), gameEvent);
			}

			gameEventQueue.clear();
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
	std::vector<eGameEvent> gameEventQueue;
};