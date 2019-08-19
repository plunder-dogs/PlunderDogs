#pragma once

#include "Map.h"
#include "Player.h"
#include "Faction.h"
#include "Particle.h"
#include <array>

struct GameEvent;
class Battle : private NonCopyable
{
	enum class eDeploymentState
	{
		NotStarted = 0,
		DeployingPlayer,
		DeployingAI,
		Finished
	};

public:
	Battle(std::array<Faction, static_cast<size_t>(eFactionName::eTotal)>& factions);
	~Battle();

	static std::unique_ptr<Battle> startSinglePlayerGame(std::array<Faction, static_cast<size_t>(eFactionName::eTotal)>& factions, const std::string& levelName);
	static std::unique_ptr<Battle> startOnlineGame(std::array<Faction, static_cast<size_t>(eFactionName::eTotal)>& factions,
		const std::string& levelName, const std::vector<ServerMessageSpawnPosition>& factionSpawnPositions);

	bool isShipBelongToCurrentFaction(ShipOnTile shipOnTile) const;
	const Map& getMap() const;
	eBattlePhase getCurrentBattlePhase() const;
	const Faction& getCurrentFaction() const;
	std::vector<eFactionName> getAllFactionsInPlay() const;
	const Ship& getFactionShip(ShipOnTile shipOnTile) const;
	const Faction& getFaction(eFactionName factionName) const;

	void receiveServerMessage(const ServerMessage& receivedServerMessage);

	void endCurrentBattlePhase();
	void render(sf::RenderWindow& window);
	void renderFactionShipsMovementGraph(sf::RenderWindow& window);
	void handleInput(const sf::RenderWindow& window, const sf::Event& currentEvent);
	void update(float deltaTime);

	//Deploy Phase
	void deployFactionShipAtPosition(sf::Vector2i startingPosition, eDirection startingDirection);
	void setShipDeploymentAtPosition(sf::Vector2i position, eDirection direction = eDirection::eNorth);
	//Movement Phase
	void moveFactionShipToPosition(ShipOnTile shipOnTile);
	void moveFactionShipToPosition(ShipOnTile shipOnTile, eDirection endDirection);
	void moveFactionShipsToPosition(ShipSelector& selectedShips);
	void generateFactionShipMovementArea(ShipOnTile shipOnTile, sf::Vector2i destination, bool displayOnlyLastPosition = false);
	void generateFactionShipsMovementArea(const std::vector<const Tile*>& movementArea, const ShipSelector& shipSelector);
	void clearFactionShipMovementArea(ShipOnTile shipOnTile);
	//Attack Phase
	void fireFactionShipAtPosition(ShipOnTile firingShip, const Tile& firingPosition, const std::vector<const Tile*>& targetArea);

private:

	std::array<Faction, static_cast<size_t>(eFactionName::eTotal)>& m_factions;
	Map m_map;
	eBattlePhase m_currentBattlePhase;
	eDeploymentState m_currentDeploymentState;
	Player m_player;
	std::vector<Particle> m_explosionParticles;
	std::vector<Particle> m_fireParticles;
	Timer m_timeUntilAITurn;
	Timer m_timeBetweenAIUnits;
	Timer m_timeUntilGameOver;
	int m_currentFactionTurn;

	Faction& getCurrentPlayer();
	Faction& getFaction(eFactionName factionName);
	
	void playFireAnimation(eDirection orientation, sf::Vector2i position);
	void playExplosionAnimation(sf::Vector2i position);
	void advanceToNextBattlePhase();
	void switchToBattlePhase(eBattlePhase newBattlePhase);
	void updateMovementPhase(float deltaTime);
	void updateAttackPhase();
	void incrementFactionTurn();
	void updateWindDirection();



	void handleAIMovementPhaseTimer(float deltaTime);
	void handleAIAttackPhaseTimer(float deltaTime);
	void handleTimeUntilGameOver(float deltaTime);

	void onFactionShipDestroyed(GameEvent gameEvent);

	void deployFactionShipAtPosition(const ServerMessage& receivedServerMessage);
	void moveFactionShipToPosition(const ServerMessage& receivedServerMessage);
	void fireFactionShipAtPosition(const ServerMessage& receivedServerMessage);
};