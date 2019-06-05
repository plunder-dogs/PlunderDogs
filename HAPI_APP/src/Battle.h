#pragma once

#include "Global.h"
#include "Map.h"
#include "BattleUI.h"
#include "Faction.h"
#include <array>

enum class ActionName
{
	DeployShip = 0,
	MoveShip,
	Attack
};

struct Action
{
	ShipOnTile shipOnTile;
	ActionName actionName;
	sf::Vector2i position;
};

class Battle
{
	enum class eDeploymentState
	{
		DeployHuman = 0,
		DeployAI
	};

	struct Particle
	{
		sf::Vector2i m_position;
		Timer m_lifeSpan;
		sf::Sprite m_sprite;
		int m_frameNum = 0;
		bool m_isEmitting;
		const float m_scale;

		Particle(float lifespan, std::unique_ptr<sf::Texture>& texture, float scale);
		void setPosition(sf::Vector2i position);
		void update(float deltaTime, const Map& map);
		void render(sf::RenderWindow& window);
		void orient(eDirection direction);
	};

	class WinningFactionHandler
	{
	public:
		WinningFactionHandler();
		~WinningFactionHandler();
	
		bool isGameOver() const;

		void update(BattleUI& battleUI, float deltaTime);

		void onYellowShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& factions);
		void onBlueShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& players);
		void onGreenShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& players);
		void onRedShipDestroyed(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& players);

	private:
		int m_yellowShipsDestroyed;
		int m_blueShipsDestroyed;
		int m_greenShipsDestroyed;
		int m_redShipsDestroyed;
		void onReset();

		void checkGameStatus(const std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& players);
		Timer m_winTimer;
		FactionName m_winningFaction;
		bool m_gameOver;
	};

public:
	Battle(std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& players);
	Battle(const Battle&) = delete;
	Battle& operator=(const Battle&) = delete;
	Battle(Battle&&) = delete;
	Battle&& operator=(Battle&&) = delete;
	~Battle();

	const Map& getMap() const;
	BattlePhase getCurrentPhase() const;
	FactionName getCurrentFaction() const;
	ePlayerType getCurrentPlayerType() const;
	std::vector<FactionName> getAllFactionsInPlay() const;
	const Ship& getFactionShip(ShipOnTile shipOnTile) const;
	const Faction& getFaction(FactionName factionName) const;

	void start(const std::string& newMapName);
	void render(sf::RenderWindow& window);
	void handleInput(const sf::Event& currentEvent);
	void update(float deltaTime);

	//Deploy Phase
	void deployFactionShipAtPosition(sf::Vector2i startingPosition, eDirection startingDirection);
	bool setShipDeploymentAtPosition(sf::Vector2i position);
	//Movement Phase
	void moveFactionShipToPosition(ShipOnTile shipOnTile, sf::Vector2i destination);
	void moveFactionShipToPosition(ShipOnTile shipOnTile, sf::Vector2i destination, eDirection endDirection);
	void disableFactionShipMovementPath(ShipOnTile shipOnTile);
	void generateFactionShipMovementPath(ShipOnTile shipOnTile, sf::Vector2i destination);
	//Attack Phase
	bool fireFactionShipAtPosition(ShipOnTile firingShip, ShipOnTile enemyShip, const std::vector<const Tile*>& targetArea);

private:
	std::array<std::unique_ptr<Faction>, static_cast<size_t>(FactionName::eTotal)>& m_factions;
	int m_currentFactionTurn;
	Map m_map;
	BattlePhase m_currentBattlePhase;
	eDeploymentState m_currentDeploymentState;
	BattleUI m_battleUI;
	WinningFactionHandler m_winningFactionHandler;
	std::vector<Particle> m_explosionParticles;
	std::vector<Particle> m_fireParticles;
	Timer m_timeUntilAITurn;
	Timer m_timeBetweenAIUnits;
	Timer m_lightIntensityTimer;
	eLightIntensity m_currentLightIntensity;

	std::unique_ptr<Faction>& getCurrentPlayer();
	Faction& getFaction(FactionName factionName);
	
	void playFireAnimation(eDirection orientation, sf::Vector2i position);
	void playExplosionAnimation(sf::Vector2i position);
	void nextTurn();
	void switchToBattlePhase(BattlePhase newBattlePhase);
	void notifyPlayersOnNewTurn();
	void updateLightIntensity(float deltaTime);
	void updateMovementPhase(float deltaTime);
	void updateAttackPhase();
	void incrementFactionTurn();
	void updateWindDirection();

	void handleAIMovementPhaseTimer(float deltaTime);
	void handleAIAttackPhaseTimer(float deltaTime);

	void onResetBattle();
	void onYellowShipDestroyed();
	void onBlueShipDestroyed();
	void onGreenShipDestroyed();
	void onRedShipDestroyed();
	void onEndMovementPhaseEarly();
	void onEndAttackPhaseEarly();
};