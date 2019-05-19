#pragma once

#include "Global.h"
#include "Map.h"
#include "BattleUI.h"
#include "entity.h"

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

		void onYellowShipDestroyed(std::vector<BattlePlayer>& players);
		void onBlueShipDestroyed(std::vector<BattlePlayer>& players);
		void onGreenShipDestroyed(std::vector<BattlePlayer>& players);
		void onRedShipDestroyed(std::vector<BattlePlayer>& players);

	private:
		int m_yellowShipsDestroyed;
		int m_blueShipsDestroyed;
		int m_greenShipsDestroyed;
		int m_redShipsDestroyed;
		void onReset();

		void checkGameStatus(const std::vector<BattlePlayer>& players);
		Timer m_winTimer;
		FactionName m_winningFaction;
		bool m_gameOver;
	};

public:
	Battle();
	~Battle();

	const Map& getMap() const;
	BattlePhase getCurrentPhase() const;

	FactionName getCurrentFaction() const;
	const BattlePlayer& getPlayer(FactionName name) const;
	bool isAIPlaying() const;

	void start(const std::string& newMapName, const std::vector<Player>& newPlayers);
	void render() const;
	void update(float deltaTime);
	void moveEntityToPosition(BattleEntity& entity, const Tile& destination);
	void moveEntityToPosition(BattleEntity& entity, const Tile& destination, eDirection endDirection);

	bool fireEntityWeaponAtPosition(const Tile& tileOnPlayer, const Tile& tileOnAttackPosition, const std::vector<const Tile*>& targetArea);
	void insertEntity(std::pair<int, int> startingPosition, eDirection startingDirection, const EntityProperties& entityProperties, FactionName factionName);
	void nextTurn();

	std::vector<FactionName> getAllFactions() const;
	//const BattlePlayer& getPlayer(FactionName faction) const;
	//std::vector<std::shared_ptr<BattleEntity>>& getFactionShips(FactionName faction);
	//const std::vector<std::shared_ptr<BattleEntity>>& getFactionShips(FactionName faction) const;
	void playFireAnimation(BattleEntity& entity, std::pair<int, int> position);
	void playExplosionAnimation(BattleEntity& entity);
private:
	std::vector<BattlePlayer> m_players;
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
	
	void updateMovementPhase(float deltaTime);
	void updateAttackPhase();

	bool allEntitiesAttacked(std::vector<std::shared_ptr<BattleEntity>>& playerEntities) const;
	BattlePlayer& getPlayer(FactionName factionName);


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