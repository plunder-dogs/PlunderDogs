#include "Packet.hpp"
#include "../../src/Global.h"

sf::Packet & sf::operator>>(sf::Packet & packetReceived, std::vector<eShipType>& vect)
{
	int size = 0;
	packetReceived >> size;
	vect.reserve(static_cast<size_t>(size));
	for (int i = 0; i < size; ++i)
	{
		int shipToAdd = -1;
		packetReceived >> shipToAdd;
		vect.push_back(static_cast<eShipType>(shipToAdd));
	}

	return packetReceived;
}

sf::Packet & sf::operator<<(sf::Packet & packetToSend, const std::vector<eShipType>& vect)
{
	int size = static_cast<int>(vect.size());
	packetToSend << size;
	for (int i = 0; i < size; ++i)
	{
		packetToSend << static_cast<int>(vect[i]);
	}

	return packetToSend;
}

sf::Packet & sf::operator<<(sf::Packet & packetToSend, const ServerMessage & serverMessage)
{
	packetToSend << static_cast<int>(serverMessage.type) << static_cast<int>(serverMessage.faction) << serverMessage.levelName;

	//Ship Action
	packetToSend << static_cast<int>(serverMessage.shipActions.size());
	for (int i = 0; i < serverMessage.shipActions.size(); ++i)
	{
		packetToSend << serverMessage.shipActions[i].shipID << serverMessage.shipActions[i].position.x <<
			serverMessage.shipActions[i].position.y << static_cast<int>(serverMessage.shipActions[i].direction);
	}

	//ShipsToAdd
	packetToSend << static_cast<int>(serverMessage.shipsToAdd.size());
	for (int i = 0; i < serverMessage.shipsToAdd.size(); ++i)
	{
		packetToSend << static_cast<int>(serverMessage.shipsToAdd[i]);
	}

	//SpawnPositions
	packetToSend << static_cast<int>(serverMessage.spawnPositions.size());
	for (int i = 0; i < serverMessage.spawnPositions.size(); ++i)
	{
		packetToSend << static_cast<int>(serverMessage.spawnPositions[i].factionName) <<
			serverMessage.spawnPositions[i].position.x << serverMessage.spawnPositions[i].position.y;
	}

	//Existing Factions
	packetToSend << static_cast<int>(serverMessage.existingFactions.size());
	for (int i = 0; i < serverMessage.existingFactions.size(); ++i)
	{
		packetToSend << static_cast<int>(serverMessage.existingFactions[i].factionName);
		packetToSend << serverMessage.existingFactions[i].ready;
		packetToSend << serverMessage.existingFactions[i].AIControlled;
		packetToSend << static_cast<int>(serverMessage.existingFactions[i].existingShips.size());
		for (int j = 0; j < serverMessage.existingFactions[i].existingShips.size(); ++j)
		{
			packetToSend << static_cast<int>(serverMessage.existingFactions[i].existingShips[j]);
		}
	}

	return packetToSend;
}

sf::Packet & sf::operator>>(sf::Packet & packetReceived, ServerMessage & serverMessage)
{
	int serverMessageType = 0;
	packetReceived >> serverMessageType;
	serverMessage.type = static_cast<eMessageType>(serverMessageType);

	int faction = 0;
	packetReceived >> faction;
	serverMessage.faction = static_cast<eFactionName>(faction);

	packetReceived >> serverMessage.levelName;

	int shipActionsSize = 0;
	packetReceived >> shipActionsSize;
	for (int i = 0; i < shipActionsSize; ++i)
	{
		int shipID = 0, x = 0, y = 0, direction = 0;
		packetReceived >> shipID >> x >> y >> direction;
		serverMessage.shipActions.emplace_back(shipID, x, y, static_cast<eDirection>(direction));
	}

	int shipsToAddSize = 0;
	packetReceived >> shipsToAddSize;
	for (int i = 0; i < shipsToAddSize; ++i)
	{
		int shipToAdd = 0;
		packetReceived >> shipToAdd;
		serverMessage.shipsToAdd.push_back(static_cast<eShipType>(shipToAdd));
	}

	int spawnPositionsSize = 0;
	packetReceived >> spawnPositionsSize;
	for (int i = 0; i < spawnPositionsSize; ++i)
	{
		int factionName = 0, x = 0, y = 0;
		packetReceived >> factionName >> x >> y;
		serverMessage.spawnPositions.emplace_back(static_cast<eFactionName>(factionName), x, y);
	}

	int existingFactionsSize = 0;
	packetReceived >> existingFactionsSize;
	for (int i = 0; i < existingFactionsSize; ++i)
	{
		int existingFactionName = 0;
		packetReceived >> existingFactionName;
		bool ready;
		packetReceived >> ready;
		bool AIControlled;
		packetReceived >> AIControlled;
		int existingShipsSize = 0;
		packetReceived >> existingShipsSize;
		std::vector<eShipType> existingShips;
		for (int j = 0; j < existingShipsSize; ++j)
		{
			int existingShip = 0;
			packetReceived >> existingShip;
			existingShips.emplace_back(static_cast<eShipType>(existingShip));
		}
		serverMessage.existingFactions.emplace_back(static_cast<eFactionName>(existingFactionName), std::move(existingShips), AIControlled, ready);
	}

	return packetReceived;
}