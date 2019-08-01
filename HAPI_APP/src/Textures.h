#pragma once

#include <memory>
#include "Texture.h"
class Textures
{
public:
	static Textures& getInstance()
	{
		static Textures textures;
		return textures;
	}

	void loadAllTextures();	

	std::unique_ptr<Texture> m_mouseCrossHair;
	std::unique_ptr<Texture> m_enemyTerritoryHexSheet;
	std::unique_ptr<Texture> m_levelSelectBackground;
	std::unique_ptr<Texture> m_levelSelect1;
	std::unique_ptr<Texture> m_levelSelect2;
	std::unique_ptr<Texture> m_levelSelect3;
	std::unique_ptr<Texture> m_levelSelect4;
	std::unique_ptr<Texture> m_levelSelect5;
	std::unique_ptr<Texture> m_playerSelectIconYellow;
	std::unique_ptr<Texture> m_playerSelectIconGreen;
	std::unique_ptr<Texture> m_playerSelectIconRed;
	std::unique_ptr<Texture> m_playerSelectIconBlue;
	std::unique_ptr<Texture> m_playerSelectBackground;
	std::unique_ptr<Texture> m_prebattleUIBackground;
	std::unique_ptr<Texture> m_preBattleUIPlayButton;
	std::unique_ptr<Texture> m_preBattleUIBackButton;
	std::unique_ptr<Texture> m_cross;
	std::unique_ptr<Texture> m_hexTiles;
	std::unique_ptr<Texture> m_afternoonHexTiles;
	std::unique_ptr<Texture> m_eveningHexTiles;
	std::unique_ptr<Texture> m_nightHexTiles;
	std::unique_ptr<Texture> m_spawnHex;
	std::unique_ptr<Texture> m_selectedHex;
	std::unique_ptr<Texture> m_battleIcons;
	std::unique_ptr<Texture> m_endPhaseButtons;
	std::unique_ptr<Texture> m_pauseButton;
	std::unique_ptr<Texture> m_chickenButton;
	std::unique_ptr<Texture> m_pauseMenuBackground;
	std::unique_ptr<Texture> m_resumeButton;
	std::unique_ptr<Texture> m_quitButton;
	std::unique_ptr<Texture> m_CompassBackGround;
	std::unique_ptr<Texture> m_CompassPointer;
	std::unique_ptr<Texture> m_WindArrow;
	std::unique_ptr<Texture> m_activeFactionToken;
	std::unique_ptr<Texture> m_postBattleBackground;
	std::unique_ptr<Texture> m_doneButton;
	std::unique_ptr<Texture> m_blueShipSideCannons;
	std::unique_ptr<Texture> m_blueShipBomb;
	std::unique_ptr<Texture> m_blueShipMelee;
	std::unique_ptr<Texture> m_blueShipSnipe;
	std::unique_ptr<Texture> m_redShipSideCannons;
	std::unique_ptr<Texture> m_redShipBomb;
	std::unique_ptr<Texture> m_redShipMelee;
	std::unique_ptr<Texture> m_redShipSnipe;
	std::unique_ptr<Texture> m_greenShipSideCannons;
	std::unique_ptr<Texture> m_greenShipBomb;
	std::unique_ptr<Texture> m_greenShipMelee;
	std::unique_ptr<Texture> m_greenShipSnipe;
	std::unique_ptr<Texture> m_yellowShipSideCannons;
	std::unique_ptr<Texture> m_yellowShipBomb;
	std::unique_ptr<Texture> m_yellowShipMelee;
	std::unique_ptr<Texture> m_yellowShipSnipe;


	std::unique_ptr<Texture> m_background;
	std::unique_ptr<Texture> m_explosionParticles;
	std::unique_ptr<Texture> m_fireParticles;

	std::unique_ptr<Texture> m_redSpawnHex;
	std::unique_ptr<Texture> m_blueSpawnHex;
	std::unique_ptr<Texture> m_yellowSpawnHex;
	std::unique_ptr<Texture> m_greenSpawnHex;

private:
	bool m_allTexturesLoaded = false;
};