#pragma once

#include "Global.h"

struct ShipOnTile
{
	ShipOnTile()
		: factionName(),
		shipID(INVALID_SHIP_ID)
	{}
	ShipOnTile(FactionName factionName, int shipID)
		: factionName(factionName),
		shipID(shipID)
	{}

	bool operator==(const ShipOnTile& orig)
	{
		return (factionName == orig.factionName && shipID == orig.shipID);
	}

	bool isValid() const
	{
		return (shipID != INVALID_SHIP_ID);
	}

	void clear()
	{
		shipID = INVALID_SHIP_ID;
	}

	FactionName factionName;
	int shipID;
};