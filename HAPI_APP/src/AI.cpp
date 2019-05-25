#include <vector>
#include "AI.h"
#include "MouseSelection.h"
#include "BFS.h"
#include "Battle.h"
#include "Map.h"
#include "entity.h"
#include <ctime>
#include <cstdlib>

//Finds closest living enemy, returns nullptr if none found
const Tile* findClosestEnemy(const Battle& battle, const Map& map, std::pair<int, int> alliedShipPosition, FactionName faction);
//Finds nearest firing position to ship, if none are found will return the tile the ship is on
std::pair<const Tile*, eDirection> findFiringPosition(const Map& mapPtr, const Tile* targetShip, const Tile* alliedShip, eWeaponType weapon, int range);
void attemptMove(Map& map, std::unique_ptr<Ship>& currentShip, std::pair<const Tile*, eDirection> targetTile);
void attemptShot(Battle& battle, const Map& mapPtr, std::unique_ptr<Ship>& firingShip);

void AI::handleMovementPhase(const Battle& battle, Map& map, std::unique_ptr<Player>& battlePlayer, int currentUnit)
{
	auto& ships = battlePlayer->m_ships;

	//if (ships[currentUnit]->m_battleProperties.isDead()) return;
	//find the nearest enemy ship
	const Tile* enemyPosition{ findClosestEnemy(battle, map, ships[currentUnit]->getCurrentPosition(), battlePlayer->m_factionName) };
	if (!enemyPosition)
	{
		ships[currentUnit]->setDestination();
		return;
	}
	//find the nearest tile and facing that can fire upon the chosen enemy ship
	std::pair<const Tile*, eDirection>  firingPosition{ findFiringPosition(
		map,
		enemyPosition,
		map.getTile(ships[currentUnit]->getCurrentPosition()),
		static_cast<eWeaponType>(ships[currentUnit]->getWeaponType()),
		ships[currentUnit]->getRange())};

	//move as far as possible on the path to the chosen position
	attemptMove(map, ships[currentUnit], firingPosition);

	////loop through all the ships in the faction
	//for (int i = 0; i < ships.size(); i++)
	//{
	//	if (ships[i]->m_battleProperties.isDead()) continue;
	//	//find the nearest enemy ship
	//	const Tile* enemyPosition{ findClosestEnemy(battle, map, ships[i]->m_battleProperties.getCurrentPosition(), battlePlayer.m_factionName) };
	//	if (!enemyPosition)
	//	{
	//		ships[i]->m_battleProperties.setDestination();
	//		continue;
	//	}
	//	//find the nearest tile and facing that can fire upon the chosen enemy ship
	//	std::pair<const Tile*, eDirection>  firingPosition{ AI::findFiringPosition(
	//		map, 
	//		enemyPosition, 
	//		map.getTile(ships[i]->m_battleProperties.getCurrentPosition()), 
	//		static_cast<eWeaponType>(ships[i]->getProperties().m_weaponType), 
	//		ships[i]->getProperties().m_range) };

	//	//move as far as possible on the path to the chosen position
	//	attemptMove(map, ships[i], firingPosition);
	//}
}

void AI::handleShootingPhase(Battle& battle, const Map& map, std::unique_ptr<Player>& player, int currentUnit)
{
	//if (player.m_entities[i]->m_battleProperties.isDead()) continue;
	//check if the ship is able to fire upon any enemies and fire if possible
	attemptShot(battle, map, player->m_ships[currentUnit]);

	//loop through all the ships in the faction
	//for (int i = 0; i < player.m_entities.size(); i++)
	//{
	//	if (player.m_entities[i]->m_battleProperties.isDead()) continue;
	//	//check if the ship is able to fire upon any enemies and fire if possible
	//	attemptShot(battle, map, player.m_entities[i]);
	//}
}

