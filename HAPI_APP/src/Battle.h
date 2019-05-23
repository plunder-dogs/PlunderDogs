#pragma once

#include "Global.h"
#include "Map.h"
#include "BattleUI.h"
#include "entity.h"
//
//class DeployPlayer
//{
//public:
//	DeployPlayer(const Map& map, const Player& playerToDeploy);
//
//	bool isDeployedAllShips() const;
//
//	void setSelectedShipToNewPosition(std::pair<int, int> position, const Battle& battle);
//	void deploySelectedShip(Battle& battle, std::pair<int, int> startingPosition, eDirection startingDirection);
//	void render(const Map& map);
//
//private:
//	const Player& m_playerToDeploy;
//	Ship* m_shipToDeploy;
//	std::vector<const Tile*> m_spawnArea;
//	std::vector<std::unique_ptr<Sprite>> m_spawnSprites;
//};

class Battle
{
	struct LightIntensity
	{
		LightIntensity();

		void update(float deltaTime);

		Timer m_timer;
		eLightIntensity m_lightIntensity;
	};

	struct Particle
	{
		std::pair<float, float> m_position;
		Timer m_lifeSpan;
		std::unique_ptr<HAPISPACE::Sprite> m_particle;
		int m_frameNum = 0;
		bool m_isEmitting;
		const float m_scale;

		Particle(float lifespan, std::shared_ptr<HAPISPACE::SpriteSheet> texture, float scale);
		void setPosition(std::pair<int, int> position);
		void run(float deltaTime, const Map& map);
		void render() const;
		void orient(eDirection direction);
	};

	class BattleManager
	{
	public:
		BattleManager();
		~BattleManager();
	
		bool isGameOver() const;

		void update(float deltaTime);

		void onYellowShipDestroyed(std::vector<std::unique_ptr<Player>>& players);
		void onBlueShipDestroyed(std::vector<std::unique_ptr<Player>>& players);
		void onGreenShipDestroyed(std::vector<std::unique_ptr<Player>>& players);
		void onRedShipDestroyed(std::vector<std::unique_ptr<Player>>& players);

	private:
		int m_yellowShipsDestroyed;
		int m_blueShipsDestroyed;
		int m_greenShipsDestroyed;
		int m_redShipsDestroyed;
		void onReset();

		void checkGameStatus(const std::vector<std::unique_ptr<Player>>& players);
		Timer m_winTimer;
		FactionName m_winningFaction;
		bool m_gameOver;
	};

public:
	Battle();
	Battle(const Battle&) = delete;
	Battle& operator=(const Battle&) = delete;
	Battle(Battle&&) = delete;
	Battle&& operator=(Battle&&) = delete;
	~Battle();

	const Map& getMap() const;
	BattlePhase getCurrentPhase() const;
	FactionName getCurrentFaction() const;
	std::vector<FactionName> getAllFactions() const;
	const Player& getPlayer(FactionName name) const;
	bool isAIPlaying() const;

	void start(const std::string& newMapName, std::vector<std::unique_ptr<Player>>& newPlayers);
	void render() const;
	void update(float deltaTime);
	void insertEntity(std::pair<int, int> startingPosition, eDirection startingDirection, const ShipGlobalProperties& entityProperties, FactionName factionName);
	void nextTurn();

	//Deploy Phase
	void deployShipAtPosition(std::pair<int, int> startingPosition, eDirection startingDirection);

	bool setShipDeploymentAtPosition(std::pair<int, int> position);
	//Movement Phase
	void moveEntityToPosition(Ship& entity, const Tile& destination);
	void moveEntityToPosition(Ship& entity, const Tile& destination, eDirection endDirection);
	//Attack Phase
	bool fireEntityWeaponAtPosition(const Tile& tileOnPlayer, const Tile& tileOnAttackPosition, const std::vector<const Tile*>& targetArea);
	void playFireAnimation(Ship& entity, std::pair<int, int> position);
	void playExplosionAnimation(Ship& entity);
	
	//const BattlePlayer& getPlayer(FactionName faction) const;
	//std::vector<std::shared_ptr<BattleEntity>>& getFactionShips(FactionName faction);
	//const std::vector<std::shared_ptr<BattleEntity>>& getFactionShips(FactionName faction) const;

private:
	std::vector<std::unique_ptr<Player>> m_players;
	int m_currentPlayerTurn;
	Map m_map;
	BattlePhase m_currentPhase;
	BattleUI m_battleUI;
	BattleManager m_battleManager;
	std::vector<Particle> m_explosionParticles;
	std::vector<Particle> m_fireParticles;
	Timer m_timeUntilAITurn;
	Timer m_timeBetweenAIUnits;
	
	bool m_AITurn;
	LightIntensity m_lightIntensity;
	
	void updateDeploymentPhase();
	void updateMovementPhase(float deltaTime);
	void updateAttackPhase();

	bool allEntitiesAttacked(std::vector<std::unique_ptr<Ship>>& playerEntities) const;
	Player& getPlayer(FactionName factionName);

	void incrementPlayerTurn();
	void updateWindDirection();

	void handleAIMovementPhaseTimer(float deltaTime);
	void handleAIAttackPhaseTimer(float deltaTime);
	//void resetAITimers();

	void onResetBattle();
	void onYellowShipDestroyed();
	void onBlueShipDestroyed();
	void onGreenShipDestroyed();
	void onRedShipDestroyed();
	void onEndMovementPhaseEarly();
	void onEndAttackPhaseEarly();
};