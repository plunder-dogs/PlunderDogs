#pragma once

#include "Texture.h"
#include "Utilities/NonCopyable.h"
#include <memory>
#include <unordered_map>
#include <string>

//Texture names
const std::string HEX_TILES = "hexTiles.xml";
const std::string CROSSHAIR = "crossHair.xml";
const std::string CROSS = "cross.xml";
const std::string SPAWN_HEX = "spawnHex.xml";
const std::string SELECTED_HEX = "whiteHex75.xml";
const std::string BLUE_SHIP_SIDE_CANNONS = "shipBlueSide.xml";
const std::string BLUE_SHIP_BOMB = "shipBlueBomb.xml";
const std::string BLUE_SHIP_MELEE = "shipBlueMelee.xml";
const std::string BLUE_SHIP_SNIPE = "shipBlueSnipe.xml";
const std::string RED_SHIP_SIDE_CANNONS = "shipRedSide.xml";
const std::string RED_SHIP_BOMB = "shipRedBomb.xml";
const std::string RED_SHIP_MELEE = "shipRedMelee.xml";
const std::string RED_SHIP_SNIPE = "shipRedSnipe.xml";
const std::string GREEN_SHIP_SIDE_CANNONS = "shipGreenSide.xml";
const std::string GREEN_SHIP_BOMB = "shipGreenBomb.xml";
const std::string GREEN_SHIP_MELEE = "ShipGreenMelee.xml";
const std::string GREEN_SHIP_SNIPE = "shipGreenSnipe.xml";
const std::string YELLOW_SHIP_MELEE = "shipYellowMelee.xml";
const std::string YELLOW_SHIP_SIDE_CANNONS = "shipYellowSide.xml";
const std::string YELLOW_SHIP_BOMB = "shipYellowBomb.xml";
const std::string YELLOW_SHIP_SNIPE = "shipYellowSnipe.xml";
const std::string EXPLOSION_PARTICLES = "explosion.xml";
const std::string RED_SPAWN_HEX = "spawnHexRed.xml";
const std::string BLUE_SPAWN_HEX = "spawnHexBlue.xml";
const std::string YELLOW_SPAWN_HEX = "spawnHexYellow.xml";
const std::string GREEN_SPAWN_HEX = "spawnHexGreen.xml";
const std::string FIRE_PARTICLES = "fire.xml";
const std::string BACKGROUND = "background.xml";

class Textures : private NonCopyable
{
public:
	static Textures& getInstance()
	{
		static Textures textures;
		return textures;
	}

	bool loadAllTextures();	
	const Texture& getTexture(const std::string& textureName) const;

private:
	Textures() {}
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
	bool m_allTexturesLoaded = false;

	bool loadTexture(const std::string& fileName, const std::string& directory);
};