//void AI::handleDeploymentPhase(Battle& battle, Map& map, Player& bPlayer, const Player& player)
//{
//	std::vector<Tile*> spawnArea{ map.getTileRadius(bPlayer,3,true,true) };
//	assert(spawnArea.size() > 6);
//	int location = static_cast<int>(std::rand() % (spawnArea.size() - 6));
//	int spawnPoint{ location };
//	eDirection randomDir = static_cast<eDirection>(std::rand() % 6);
//	for (int i = 0; i < player.m_ships.size(); i++)
//	{
//		//battle.insertEntity(spawnArea[spawnPoint]->m_tileCoordinate, randomDir, *player.m_s[i], player.m_factionName);
//		spawnPoint++;
//	}
//}

//void AI::handleShipSelection(std::vector<ShipGlobalProperties>& shipPool, std::vector<ShipGlobalProperties*>& selectedShips)
//{
//	int randomNumber{ std::rand() % 8 };
//	int numSideCannons{ 0 };
//	int numOfAddedSideCannons{ 0 };
//	int numTurtle{ 0 };
//	int numOfAddedTurtle{ 0 };
//	int numFlame{ 0 };
//	int numOfAddedFlame{ 0 };
//	int numSniper{ 0 };
//	int numOfAddedSniper{ 0 };
//
//	switch (randomNumber)
//	{
//	case 0:
//		numFlame = 4;
//		numSideCannons = 2;
//		break;
//	case 1:
//		numTurtle = 2;
//		numSideCannons = 2;
//		numSniper = 2;
//		break;
//	case 2:
//		numTurtle = 3;
//		numSideCannons = 3;
//		break;
//	case 3:
//		numFlame = 2;
//		numSideCannons = 2;
//		numTurtle = 2;
//		break;
//	case 4:
//		numSniper = 4;
//		numSideCannons = 2;
//		break;
//	case 5:
//		numFlame = 3;
//		numSideCannons = 3;
//		break;
//	case 6:
//		numSideCannons = 2;
//		numTurtle = 1;
//		numFlame = 2;
//		numSniper = 1;
//		break;
//	case 7:
//		numFlame = 4;
//		numSniper = 2;
//		break;
//	}
//
//	assert(numSideCannons + numTurtle + numFlame + numSniper < 7);
//	for (unsigned int i = 0; i < shipPool.size() && numOfAddedFlame < numFlame; i++)
//	{
//		if (shipPool[i].m_weaponType == eFlamethrower)
//		{
//			ShipGlobalProperties* tmp{ &shipPool[i] };
//			selectedShips.push_back(tmp);
//			numOfAddedFlame++;
//		}
//	}
//	for (unsigned int i = 0; i < shipPool.size() && numOfAddedSideCannons < numSideCannons; i++)
//	{
//		if (shipPool[i].m_weaponType == eSideCannons)
//		{
//			ShipGlobalProperties* tmp{ &shipPool[i] };
//			selectedShips.push_back(tmp);
//			numOfAddedSideCannons++;
//		}
//	}
//	for (unsigned int i = 0; i < shipPool.size() && numOfAddedTurtle < numTurtle; i++)
//	{
//		if (shipPool[i].m_weaponType == eShotgun)
//		{
//			ShipGlobalProperties* tmp{ &shipPool[i] };
//			selectedShips.push_back(tmp);
//			numOfAddedTurtle++;
//		}
//	}
//	for (unsigned int i = 0; i < shipPool.size() && numOfAddedSniper < numSniper; i++)
//	{
//		if (shipPool[i].m_weaponType == eStraightShot)
//		{
//			ShipGlobalProperties* tmp{ &shipPool[i] };
//			selectedShips.push_back(tmp);
//			numOfAddedSniper++;
//		}
//	}
//}

