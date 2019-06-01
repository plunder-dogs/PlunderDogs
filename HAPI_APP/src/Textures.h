#pragma once

#include "SFML/Graphics.hpp"
#include <memory>

struct Textures
{
	static void loadTexture(const std::string& textureName, std::unique_ptr<sf::Texture>& texture);

	static void loadAllTextures()
	{
		//fire_surface
		loadTexture("crossHair.png", m_mouseCrossHair);	
		loadTexture("cross.png", m_cross);
		loadTexture("hexTiles_surface.png", m_hexTiles);
		loadTexture("spawnHex.png", m_spawnHex);
		loadTexture("whiteHex75.png", m_selectedHex);

		//blueShips
		m_blueShipSideCannons = HAPI_Wrapper::loadSpriteSheet("shipBlueSide.xml");
		if (!m_blueShipSideCannons)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipBlueSide.xml'", "Texture Error");
			return false;
		}
		m_blueShipBomb = HAPI_Wrapper::loadSpriteSheet("shipBlueBomb.xml");
		if (!m_blueShipBomb)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipBlueBomb.xml'", "Texture Error");
			return false;
		}
		m_blueShipMelee = HAPI_Wrapper::loadSpriteSheet("shipBlueMelee.xml");
		if (!m_blueShipMelee)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipBlueMelee.xml'", "Texture Error");
			return false;
		}
		m_blueShipSnipe = HAPI_Wrapper::loadSpriteSheet("shipBlueSnipe.xml");
		if (!m_blueShipSnipe)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipBlueSnipe.xml'", "Texture Error");
			return false;
		}

		//RedShips
		m_redShipSideCannons = HAPI_Wrapper::loadSpriteSheet("shipRedSide.xml");
		if (!m_redShipSideCannons)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipRedSide.xml'", "Texture Error");
			return false;
		}
		m_redShipBomb = HAPI_Wrapper::loadSpriteSheet("shipRedBomb.xml");
		if (!m_redShipBomb)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipRedBomb.xml'", "Texture Error");
			return false;
		}
		m_redShipMelee = HAPI_Wrapper::loadSpriteSheet("shipRedMelee.xml");
		if (!m_redShipMelee)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipRedMelee.xml'", "Texture Error");
			return false;
		}
		m_redShipSnipe = HAPI_Wrapper::loadSpriteSheet("shipRedSnipe.xml");
		if (!m_redShipSnipe)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipRedSnipe.xml'", "Texture Error");
			return false;
		}

		//GreenShips
		m_greenShipSideCannons = HAPI_Wrapper::loadSpriteSheet("shipGreenSide.xml");
		if (!m_greenShipSideCannons)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipGreenSide.xml'", "Texture Error");
			return false;
		}
		m_greenShipBomb = HAPI_Wrapper::loadSpriteSheet("shipGreenBomb.xml");
		if (!m_greenShipBomb)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipGreenBomb.xml'", "Texture Error");
			return false;
		}
		m_greenShipMelee = HAPI_Wrapper::loadSpriteSheet("shipGreenMelee.xml");
		if (!m_greenShipMelee)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipGreenMelee.xml'", "Texture Error");
			return false;
		}
		m_greenShipSnipe = HAPI_Wrapper::loadSpriteSheet("shipGreenSnipe.xml");
		if (!m_greenShipSnipe)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipGreenSnipe.xml'", "Texture Error");
			return false;
		}

		//YellowShips
		m_yellowShipSideCannons = HAPI_Wrapper::loadSpriteSheet("shipYellowSide.xml");
		if (!m_yellowShipSideCannons)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipYellowSide.xml'", "Texture Error");
			return false;
		}
		m_yellowShipBomb = HAPI_Wrapper::loadSpriteSheet("shipYellowBomb.xml");
		if (!m_yellowShipBomb)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipYellowBomb.xml'", "Texture Error");
			return false;
		}
		m_yellowShipMelee = HAPI_Wrapper::loadSpriteSheet("shipYellowMelee.xml");
		if (!m_yellowShipMelee)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipYellowMelee.xml'", "Texture Error");
			return false;
		}
		m_yellowShipSnipe = HAPI_Wrapper::loadSpriteSheet("shipYellowSnipe.xml");
		if (!m_yellowShipSnipe)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'shipYellowSnipe.xml'", "Texture Error");
			return false;
		}

		m_explosionParticles = HAPI_Wrapper::loadSpriteSheet("explosion.xml");
		if (!m_explosionParticles)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'explosion.xml'", "Texture Error");
			return false;
		}


		
		m_redSpawnHex = HAPI_Wrapper::makeSpriteSheet("spawnHexRed.png");
		if (!m_redSpawnHex)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'spawnHexRed.png'", "Texture Error");
			return false;
		}

		m_blueSpawnHex = HAPI_Wrapper::makeSpriteSheet("spawnHexBlue.png");
		if (!m_blueSpawnHex)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'spawnHexBlue.png'", "Texture Error");
			return false;
		}

		m_yellowSpawnHex = HAPI_Wrapper::makeSpriteSheet("spawnHexYellow.png");
		if (!m_yellowSpawnHex)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'spawnHexYellow.png'", "Texture Error");
			return false;
		}

		m_greenSpawnHex = HAPI_Wrapper::makeSpriteSheet("spawnHexGreen.png");
		if (!m_greenSpawnHex)
		{
			HAPI_Sprites.UserMessage("Connot Load: 'spawnHexGreen.png'", "Texture Error");
			return false;
		}

		return true;
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