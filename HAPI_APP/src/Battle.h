#pragma once

#include "Map.h"
#include "BattleUI.h"
#include "Faction.h"
#include "Particle.h"
#include <array>

//All messages can be TCP
//UDP messages not needed because turn based
//One computer can run a server and other clients will connect to it
//Simplest way is running it as a simulation
//Update remote clients on most update to date data of the game

//Events that'll be sent over the networking + used by local plaeyr and AI
enum class GameMessageEvent
{
	eMoveToPosition = 0,
	eAttackAtPosition,
	Ready,
	Quit,
	ShipDestroyed,
	FactionEliminated
};

struct GameMessage
{
	FactionName factionName;
	ShipOnTile shipOnTile;
	sf::Vector2f position;
	ePlayerType senderType;
	GameMessageEvent messageEvent;
	//More Data
	//Timestamp - Maybe
	//Other networking stuff
};

struct GameEvent;
class Battle
{
	enum class eDeploymentState
	{
		DeployingPlayer = 0,
		DeployingAI,
		Finished
	};

public:
	Battle(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& players);
	Battle(const Battle&) = delete;
	Battle& operator=(const Battle&) = delete;
	Battle(Battle&&) = delete;
	Battle&& operator=(Battle&&) = delete;
	~Battle();

	bool isRunning() const;
	bool isShipBelongToFactionInPlay(ShipOnTile shipOnTile) const;
	const Map& getMap() const;
	BattlePhase getCurrentBattlePhase() const;
	const std::unique_ptr<Faction>& getCurrentFaction() const;
	ePlayerType getCurrentPlayerType() const;
	std::vector<FactionName> getAllFactionsInPlay() const;
	const Ship& getFactionShip(ShipOnTile shipOnTile) const;
	const std::vector<Ship>& getCurrentFactionShips() const;
	const std::unique_ptr<Faction>& getFaction(FactionName factionName) const;

	void start(const std::string& newMapName);
	void render(sf::RenderWindow& window);
	void renderFactionShipsMovementGraphs(sf::RenderWindow& window);
	void handleInput(const sf::RenderWindow& window, const sf::Event& currentEvent);
	void update(float deltaTime);

	
	void addMessage(GameMessage gameMessage)
	{
		//Add to buffer
		//Callers of this function will be Player, AI & Remote Players
	}

	void handleMessageBuffer()
	{
		//Iterate over each message
		//Act on it accordingly
		GameMessage gameMessage;
		switch (gameMessage.senderType)
		{
		case ePlayerType::eHuman :
			//Inform remote players
			break;

		case ePlayerType::eAI :
			//Issue in AI will have to act same across all clients
			break;

		case ePlayerType::eRemotePlayer :
			//Receive inform from remote players
			break;
		}
	}

	//Deploy Phase
	void deployFactionShipAtPosition(sf::Vector2i startingPosition, eDirection startingDirection);
	void setShipDeploymentAtPosition(sf::Vector2i position, eDirection direction = eDirection::eNorth);
	//Movement Phase
	void moveFactionShipToPosition(ShipOnTile shipOnTile);
	void moveFactionShipToPosition(ShipOnTile shipOnTile, eDirection endDirection);
	void clearFactionShipMovementArea(ShipOnTile shipOnTile);
	void generateFactionShipMovementArea(ShipOnTile shipOnTile, sf::Vector2i destination, bool displayOnlyLastPosition = false);
	//Attack Phase
	void fireFactionShipAtPosition(ShipOnTile firingShip, const Tile& firingPosition, const std::vector<const Tile*>& targetArea);

private:
	std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& m_factions;
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

	std::unique_ptr<Faction>& getCurrentPlayer();
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
};