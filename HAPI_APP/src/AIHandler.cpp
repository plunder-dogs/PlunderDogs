#include <vector>
#include "AIHandler.h"
#include "Utilities/Utilities.h"
#include "PathFinding.h"
#include "Battle.h"
#include "Map.h"
#include "Global.h"
#include <ctime>
#include <cstdlib>
#include <assert.h>

//Finds closest living enemy, returns nullptr if none found
const Tile* findClosestEnemy(const Battle& battle, const Map& map, sf::Vector2i alliedShipPosition, FactionName faction);
////Finds nearest firing position to ship, if none are found will return the tile the ship is on
std::pair<const Tile*, eDirection> findFiringPosition(const Map& mapPtr, const Tile* targetShip, const Tile* alliedShip, eShipType shipType, int range);
void attemptMove(std::vector<Ray2D>& movementArea, const Faction& faction, Map& map, Ship& currentShip, std::pair<const Tile*, eDirection> targetTile);
void attemptShot(std::vector<const Tile*>& targetArea, Battle& battle, const Map& mapPtr, Ship& firingShip);

void AIHandler::handleMovementPhase(const Battle& battle, Map& map, Faction& battlePlayer, int currentUnit)
{
	auto& ships = battlePlayer.m_ships;

	//if (ships[currentUnit]->m_battleProperties.isDead()) return;
	//find the nearest enemy ship
	const Tile* enemyPosition{ findClosestEnemy(battle, map, ships[currentUnit].getCurrentPosition(), battlePlayer.m_factionName) };
	if (!enemyPosition)
	{
		ships[currentUnit].setDestination();
		return;
	}
	//find the nearest tile and facing that can fire upon the chosen enemy ship
	std::pair<const Tile*, eDirection>  firingPosition{ findFiringPosition(
		map,
		enemyPosition,
		map.getTile(ships[currentUnit].getCurrentPosition()),
		ships[currentUnit].getShipType(),
		ships[currentUnit].getRange())};

	//move as far as possible on the path to the chosen position
	attemptMove(m_movementArea, battle.getCurrentFaction(), map, ships[currentUnit], firingPosition);
}

void AIHandler::handleShootingPhase(Battle& battle, const Map& map, Faction& player, int currentUnit)
{
	//if (player.m_entities[i]->m_battleProperties.isDead()) continue;
	//check if the ship is able to fire upon any enemies and fire if possible
	attemptShot(m_targetArea, battle, map, player.m_ships[currentUnit]);
}

void AIHandler::handleDeploymentPhase(Battle& battle, const Faction& currentPlayer)
{	
	assert(currentPlayer.m_controllerType == eControllerType::eAI);

	for (const auto& i : currentPlayer.m_spawnArea.m_tileArea)
	{
		battle.deployFactionShipAtPosition(i->m_tileCoordinate, eDirection::eNorth);
		if (currentPlayer.isAllShipsDeployed())
		{
			break;
		}
	}
}

void AIHandler::loadShips(Faction& player)
{
	assert(player.m_controllerType == eControllerType::eAI);

	int randomNumber{ std::rand() % 8 };
	int numSideCannons{ 0 };
	int numTurtle{ 0 };
	int numFlame{ 0 };
	int numSniper{ 0 };

	switch (randomNumber)
	{
	case 0:
		numFlame = 4;
		numSideCannons = 2;
		break;
	case 1:
		numTurtle = 2;
		numSideCannons = 2;
		numSniper = 2;
		break;
	case 2:
		numTurtle = 3;
		numSideCannons = 3;
		break;
	case 3:
		numFlame = 2;
		numSideCannons = 2;
		numTurtle = 2;
		break;
	case 4:
		numSniper = 4;
		numSideCannons = 2;
		break;
	case 5:
		numFlame = 3;
		numSideCannons = 3;
		break;
	case 6:
		numSideCannons = 2;
		numTurtle = 1;
		numFlame = 2;
		numSniper = 1;
		break;
	case 7:
		numFlame = 4;
		numSniper = 2;
		break;
	}

	assert(numSideCannons + numTurtle + numFlame + numSniper < 7);
	for (int i = 0; i < numFlame; ++i)
	{
		player.addShip(player.m_factionName, eShipType::eFire);
	}
	for (int i = 0; i < numSideCannons; ++i)
	{
		player.addShip(player.m_factionName, eShipType::eFrigate);
	}
	for (int i = 0; i < numTurtle; ++i)
	{
		player.addShip(player.m_factionName, eShipType::eTurtle);
	}
	for (int i = 0; i < numSniper; ++i)
	{
		player.addShip(player.m_factionName, eShipType::eSniper);
	}
}

