#pragma once

#include "Global.h"

struct ServerMessageDeploy
{
	ServerMessageDeploy(ShipOnTile shipToDeploy, sf::Vector2i deployAtPosition)
		: shipToDeploy(shipToDeploy),
		deployAtPosition(deployAtPosition)
	{}

	ShipOnTile shipToDeploy;
	sf::Vector2i deployAtPosition;
};