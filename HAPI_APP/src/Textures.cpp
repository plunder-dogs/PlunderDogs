#include "Textures.h"
#include "Global.h"
#include "Utilities/XMLParser.h"
#include <assert.h>

void Textures::loadAllTextures()
{
	assert(!m_allTexturesLoaded);

	XMLParser::loadTexture(m_hexTiles, DATA_DIRECTORY, "hexTiles.xml");
	XMLParser::loadTexture(m_verticalHealthBar, DATA_DIRECTORY, "verticalHealthBar.xml");
	XMLParser::loadTexture(m_mouseCrossHair, DATA_DIRECTORY, "crossHair.xml");
	XMLParser::loadTexture(m_cross, DATA_DIRECTORY, "cross.xml");
	XMLParser::loadTexture(m_spawnHex, DATA_DIRECTORY,"spawnHex.xml");
	XMLParser::loadTexture(m_selectedHex, DATA_DIRECTORY, "whiteHex75.xml");
	XMLParser::loadTexture(m_blueShipSideCannons, SHIP_DATA_DIRECTORY, "shipBlueSide.xml");
	XMLParser::loadTexture(m_blueShipBomb, SHIP_DATA_DIRECTORY, "shipBlueBomb.xml");
	XMLParser::loadTexture(m_blueShipMelee, SHIP_DATA_DIRECTORY, "shipBlueMelee.xml");
	XMLParser::loadTexture(m_blueShipSnipe, SHIP_DATA_DIRECTORY, "shipBlueSnipe.xml");
	XMLParser::loadTexture(m_redShipSideCannons, SHIP_DATA_DIRECTORY, "shipRedSide.xml");
	XMLParser::loadTexture(m_redShipBomb, SHIP_DATA_DIRECTORY, "shipRedBomb.xml");
	XMLParser::loadTexture(m_redShipMelee, SHIP_DATA_DIRECTORY, "shipRedMelee.xml");
	XMLParser::loadTexture(m_redShipSnipe, SHIP_DATA_DIRECTORY, "shipRedSnipe.xml");
	XMLParser::loadTexture(m_greenShipSideCannons, SHIP_DATA_DIRECTORY, "shipGreenSide.xml");
	XMLParser::loadTexture(m_greenShipBomb, SHIP_DATA_DIRECTORY, "shipGreenBomb.xml");
	XMLParser::loadTexture(m_greenShipMelee, SHIP_DATA_DIRECTORY, "shipGreenMelee.xml");
	XMLParser::loadTexture(m_greenShipSnipe, SHIP_DATA_DIRECTORY, "shipGreenSnipe.xml");
	XMLParser::loadTexture(m_yellowShipMelee, SHIP_DATA_DIRECTORY, "shipYellowMelee.xml");
	XMLParser::loadTexture(m_yellowShipSideCannons, SHIP_DATA_DIRECTORY, "shipYellowSide.xml");
	XMLParser::loadTexture(m_yellowShipBomb, SHIP_DATA_DIRECTORY, "shipYellowBomb.xml");
	XMLParser::loadTexture(m_yellowShipSnipe, SHIP_DATA_DIRECTORY, "shipYellowSnipe.xml");
	XMLParser::loadTexture(m_explosionParticles, DATA_DIRECTORY, "explosion.xml");
	XMLParser::loadTexture(m_redSpawnHex, DATA_DIRECTORY, "spawnHexRed.xml");
	XMLParser::loadTexture(m_blueSpawnHex, DATA_DIRECTORY, "spawnHexBlue.xml");
	XMLParser::loadTexture(m_yellowSpawnHex, DATA_DIRECTORY, "spawnHexYellow.xml");
	XMLParser::loadTexture(m_greenSpawnHex, DATA_DIRECTORY, "spawnHexGreen.xml");
	//XMLParser::loadTexture(m_CompassBackGround, DATA_DIRECTORY, "windDirectionArrow.xml");
	XMLParser::loadTexture(m_fireParticles, DATA_DIRECTORY, "fire.xml");

	m_allTexturesLoaded = true;
}