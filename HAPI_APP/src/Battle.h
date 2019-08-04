#pragma once

#include "Map.h"
#include "BattleUI.h"
#include "Faction.h"
#include "Particle.h"
#include <array>

struct GameEvent;
class Battle : private NonCopyable
{
	enum class eDeploymentState
	{
		DeployingPlayer = 0,
		DeployingAI,
		Finished
	};

public:
	Battle(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& players);
	~Battle();

	bool isRunning() const;
	bool isShipBelongToFactionInPlay(ShipOnTile shipOnTile) const;
	const Map& getMap() const;
	BattlePhase getCurrentBattlePhase() const;
	const Faction& getCurrentFaction() const;
	eControllerType getCurrentPlayerType() const;
	std::vector<FactionName> getAllFactionsInPlay() const;
	const Ship& getFactionShip(ShipOnTile shipOnTile) const;
	const std::vector<Ship>& getCurrentFactionShips() const;
	const Faction& getFaction(FactionName factionName) const;

	void receiveServerMessage(const ServerMessage& receivedServerMessage);

	void quitGame();
	void startOnlineGame(const std::string& levelName, const std::vector<ServerMessageSpawnPosition>& factionSpawnPositions);
	void startSinglePlayerGame(const std::string& levelName);
	void render(sf::RenderWindow& window);
	void renderFactionShipsMovementGraphs(sf::RenderWindow& window);
	void handleInput(const sf::RenderWindow& window, const sf::Event& currentEvent);
	void update(float deltaTime);

	//Deploy Phase
	void deployFactionShipAtPosition(sf::Vector2i startingPosition, eDirection startingDirection);
	void setShipDeploymentAtPosition(sf::Vector2i position, eDirection direction = eDirection::eNorth);
	//Movement Phase
	void moveFactionShipToPosition(ShipOnTile shipOnTile);
	void moveFactionShipToPosition(ShipOnTile shipOnTile, eDirection endDirection);
	void clearFactionShipMovementArea(ShipOnTile shipOnTile);
	void generateFactionShipMovementArea(ShipOnTile shipOnTile, sf::Vector2i destination, bool displayOnlyLastPosition = false);
	//Disallow ending position of ships movement area to overlap with 
	//ship belonging to same faction
	void rectifyFactionShipMovementArea(ShipOnTile shipOnTile);
	//Attack Phase
	void fireFactionShipAtPosition(ShipOnTile firingShip, const Tile& firingPosition, const std::vector<const Tile*>& targetArea);

private:
	std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& m_factions;
	int m_currentFactionTurn;
	Map m_map;
	BattlePhase m_currentBattlePhase;
	eDeploymentState m_currentDeploymentState;
	BattleUI m_battleUI;
	std::vector<Particle> m_explosionParticles;
	std::vector<Particle> m_fireParticles;
	Timer m_timeUntilAITurn;
	Timer m_timeBetweenAIUnits;
	Timer m_lightIntensityTimer;
	Timer m_timeUntilGameOver;
	eLightIntensity m_currentLightIntensity;
	bool m_isRunning;
	bool m_onlineGame = false;

	Faction& getCurrentPlayer();
	Faction& getFaction(FactionName factionName);
	
	void playFireAnimation(eDirection orientation, sf::Vector2i position);
	void playExplosionAnimation(sf::Vector2i position);
	void advanceToNextBattlePhase();
	void switchToBattlePhase(BattlePhase newBattlePhase);
	void updateLightIntensity(float deltaTime);
	void updateMovementPhase(float deltaTime);
	void updateAttackPhase();
	void incrementFactionTurn();
	void updateWindDirection();

	void handleAIMovementPhaseTimer(float deltaTime);
	void handleAIAttackPhaseTimer(float deltaTime);
	void handleTimeUntilGameOver(float deltaTime);

	void onEndBattlePhaseEarly(GameEvent gameEvent);
	void onFactionShipDestroyed(GameEvent gameEvent);

	void deployFactionShipAtPosition(const ServerMessage& receivedServerMessage);
	void moveFactionShipToPosition(const ServerMessage& receivedServerMessage);
	void fireFactionShipAtPosition(const ServerMessage& receivedServerMessage);
};