const Tile* findClosestEnemy(const Battle& battle, const Map& map, std::pair<int, int> alliedShipPosition, FactionName ourFaction)
{
	const Tile* closestEnemy{ nullptr };
	int closestDistance{ INT_MAX };
	std::pair<int, int> alliedPos{ MouseSelection::coordToHexPos(alliedShipPosition) };
	auto activeFactions = battle.getAllFactions();
	for (FactionName i : activeFactions)
	{
		if (i == static_cast<int>(ourFaction))
			continue;

		const auto& factionShips = battle.getPlayer(static_cast<FactionName>(i)).m_ships;

		for (int j = 0; j < factionShips.size(); j++)
		{
			if (factionShips[j]->isDead()) continue;
			//Find the distance^2 from the allied ship to the enemy ship, 
			//then set closestEnemy to that enemy if it's the closest yet found
			std::pair<int, int> enemyPos = MouseSelection::coordToHexPos(
				factionShips[j]->getCurrentPosition());

			std::pair<int, int> diff(
				{ enemyPos.first - alliedPos.first, enemyPos.second - alliedPos.second });
			int enemyDistance = diff.first * diff.first + diff.second * diff.second;
			
			if (enemyDistance < closestDistance)
			{
				closestDistance = enemyDistance;
				closestEnemy = map.getTile(factionShips[j]->getCurrentPosition());
			}
		}
	}
	return closestEnemy;
}

//Support function for AI::findFiringPositions. 
//Finds the closest tile a specified distance away from a target tile
const Tile* firePosRadial(const Map& map, const Tile* targetShip, const Tile* alliedShip, int range)
{
	const Tile* closestTile{ alliedShip };
	int closestDistance{ INT_MAX };
	std::pair<int, int> alliedPos{ MouseSelection::coordToHexPos(alliedShip->m_tileCoordinate) };
	std::vector<const Tile*> availableTiles{ map.cGetTileRing(targetShip->m_tileCoordinate, range) };
	for (const Tile* it : availableTiles)
	{
		//Ensure it's a valid tile, if not skip this one
		if (!it) continue;
		if (it->m_type != eSea && it->m_type != eOcean) continue;
		if (it->m_shipOnTile) continue;
		//Determine distance
		std::pair<int, int> tempPos = MouseSelection::coordToHexPos(it->m_tileCoordinate);
		std::pair<int, int> diff(
			{ tempPos.first - alliedPos.first, tempPos.second - alliedPos.second });
		int tempDistance = diff.first * diff.first + diff.second * diff.second;
		//If distance is smallest, set as new target tile
		if (tempDistance < closestDistance)
		{
			closestDistance = tempDistance;
			closestTile = it;
		}
	}
	return closestTile;
}

//Support function for AI::findFiringPositions. 
//Finds the closest tile for line firing by projecting lines of length "range"
const Tile* firePosLine(const Map& map, const Tile* targetShip, const Tile* alliedShip, int range)
{
	const Tile* closestTile{ alliedShip };
	int closestDistance{ INT_MAX };
	std::pair<int, int> alliedPos{ MouseSelection::coordToHexPos(alliedShip->m_tileCoordinate) };
	//TODO: can't use const Tile* for some reason
	std::vector<const Tile*> availableTiles;
	availableTiles.reserve(range);
	//Iterate through all 6 possible lines
	for (int i = 0; i < 6; i++)
	{
		availableTiles = map.cGetTileLine(targetShip->m_tileCoordinate, range, static_cast<eDirection>(i), true);
		for (const Tile* it : availableTiles)
		{
			//Ensure it's a valid tile, if not skip this one
			if (!it) continue;
			if (it->m_type != eSea && it->m_type != eOcean) continue;
			if (it->m_shipOnTile) continue;
			//Determine distance
			std::pair<int, int> tempPos = MouseSelection::coordToHexPos(it->m_tileCoordinate);
			std::pair<int, int> diff(
				{ tempPos.first - alliedPos.first, tempPos.second - alliedPos.second });
			int tempDistance = diff.first * diff.first + diff.second * diff.second;
			//If distance is smallest, set as new target tile
			if (tempDistance < closestDistance)
			{
				closestDistance = tempDistance;
				closestTile = it;
			}
		}
	}
	return closestTile;
}

