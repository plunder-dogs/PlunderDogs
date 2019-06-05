#include "Textures.h"
#include "Global.h"
#include <assert.h>

std::unique_ptr<sf::Texture> Textures::m_mouseCrossHair;
std::unique_ptr<sf::Texture> Textures::m_levelSelectBackground;
std::unique_ptr<sf::Texture> Textures::m_enemyTerritoryHexSheet;
std::unique_ptr<sf::Texture> Textures::m_levelSelect1;
std::unique_ptr<sf::Texture> Textures::m_levelSelect2;
std::unique_ptr<sf::Texture> Textures::m_levelSelect3;
std::unique_ptr<sf::Texture> Textures::m_levelSelect4;
std::unique_ptr<sf::Texture> Textures::m_levelSelect5;
std::unique_ptr<sf::Texture> Textures::m_prebattleUIBackground;
std::unique_ptr<sf::Texture> Textures::m_preBattleUIPlayButton;
std::unique_ptr<sf::Texture> Textures::m_preBattleUIBackButton;
std::unique_ptr<sf::Texture> Textures::m_playerSelectIconYellow; 
std::unique_ptr<sf::Texture> Textures::m_playerSelectIconGreen;
std::unique_ptr<sf::Texture> Textures::m_playerSelectIconRed;
std::unique_ptr<sf::Texture> Textures::m_playerSelectIconBlue;
std::unique_ptr<sf::Texture> Textures::m_playerSelectBackground;
std::unique_ptr<sf::Texture> Textures::m_cross;
std::unique_ptr<sf::Texture> Textures::m_hexTiles;
std::unique_ptr<sf::Texture> Textures::m_afternoonHexTiles;
std::unique_ptr<sf::Texture> Textures::m_eveningHexTiles;
std::unique_ptr<sf::Texture> Textures::m_nightHexTiles;
std::unique_ptr<sf::Texture> Textures::m_spawnHex;
std::unique_ptr<sf::Texture> Textures::m_selectedHex;
std::unique_ptr<sf::Texture> Textures::m_battleIcons;
std::unique_ptr<sf::Texture> Textures::m_endPhaseButtons;
std::unique_ptr<sf::Texture> Textures::m_pauseButton;
std::unique_ptr<sf::Texture> Textures::m_chickenButton;
std::unique_ptr<sf::Texture> Textures::m_pauseMenuBackground;
std::unique_ptr<sf::Texture> Textures::m_resumeButton;
std::unique_ptr<sf::Texture> Textures::m_quitButton;
std::unique_ptr<sf::Texture> Textures::m_CompassBackGround;
std::unique_ptr<sf::Texture> Textures::m_CompassPointer;
std::unique_ptr<sf::Texture> Textures::m_WindArrow;
std::unique_ptr<sf::Texture> Textures::m_activeFactionToken;
std::unique_ptr<sf::Texture> Textures::m_postBattleBackground;
std::unique_ptr<sf::Texture> Textures::m_doneButton;


std::unique_ptr<sf::Texture> Textures::m_blueShipSideCannons; 
std::unique_ptr<sf::Texture> Textures::m_blueShipBomb;
std::unique_ptr<sf::Texture> Textures::m_blueShipMelee;
std::unique_ptr<sf::Texture> Textures::m_blueShipSnipe;
std::unique_ptr<sf::Texture> Textures::m_redShipSideCannons;
std::unique_ptr<sf::Texture> Textures::m_redShipBomb;
std::unique_ptr<sf::Texture> Textures::m_redShipMelee;
std::unique_ptr<sf::Texture> Textures::m_redShipSnipe;
std::unique_ptr<sf::Texture> Textures::m_greenShipSideCannons;
std::unique_ptr<sf::Texture> Textures::m_greenShipBomb;
std::unique_ptr<sf::Texture> Textures::m_greenShipMelee;
std::unique_ptr<sf::Texture> Textures::m_greenShipSnipe;
std::unique_ptr<sf::Texture> Textures::m_yellowShipSideCannons;
std::unique_ptr<sf::Texture> Textures::m_yellowShipBomb;
std::unique_ptr<sf::Texture> Textures::m_yellowShipMelee;
std::unique_ptr<sf::Texture> Textures::m_yellowShipSnipe;

std::unique_ptr<sf::Texture> Textures::m_background;
std::unique_ptr<sf::Texture> Textures::m_explosionParticles;
std::unique_ptr<sf::Texture> Textures::m_fireParticles;

std::unique_ptr<sf::Texture> Textures::m_redSpawnHex;
std::unique_ptr<sf::Texture> Textures::m_yellowSpawnHex;
std::unique_ptr<sf::Texture> Textures::m_greenSpawnHex;
std::unique_ptr<sf::Texture> Textures::m_blueSpawnHex;

std::unique_ptr<sf::Texture> Textures::loadTexture(const std::string & textureName)
{
	std::unique_ptr<sf::Texture> texture = std::make_unique<sf::Texture>();
	bool textureLoaded = texture->loadFromFile(DIRECTORY + textureName);
	assert(textureLoaded);
	
	return texture;
}