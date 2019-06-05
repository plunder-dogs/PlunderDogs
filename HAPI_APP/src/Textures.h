#pragma once

#include "SFML/Graphics.hpp"
#include <memory>

struct Textures
{
	static std::unique_ptr<sf::Texture> loadTexture(const std::string& textureName);
	static void loadAllTextures()
	{
		m_mouseCrossHair = loadTexture("crossHair.png");
		m_cross = loadTexture("cross.png");
		m_hexTiles = loadTexture("hexTiles_surface.png");
		m_spawnHex = loadTexture("spawnHex.png");
		m_blueShipSideCannons = loadTexture("shipBlueBomb_surface.png");
		m_selectedHex = loadTexture("whiteHex75.png");
		m_blueShipSideCannons = loadTexture("shipBlueSide_surface.png");
		m_blueShipBomb = loadTexture("shipBlueBomb_surface.png");
		m_blueShipMelee = loadTexture("shipBlueMelee_surface.png");
		m_blueShipSnipe = loadTexture("shipBlueSnipe_surface.png");
		m_redShipSideCannons = loadTexture("shipRedSide_surface.png");
		m_yellowShipMelee = loadTexture("shipYellowMelee_surface.png");
		m_redShipBomb = loadTexture("shipRedBomb_surface.png");
		m_redShipMelee = loadTexture("shipRedMelee_surface.png");
		m_redShipSnipe = loadTexture("shipRedSnipe_surface.png");
		m_greenShipSideCannons = loadTexture("shipGreenSide_surface.png");
		m_greenShipBomb = loadTexture("shipGreenBomb_surface.png");
		m_greenShipMelee = loadTexture("shipGreenMelee_surface.png");
		m_greenShipSnipe = loadTexture("shipGreenSnipe_surface.png");
		m_yellowShipSideCannons = loadTexture("shipYellowSide_surface.png");
		m_yellowShipBomb = loadTexture("shipYellowBomb_surface.png");
		m_yellowShipSnipe = loadTexture("shipYellowSnipe_surface.png");
		m_explosionParticles = loadTexture("explosion_surface.png");
		m_redSpawnHex = loadTexture("spawnHexRed.png");
		m_blueSpawnHex = loadTexture("spawnHexBlue.png");
		m_yellowSpawnHex = loadTexture("spawnHexYellow.png");
		m_greenSpawnHex = loadTexture("spawnHexGreen.png");
		m_CompassPointer = loadTexture("windDirectionArrow.png");
		m_fireParticles = loadTexture("fire_surface.png");
		m_nightHexTiles = loadTexture("nightHexTiles_surface.png");
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