#pragma once



//TODO: temp





//
//struct Faction
//{
//	Faction(FactionName name, ePlayerType playerType);
//	
//	void render(const Map& map) const;
//	void onNewTurn();
//
//	//Deployment Phase
//	void addShip(FactionName factionName, eShipType shipType);
//	void createSpawnArea(Map& map);
//	void deployShipAtPosition(std::pair<int, int> startingPosition, eDirection startingDirection);
//	void setDeploymentAtPosition();
//	//Movement Phase
//	bool moveShipToPosition(Map& map, int shipID, std::pair<int, int> destination);
//	bool moveShipToPosition(Map& map, int shipID, std::pair<int, int> destination, eDirection endDirection);
//	void generateShipMovementPath(const Map& map, int shipID, std::pair<int, int> destination);
//	void disableShipMovementPath(int shipID);
//	//Attack Phase
//	void shipTakeDamage(int shipID, int damage);
//	
//	std::vector<Ship> m_ships;
//	const FactionName m_factionName;
//	const ePlayerType m_playerType;
//	bool m_eliminated;
//	Ship* m_shipToDeploy;
//	std::vector<SpawnNode> m_spawnArea;
//};