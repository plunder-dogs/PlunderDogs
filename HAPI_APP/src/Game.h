#pragma once

#include "Textures.h"
#include "Battle.h"
#include "AI.h"
#include <array>
#include "SFML/Graphics.hpp"
#include "Utilities/XMLParser.h"
#include <iostream>
#include "NetworkHandler.h"
#include <assert.h>

class Game : private NonCopyable
{
public:
	Game(bool onlineGame);

	void run();

private:
	std::array<Faction, static_cast<size_t>(FactionName::eTotal)> m_factions;
	sf::RenderWindow m_window;
	bool m_onlineGame;
	bool m_gameLobbyActive;
	bool m_ready;
	Battle m_battle;
	sf::Event m_currentSFMLEvent;
	sf::Clock m_gameClock;
	float m_deltaTime;

	void handleServerMessages();
	void handleInput();
	void handleGameLoop();

	void assignFaction(FactionName factionName, eControllerType controllerType, const std::vector<eShipType>& shipsToAdd);
	FactionName getLocalFactionName() const;
};

//
////
//Ray2D start = { m_currentPosition, m_currentDirection };
//Ray2D end = { destination.x, destination.y };
//std::queue<Ray2D> pathToTile = PathFinding::getInstance().findPath(map, start, end, m_movementPoints);
//if (pathToTile.empty())
//{
//	return;
//}
//
//m_movementArea.clearTileArea();
//while (!pathToTile.empty())
//{
//	m_movementArea.m_tileArea.push_back(pathToTile.front());
//	pathToTile.pop();
//}
//
//m_displayOnlyLastPosition = displayOnlyLastPosition;
//if (m_displayOnlyLastPosition)
//{
//	sf::Vector2i lastPosition = m_movementArea.m_tileArea.back().pair();
//	m_movementArea.m_tileAreaGraph[0].setPosition(lastPosition);
//	m_movementArea.m_tileAreaGraph[0].activate();
//}
//else
//{
//	m_movementArea.activateGraph();
//}
//}