std::pair<const Tile*, eDirection> findFiringPosition(const Map& map, const Tile* targetShip, const Tile* alliedShip, eWeaponType weapon, int range)
{
	const Tile* closestTile{ alliedShip };
	eDirection facingDirection{ eNorth };
	switch (weapon)
	{
	case eSideCannons:
	{
		closestTile = firePosRadial(map, targetShip, alliedShip, range);
		facingDirection = MouseSelection::calculateDirection(closestTile, targetShip).second;
		switch (facingDirection)
		{
		case eNorth: facingDirection = eNorthEast;
			break;
		case eNorthEast: facingDirection = eSouthEast;
			break;
		case eSouthEast: facingDirection = eSouth;
			break;
		case eSouth: facingDirection = eSouthWest;
			break;
		case eSouthWest: facingDirection = eNorthWest;
			break;
		case eNorthWest: facingDirection = eNorth;
			break;
		}
		break;
	}
	case eStraightShot:
	{
		closestTile = firePosLine(map, targetShip, alliedShip, range);
		facingDirection = MouseSelection::calculateDirection(closestTile, targetShip).second;
		break;
	}
	case eShotgun:
	{
		closestTile = firePosRadial(map, targetShip, alliedShip, range);
		facingDirection = MouseSelection::calculateDirection(closestTile, targetShip).second;
		break;
	}
	case eFlamethrower:
	{
		closestTile = firePosLine(map, targetShip, alliedShip, range);
		facingDirection = MouseSelection::calculateDirection(closestTile, targetShip).second;
		switch(facingDirection)
		{
		case eNorth: facingDirection = eSouth;
			break;
		case eNorthEast: facingDirection = eSouthWest;
			break;
		case eSouthEast: facingDirection = eNorthWest;
			break;
		case eSouth: facingDirection = eNorth;
			break;
		case eSouthWest: facingDirection = eNorthEast;
			break;
		case eNorthWest: facingDirection = eSouthEast;
			break;
		}
		break;
	}
	}
	return { closestTile, facingDirection };
}

void attemptMove(Map& map, std::unique_ptr<Ship>& currentShip, std::pair<const Tile*, eDirection> targetTile)
{
	//Call generate path
	const Tile* tile{ map.getTile(currentShip->getCurrentPosition()) };
	//int pathLength = currentShip->m_battleProperties.generateMovementGraph(map, *tile, *targetTile.first);
	auto availableTiles = BFS::findArea(
		map,
		posi(currentShip->getCurrentPosition(), currentShip->getCurrentDirection()),
		static_cast<float>(currentShip->getMovementPoints()));
	//Loop to find the closest tile to the target tile
	std::pair<int, int> targetPos = targetTile.first->m_tileCoordinate;
	std::pair<int, int> currentPos = currentShip->getCurrentPosition();
	int closestDistance{ INT_MAX };
	posi bestTile{ -1, -1, eNorth };
	for (posi it : availableTiles)
	{
		std::pair<int, int> diff(
			{ targetPos.first - it.x, targetPos.second - it.y });
		int tileDistance = diff.first * diff.first + diff.second * diff.second;

		if (tileDistance < closestDistance)
		{
			closestDistance = tileDistance;
			bestTile = it;
		}
	}
	//If a tile is found
	if (bestTile != posi(-1, -1, eNorth))
	{
		currentShip->generateMovementGraph(map, *map.getTile(currentPos), *map.getTile(bestTile.pair()));
		currentShip->moveEntity(map, *map.getTile(bestTile.pair()), targetTile.second);
	}
	currentShip->setDestination();
}

