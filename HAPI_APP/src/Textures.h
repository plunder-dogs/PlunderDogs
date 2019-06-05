#pragma once

#include "SFML/Graphics.hpp"
#include <memory>

struct Textures
{
	static void loadTexture(const std::string& textureName, std::unique_ptr<sf::Texture>& texture);
	static std::unique_ptr<sf::Texture> loadTexture(const std::string& textureName);
	static void loadAllTextures()
	{
		//fire_surface
		m_mouseCrossHair = loadTexture("crossHair.png");
		m_cross = loadTexture("cross.png");
		m_hexTiles = loadTexture("hexTiles_surface.png");
		m_selectedHex = loadTexture("spawnHex.png");
		m_blueShipSideCannons = loadTexture("shipBlueBomb_surface.png");
		//m_blueShipBomb

		loadTexture("cross.png", m_cross);
		loadTexture("hexTiles_surface.png", m_hexTiles);
		loadTexture("spawnHex.png", m_spawnHex);
		loadTexture("whiteHex75.png", m_selectedHex);
		loadTexture("shipBlueSide_surface.png", m_blueShipSideCannons);
		loadTexture("shipBlueBomb_surface.png", m_blueShipBomb);
		loadTexture("shipBlueMelee_surface.png", m_blueShipMelee);
		loadTexture("shipBlueSnipe_surface.png", m_blueShipSnipe);
		loadTexture("shipRedSide_surface.png", m_redShipSideCannons);

		//m_yellowShipMelee = HAPI_Wrapper::loadSpriteSheet("shipYellowMelee.xml");
		//m_redShipBomb = HAPI_Wrapper::loadSpriteSheet("shipRedBomb.xml");
		//m_redShipMelee = HAPI_Wrapper::loadSpriteSheet("shipRedMelee.xml");
		//m_redShipSnipe = HAPI_Wrapper::loadSpriteSheet("shipRedSnipe.xml");
			//m_greenShipSideCannons = HAPI_Wrapper::loadSpriteSheet("shipGreenSide.xml");
		//m_greenShipBomb = HAPI_Wrapper::loadSpriteSheet("shipGreenBomb.xml");
		//m_greenShipMelee = HAPI_Wrapper::loadSpriteSheet("shipGreenMelee.xml");
		//m_greenShipSnipe = HAPI_Wrapper::loadSpriteSheet("shipGreenSnipe.xml");
		//m_yellowShipSideCannons = HAPI_Wrapper::loadSpriteSheet("shipYellowSide.xml");
		//m_yellowShipBomb = HAPI_Wrapper::loadSpriteSheet("shipYellowBomb.xml");
		//m_yellowShipSnipe = HAPI_Wrapper::loadSpriteSheet("shipYellowSnipe.xml");
		//m_explosionParticles = HAPI_Wrapper::loadSpriteSheet("explosion.xml");
		//m_redSpawnHex = HAPI_Wrapper::makeSpriteSheet("spawnHexRed.png");
		//m_blueSpawnHex = HAPI_Wrapper::makeSpriteSheet("spawnHexBlue.png");
		//m_yellowSpawnHex = HAPI_Wrapper::makeSpriteSheet("spawnHexYellow.png");
		//m_greenSpawnHex = HAPI_Wrapper::makeSpriteSheet("spawnHexGreen.png");
	}

	static std::unique_ptr<sf::Texture> m_mouseCrossHair;
	static std::unique_ptr<sf::Texture> m_enemyTerritoryHexSheet;
	static std::unique_ptr<sf::Texture> m_levelSelectBackground;
	static std::unique_ptr<sf::Texture> m_levelSelect1;
	static std::unique_ptr<sf::Texture> m_levelSelect2;
	static std::unique_ptr<sf::Texture> m_levelSelect3;
	static std::unique_ptr<sf::Texture> m_levelSelect4;
	static std::unique_ptr<sf::Texture> m_levelSelect5;
	static std::unique_ptr<sf::Texture> m_playerSelectIconYellow;
	static std::unique_ptr<sf::Texture> m_playerSelectIconGreen;
	static std::unique_ptr<sf::Texture> m_playerSelectIconRed;
	static std::unique_ptr<sf::Texture> m_playerSelectIconBlue;
	static std::unique_ptr<sf::Texture> m_playerSelectBackground;
	static std::unique_ptr<sf::Texture> m_prebattleUIBackground;
	static std::unique_ptr<sf::Texture> m_preBattleUIPlayButton;
	static std::unique_ptr<sf::Texture> m_preBattleUIBackButton;
	static std::unique_ptr<sf::Texture> m_cross;
	static std::unique_ptr<sf::Texture> m_hexTiles;
	static std::unique_ptr<sf::Texture> m_afternoonHexTiles;
	static std::unique_ptr<sf::Texture> m_eveningHexTiles;
	static std::unique_ptr<sf::Texture> m_nightHexTiles;
	static std::unique_ptr<sf::Texture> m_spawnHex;
	static std::unique_ptr<sf::Texture> m_selectedHex;
	static std::unique_ptr<sf::Texture> m_battleIcons;
	static std::unique_ptr<sf::Texture> m_endPhaseButtons;
	static std::unique_ptr<sf::Texture> m_pauseButton;
	static std::unique_ptr<sf::Texture> m_chickenButton;
	static std::unique_ptr<sf::Texture> m_pauseMenuBackground;
	static std::unique_ptr<sf::Texture> m_resumeButton;
	static std::unique_ptr<sf::Texture> m_quitButton;
	static std::unique_ptr<sf::Texture> m_CompassBackGround;
	static std::unique_ptr<sf::Texture> m_CompassPointer;
	static std::unique_ptr<sf::Texture> m_WindArrow;
	static std::unique_ptr<sf::Texture> m_activeFactionToken;
	static std::unique_ptr<sf::Texture> m_postBattleBackground;
	static std::unique_ptr<sf::Texture> m_doneButton;
	static std::unique_ptr<sf::Texture> m_blueShipSideCannons;
	static std::unique_ptr<sf::Texture> m_blueShipBomb;
	static std::unique_ptr<sf::Texture> m_blueShipMelee;
	static std::unique_ptr<sf::Texture> m_blueShipSnipe;
	static std::unique_ptr<sf::Texture> m_redShipSideCannons;
	static std::unique_ptr<sf::Texture> m_redShipBomb;
	static std::unique_ptr<sf::Texture> m_redShipMelee;
	static std::unique_ptr<sf::Texture> m_redShipSnipe;
	static std::unique_ptr<sf::Texture> m_greenShipSideCannons;
	static std::unique_ptr<sf::Texture> m_greenShipBomb;
	static std::unique_ptr<sf::Texture> m_greenShipMelee;
	static std::unique_ptr<sf::Texture> m_greenShipSnipe;
	static std::unique_ptr<sf::Texture> m_yellowShipSideCannons;
	static std::unique_ptr<sf::Texture> m_yellowShipBomb;
	static std::unique_ptr<sf::Texture> m_yellowShipMelee;
	static std::unique_ptr<sf::Texture> m_yellowShipSnipe;


	static std::unique_ptr<sf::Texture> m_background;
	static std::unique_ptr<sf::Texture> m_explosionParticles;
	static std::unique_ptr<sf::Texture> m_fireParticles;

	static std::unique_ptr<sf::Texture> m_redSpawnHex;
	static std::unique_ptr<sf::Texture> m_blueSpawnHex;
	static std::unique_ptr<sf::Texture> m_yellowSpawnHex;
	static std::unique_ptr<sf::Texture> m_greenSpawnHex;
};