const Tile* findClosestEnemy(const Battle& battle, const Map& map, sf::Vector2i alliedShipPosition, FactionName ourFaction)
{
	const Tile* closestEnemy{ nullptr };
	int closestDistance{ INT_MAX };
	sf::Vector2i alliedPos{ Math::coordToHexPos(alliedShipPosition) };
	auto activeFactions = battle.getAllFactionsInPlay();
	for (FactionName i : activeFactions)
	{
		if (i == static_cast<int>(ourFaction))
			continue;

		const auto& factionShips = battle.getFaction(static_cast<FactionName>(i)).m_ships;

		for (int j = 0; j < factionShips.size(); j++)
		{
			if (factionShips[j].isDead()) continue;
			//Find the distance^2 from the allied ship to the enemy ship, 
			//then set closestEnemy to that enemy if it's the closest yet found
			sf::Vector2i enemyPos = Math::coordToHexPos(
				factionShips[j].getCurrentPosition());

			sf::Vector2i diff(
				{ enemyPos.x - alliedPos.x, enemyPos.y - alliedPos.y });
			int enemyDistance = diff.x * diff.x + diff.y * diff.y;
			
			if (enemyDistance < closestDistance)
			{
				closestDistance = enemyDistance;
				closestEnemy = map.getTile(factionShips[j].getCurrentPosition());
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
	sf::Vector2i alliedPos{ Math::coordToHexPos(alliedShip->m_tileCoordinate) };
	std::vector<const Tile*> availableTiles{ map.getTileRing(targetShip->m_tileCoordinate, range) };
	for (const Tile* it : availableTiles)
	{
		//Ensure it's a valid tile, if not skip this one
		if (!it) continue;
		if (it->m_type != eSea && it->m_type != eOcean) continue;
		if (it->m_shipOnTile.isValid()) continue;
		//Determine distance
		sf::Vector2i tempPos = Math::coordToHexPos(it->m_tileCoordinate);
		sf::Vector2i diff(
			{ tempPos.x - alliedPos.x, tempPos.y - alliedPos.y });
		int tempDistance = diff.x * diff.x + diff.y * diff.y;
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
	sf::Vector2i alliedPos{ Math::coordToHexPos(alliedShip->m_tileCoordinate) };
	//TODO: can't use const Tile* for some reason
	std::vector<const Tile*> availableTiles;
	availableTiles.reserve(range);
	//Iterate through all 6 possible lines
	for (int i = 0; i < 6; i++)
	{
		availableTiles = map.getTileLine(targetShip->m_tileCoordinate, range, static_cast<eDirection>(i), true);
		for (const Tile* it : availableTiles)
		{
			//Ensure it's a valid tile, if not skip this one
			if (!it) continue;
			if (it->m_type != eSea && it->m_type != eOcean) continue;
			if (it->m_shipOnTile.isValid()) continue;
			//Determine distance
			sf::Vector2i tempPos = Math::coordToHexPos(it->m_tileCoordinate);
			sf::Vector2i diff(
				{ tempPos.x - alliedPos.x, tempPos.y - alliedPos.y });
			int tempDistance = diff.x * diff.x + diff.y * diff.y;
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

std::pair<const Tile*, eDirection> findFiringPosition(const Map& map, const Tile* targetShip, const Tile* alliedShip, eShipType shipType, int range)
{
	const Tile* closestTile{ alliedShip };
	eDirection facingDirection{ eNorth };
	switch (shipType)
	{
	case eShipType::eFrigate:
	{
		closestTile = firePosRadial(map, targetShip, alliedShip, range);
		facingDirection = Math::calculateDirection(closestTile, targetShip).second;
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
	case eShipType::eSniper:
	{
		closestTile = firePosLine(map, targetShip, alliedShip, range);
		facingDirection = Math::calculateDirection(closestTile, targetShip).second;
		break;
	}
	case eShipType::eTurtle:
	{
		closestTile = firePosRadial(map, targetShip, alliedShip, range);
		facingDirection = Math::calculateDirection(closestTile, targetShip).second;
		break;
	}
	case eShipType::eFire:
	{
		closestTile = firePosLine(map, targetShip, alliedShip, range);
		facingDirection = Math::calculateDirection(closestTile, targetShip).second;
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

void attemptMove(std::vector<Ray2D>& movementArea, const Faction& faction, Map& map, Ship& currentShip, std::pair<const Tile*, eDirection> targetTile)
{
	//Call generate path
	const Tile* tile{ map.getTile(currentShip.getCurrentPosition()) };
	//int pathLength = currentShip->m_battleProperties.generateMovementGraph(map, *tile, *targetTile.x);
	movementArea.clear();
	movementArea = PathFinding::getInstance().findArea(
		map,
		Ray2D(currentShip.getCurrentPosition(), currentShip.getCurrentDirection()),
		static_cast<float>(currentShip.getMovementPoints()));
	//Loop to find the closest tile to the target tile
	sf::Vector2i targetPos = targetTile.first->m_tileCoordinate;
	sf::Vector2i currentPos = currentShip.getCurrentPosition();
	int closestDistance{ INT_MAX };
	Ray2D bestTile{ -1, -1, eNorth };
	for (Ray2D it : movementArea)
	{
		sf::Vector2i diff(
			{ targetPos.x - it.x, targetPos.y - it.y });
		int tileDistance = diff.x * diff.x + diff.y * diff.y;

		if (tileDistance < closestDistance)
		{
			closestDistance = tileDistance;
			bestTile = it;
		}
	}
	//If a tile is found
	if (bestTile != Ray2D(-1, -1, eNorth))
	{
		currentShip.generateMovementArea(faction, map, map.getTile(bestTile.pair())->m_tileCoordinate, true);
		currentShip.startMovement(map, targetTile.second);		
	}
	currentShip.setDestination();
}

void attemptShot(std::vector<const Tile*>& targetArea, Battle& battle, const Map& map, Ship& firingShip)
{
	targetArea.clear();
	switch (firingShip.getShipType())
	{
	case eShipType::eTurtle:
	{
		targetArea = map.getTileCone(firingShip.getCurrentPosition(), firingShip.getRange(), firingShip.getCurrentDirection());
		for (int i = 0; i < targetArea.size(); i++)
		{
			if (!targetArea[i]) continue;
			if (!targetArea[i]->m_shipOnTile.isValid()) continue;
			if (targetArea[i]->m_shipOnTile.factionName == firingShip.getFactionName() || battle.getFactionShip(targetArea[i]->m_shipOnTile).isDead()) continue;
			const Tile& tileOnFiringShip = *map.getTile(firingShip.getCurrentPosition());
			battle.fireFactionShipAtPosition(tileOnFiringShip.m_shipOnTile, *targetArea[i], targetArea);
			break;
		}
		break;
	}
	case eShipType::eSniper:
	{
		targetArea = map.getTileLine(firingShip.getCurrentPosition(), firingShip.getRange(), firingShip.getCurrentDirection(), true);
		for (int i = 0; i < targetArea.size(); i++)
		{
			if (!targetArea[i]) continue;
			if (!targetArea[i]->m_shipOnTile.isValid()) continue;
			if (targetArea[i]->m_shipOnTile.factionName == firingShip.getFactionName() || battle.getFactionShip(targetArea[i]->m_shipOnTile).isDead()) continue;
			const Tile& tileOnFiringShip = *map.getTile(firingShip.getCurrentPosition());
			battle.fireFactionShipAtPosition(tileOnFiringShip.m_shipOnTile, *targetArea[i], targetArea);
			break;
		}
		break;
	}
	case eShipType::eFrigate:
	{
		targetArea = map.getTileRadius(firingShip.getCurrentPosition(), firingShip.getRange());
		for (int i = 0; i < targetArea.size(); i++)
		{
			if (!targetArea[i]) continue;
			if (!targetArea[i]->m_shipOnTile.isValid()) continue;
			if (targetArea[i]->m_shipOnTile.factionName == firingShip.getFactionName() || battle.getFactionShip(targetArea[i]->m_shipOnTile).isDead()) continue;
			const Tile& tileOnFiringShip = *map.getTile(firingShip.getCurrentPosition());
			battle.fireFactionShipAtPosition(tileOnFiringShip.m_shipOnTile, *targetArea[i], targetArea);
			break;
		}
		break;
	}
	case eShipType::eFire:
	{
		eDirection backwardsDirection{ eNorth };
		switch(firingShip.getCurrentDirection())
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
			
		targetArea = map.getTileLine(firingShip.getCurrentPosition(), firingShip.getRange(), backwardsDirection, true);
		for (int i = 0; i < targetArea.size(); i++)
		{
			if (!targetArea[i]) continue;
			if (!targetArea[i]->m_shipOnTile.isValid()) continue;
			if (targetArea[i]->m_shipOnTile.factionName == firingShip.getFactionName() || battle.getFactionShip(targetArea[i]->m_shipOnTile).isDead()) continue;
			const Tile& tileOnFiringShip = *map.getTile(firingShip.getCurrentPosition());
			battle.fireFactionShipAtPosition(tileOnFiringShip.m_shipOnTile, *targetArea[i], targetArea);
			break;
		}
		break;
	}
	}

	firingShip.fireWeapon();
}