void attemptShot(Battle& battle, const Map& map, std::unique_ptr<Ship>& firingShip)
{
	std::vector< const Tile*> firingArea;
	switch (firingShip->getWeaponType())
	{
	case eSideCannons:
	{
		firingArea = map.cGetTileCone(firingShip->getCurrentPosition(), firingShip->getRange(), firingShip->getCurrentDirection());
		for (int i = 0; i < firingArea.size(); i++)
		{
			if (!firingArea[i]) continue;
			if (!firingArea[i]->m_shipOnTile) continue;
			if (firingArea[i]->m_shipOnTile->getFactionName() == firingShip->getFactionName() || firingArea[i]->m_shipOnTile->isDead()) continue;
			const Tile& tileOnFiringShip = *map.getTile(firingShip->getCurrentPosition());
			battle.fireEntityWeaponAtPosition(tileOnFiringShip, *firingArea[i], firingArea);
			break;
		}
		break;
	}
	case eStraightShot:
	{
		firingArea = map.cGetTileLine(firingShip->getCurrentPosition(), firingShip->getRange(), firingShip->getCurrentDirection(), true);
		for (int i = 0; i < firingArea.size(); i++)
		{
			if (!firingArea[i]) continue;
			if (!firingArea[i]->m_shipOnTile) continue;
			if (firingArea[i]->m_shipOnTile->getFactionName() == firingShip->getFactionName() || firingArea[i]->m_shipOnTile->isDead()) continue;
			const Tile& tileOnFiringShip = *map.getTile(firingShip->getCurrentPosition());
			battle.fireEntityWeaponAtPosition(tileOnFiringShip, *firingArea[i], firingArea);
			
			//battle.fireEntityWeaponAtPosition(*firingShip, *firingArea[i], firingArea);
			break;
		}
		break;
	}
	case eShotgun:
	{
		firingArea = map.cGetTileRadius(firingShip->getCurrentPosition(), firingShip->getRange());
		for (int i = 0; i < firingArea.size(); i++)
		{
			if (!firingArea[i]) continue;
			if (!firingArea[i]->m_shipOnTile) continue;
			if (firingArea[i]->m_shipOnTile->getFactionName() == firingShip->getFactionName() || firingArea[i]->m_shipOnTile->isDead()) continue;
			const Tile& tileOnFiringShip = *map.getTile(firingShip->getCurrentPosition());
			battle.fireEntityWeaponAtPosition(tileOnFiringShip, *firingArea[i], firingArea);

			//battle.fireEntityWeaponAtPosition(*firingShip, *firingArea[i], firingArea);
			break;
		}
		break;
	}
	case eFlamethrower:
	{
		eDirection backwardsDirection{ eNorth };
		switch(firingShip->getCurrentDirection())
		{
		case eNorth: backwardsDirection = eSouth;
			break;
		case eNorthEast: backwardsDirection = eSouthWest;
			break;
		case eSouthEast: backwardsDirection = eNorthWest;
			break;
		case eSouth: backwardsDirection = eNorth;
			break;
		case eSouthWest: backwardsDirection = eNorthEast;
			break;
		case eNorthWest: backwardsDirection = eSouthEast;
			break;
		}
			
		firingArea = map.cGetTileLine(firingShip->getCurrentPosition(), firingShip->getRange(), backwardsDirection, true);
		for (int i = 0; i < firingArea.size(); i++)
		{
			if (!firingArea[i]) continue;
			if (!firingArea[i]->m_shipOnTile) continue;
			if (firingArea[i]->m_shipOnTile->getFactionName() == firingShip->getFactionName() || firingArea[i]->m_shipOnTile->isDead()) continue;
			const Tile& tileOnFiringShip = *map.getTile(firingShip->getCurrentPosition());
			battle.fireEntityWeaponAtPosition(tileOnFiringShip, *firingArea[i], firingArea);

			//battle.fireEntityWeaponAtPosition(*firingShip, *firingArea[i], firingArea);
			break;
		}
		break;
	}
	}
	firingShip->fireWeapon();
}