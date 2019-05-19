#include "BattleGUI.h"
#include "Utilities/Utilities.h"
#include "Utilities/MapParser.h"
#include "GameEventMessenger.h"
#include "Battle.h"

BattleGUI::BattleGUI()
	:
	m_battleIcons(HAPI_Sprites.MakeSprite(Textures::m_battleIcons)),
	m_pauseButton(HAPI_Sprites.MakeSprite(Textures::m_pauseButton)),
	m_chickenButton(HAPI_Sprites.MakeSprite(Textures::m_chickenButton)),
	m_pauseMenuBackground(HAPI_Sprites.MakeSprite(Textures::m_pauseMenuBackground)),
	m_resumeButton(HAPI_Sprites.MakeSprite(Textures::m_resumeButton)),
	m_quitButton(HAPI_Sprites.MakeSprite(Textures::m_quitButton)),
	m_postBattleBackground(HAPI_Sprites.MakeSprite(Textures::m_postBattleBackground)),
	m_doneButton(HAPI_Sprites.MakeSprite(Textures::m_doneButton)),
	m_activeFactionToken(HAPI_Sprites.MakeSprite(Textures::m_activeFactionToken)),
	m_CompassPointer(HAPI_Sprites.MakeSprite(Textures::m_CompassPointer)),
	m_CompassBackGround(HAPI_Sprites.MakeSprite(Textures::m_CompassBackGround)),
	m_currentBattleWindow(BattleWindow::eCombat),
	m_maxCameraOffset(0, 0),
	m_endPhaseButtons(HAPI_Sprites.MakeSprite(Textures::m_endPhaseButtons)),
	m_AIInPlay(false)
{	
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleGUI::onBattleReset, this), "BattleGUI", GameEvent::eResetBattle);
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleGUI::onRedWin, this), "BattleGUI", GameEvent::eRedWin);
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleGUI::onYellowWin, this), "BattleGUI", GameEvent::eYellowWin);
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleGUI::onBlueWin, this), "BattleGUI", GameEvent::eBlueWin);
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleGUI::onGreenWin, this), "BattleGUI", GameEvent::eGreenWin);
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleGUI::onEnteringMovementPhase, this), "BattleGUI", GameEvent::eEnteringMovementPhase);
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleGUI::onEnteringAttackPhase, this), "BattleGUI", GameEvent::eEnteringAttackPhase);
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleGUI::onUnableToSkipPhase, this), "BattleGUI", GameEvent::eUnableToSkipPhase);
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleGUI::onEnteredAITurn, this), "BattleGUI", GameEvent::eEnteredAITurn);
	GameEventMessenger::getInstance().subscribe(std::bind(&BattleGUI::onLeftAITurn, this), "BattleGUI", GameEvent::eLeftAITurn);

	m_battleIcons->GetTransformComp().SetPosition({ 510, 890 });
	m_endPhaseButtons->GetTransformComp().SetPosition({ 40, 900 });
	m_pauseButton->GetTransformComp().SetPosition({ 1790, 30 });
	m_chickenButton->GetTransformComp().SetPosition({ 1610, 840 });
	m_CompassBackGround->GetTransformComp().SetOriginToCentreOfFrame();
	m_CompassBackGround->GetTransformComp().SetPosition({ 100, 100 });
	m_CompassPointer->GetTransformComp().SetOrigin({ 21.5f,60 });
	m_CompassPointer->GetTransformComp().SetPosition({ 100, 100 });
	m_activeFactionToken->GetTransformComp().SetOriginToCentreOfFrame();
	m_activeFactionToken->GetTransformComp().SetPosition({ 960, 55 });
	
	//pauseMenu
	m_resumeButton->GetTransformComp().SetPosition({ 818, 387 });
	m_quitButton->GetTransformComp().SetPosition({ 818, 517 });
	//postBattle
	//m_postBattleBackground->GetTransformComp().SetPosition({ 360, 190 });
	m_doneButton->GetTransformComp().SetPosition({ 820, 800 });
}

BattleGUI::~BattleGUI()
{
	GameEventMessenger::getInstance().unsubscribe("BattleGUI", GameEvent::eResetBattle);
	GameEventMessenger::getInstance().unsubscribe("BattleGUI", GameEvent::eRedWin);
	GameEventMessenger::getInstance().unsubscribe("BattleGUI", GameEvent::eYellowWin);
	GameEventMessenger::getInstance().unsubscribe("BattleGUI", GameEvent::eGreenWin);
	GameEventMessenger::getInstance().unsubscribe("BattleGUI", GameEvent::eBlueWin);
	GameEventMessenger::getInstance().unsubscribe("BattleGUI", GameEvent::eEnteringMovementPhase);
	GameEventMessenger::getInstance().unsubscribe("BattleGUI", GameEvent::eEnteringAttackPhase);
	GameEventMessenger::getInstance().unsubscribe("BattleGUI", GameEvent::eUnableToSkipPhase);
	GameEventMessenger::getInstance().unsubscribe("BattleGUI", GameEvent::eEnteredAITurn);
	GameEventMessenger::getInstance().unsubscribe("BattleGUI", GameEvent::eLeftAITurn);
}

std::pair<int, int> BattleGUI::getCameraPositionOffset() const
{
	return m_cameraPositionOffset;
}

void BattleGUI::render(BattlePhase currentBattlePhase) const
{
	m_pauseButton->Render(SCREEN_SURFACE);
	//m_chickenButton->Render(SCREEN_SURFACE);
	m_CompassBackGround->Render(SCREEN_SURFACE);
	m_CompassPointer->Render(SCREEN_SURFACE);
	SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(50, 150), HAPISPACE::Colour255::CYAN, "Wind", 50, {}, HAPISPACE::Colour255::BLACK, 2.5f);

	//m_activeFactionToken->Render(SCREEN_SURFACE);

	if (currentBattlePhase != BattlePhase::Deployment)
	{
		if (m_activeFactionToken->GetFrameNumber() == FactionName::eRed)
		{
			SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(800, 50), HAPISPACE::Colour255::RED, "Red Team", 50, {}, HAPISPACE::Colour255::BLACK, 2.f);
		}
		else if (m_activeFactionToken->GetFrameNumber() == FactionName::eGreen)
		{
			SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(800, 50), HAPISPACE::Colour255::GREEN, "Green Team", 50, {}, HAPISPACE::Colour255::BLACK, 2.f);
		}
		else if (m_activeFactionToken->GetFrameNumber() == FactionName::eBlue)
		{
			SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(800, 50), HAPISPACE::Colour255::BLUE, "Blue Team", 50, {}, HAPISPACE::Colour255::BLACK, 2.f);
		}
		else if (m_activeFactionToken->GetFrameNumber() == FactionName::eYellow)
		{
			SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(800, 50), HAPISPACE::Colour255::YELLOW, "Yellow Team", 50, {}, HAPISPACE::Colour255::BLACK, 2.f);
		}
	}
	
	if (currentBattlePhase != BattlePhase::Deployment && !m_AIInPlay)
	{
		m_endPhaseButtons->Render(SCREEN_SURFACE);
	}


	//m_activeFactionToken->Render(SCREEN_SURFACE);
	//m_endPhaseButtons->Render(SCREEN_SURFACE);
	//SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(800, 585), HAPISPACE::Colour255::RED, std::to_string(m_maxCameraOffset.first), 50);//Dont delete these until the panning has been fixed - Jack
	//SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(950, 585), HAPISPACE::Colour255::RED, std::to_string(CameraPositionOffset.first), 50);
	//SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(800, 685), HAPISPACE::Colour255::GREEN, std::to_string(m_maxCameraOffset.second), 50);
	//SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(950, 685), HAPISPACE::Colour255::GREEN, std::to_string(CameraPositionOffset.second), 50);

	switch (m_currentBattleWindow)
	{
	case BattleWindow::ePause:
	{
		m_pauseMenuBackground->Render(SCREEN_SURFACE);
		m_resumeButton->Render(SCREEN_SURFACE);
		m_quitButton->Render(SCREEN_SURFACE);
		break;
	}
	case BattleWindow::ePostBattle:
	{
		
		m_postBattleBackground->Render(SCREEN_SURFACE);
		m_doneButton->Render(SCREEN_SURFACE);
		if (winningFaction == "Red")
		{
			SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(450, 400), HAPISPACE::Colour255::RED, winningFaction + " Faction Wins ", 190, {}, HAPISPACE::Colour255::BLACK, 2.5f);
		}
		else if (winningFaction == "Green")
		{
			SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(450, 400), HAPISPACE::Colour255::GREEN, winningFaction + " Faction Wins ", 190, {}, HAPISPACE::Colour255::BLACK, 2.5f);
		}
		else if (winningFaction == "Yellow")
		{
			SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(450, 400), HAPISPACE::Colour255::YELLOW, winningFaction + " Faction Wins ", 190, {}, HAPISPACE::Colour255::BLACK, 2.5f);
		}
		else if (winningFaction == "Blue")
		{
			SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(450, 400), HAPISPACE::Colour255::BLUE, winningFaction + " Faction Wins ", 190, {}, HAPISPACE::Colour255::BLACK, 2.5f);
		}
		//SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(450, 400), HAPISPACE::Colour255::YELLOW, winningFaction + " Faction Wins ", 190, {}, HAPISPACE::Colour255::BLACK, 2.5f);
	}

	}
}

void BattleGUI::renderStats(EntityProperties & entityProperties) const
{
	m_battleIcons->GetTransformComp().SetPosition({ 510, (800 + static_cast<float>(animationOffset)) });
	m_battleIcons->Render(SCREEN_SURFACE);

	SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(600, (815 + animationOffset)), HAPISPACE::Colour255::BLACK, "  "+std::to_string(entityProperties.m_currentHealth) , 44);
	SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(860, (815 + animationOffset)), HAPISPACE::Colour255::BLACK, std::to_string(entityProperties.m_range), 44);
	SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(1060, (815 + animationOffset)), HAPISPACE::Colour255::BLACK, std::to_string(entityProperties.m_movementPoints), 44);
	SCREEN_SURFACE->DrawText(HAPISPACE::VectorI(1295, (815 + animationOffset)), HAPISPACE::Colour255::BLACK, std::to_string(entityProperties.m_damage), 44);

}

void BattleGUI::update(eDirection windDirection)
{
	m_CompassPointer->GetTransformComp().SetRotation(DEGREES_TO_RADIANS((static_cast<int>(windDirection) *60) % 360));
	
	if (playAnimation)
	{
		animationOffset = 280 - (HAPI_Sprites.GetTime() - animationStartTime);
		if (animationOffset < 1)
		{
			playAnimation = false;
			animationOffset = 0;
		}

		m_battleIcons->GetTransformComp().SetPosition({ 510, (890 + static_cast<float>(animationOffset)) });
	}
	

	switch (m_currentBattleWindow)
	{
	case BattleWindow::eCombat:
	{
		//camera pan
		if (!pendingCameraMovement.IsZero())
		{			
			CameraPositionOffset.first += pendingCameraMovement.x;//translates the camera position
			CameraPositionOffset.second += pendingCameraMovement.y;

			if (CameraPositionOffset.first < -120)//checks for if its reached any of the 4 boundries, need to change it to a width variable
			{
				CameraPositionOffset.first = -120;
			}
			else if (CameraPositionOffset.first > m_maxCameraOffset.first)
			{
				CameraPositionOffset.first = m_maxCameraOffset.first;
			}
			else
			{
				CameraPositionOffset.first += pendingCameraMovement.x;
			}

			if (CameraPositionOffset.second < -100)
			{
				CameraPositionOffset.second = -100;
			}
			else if (CameraPositionOffset.second > m_maxCameraOffset.second)
			{
				CameraPositionOffset.second = m_maxCameraOffset.second;
			}
			else
			{
				CameraPositionOffset.second += pendingCameraMovement.y;
			}

			m_cameraPositionOffset = CameraPositionOffset;
			//m_battle.setMapDrawOffset(CameraPositionOffset);//TODO: CREATE A FUNCTION FOR THIS IN BATTLE THAT'S CALLED INSTEAD
		}
		break;//the battle should continue to render behind the pause menu so is before the switch case
	}
	}
}

void BattleGUI::updateFactionToken(int factionName)
{
	m_activeFactionToken->SetFrameNumber(static_cast<int>(factionName));
}

void BattleGUI::OnMouseLeftClick(const HAPI_TMouseData& mouseData, BattlePhase currentBattlePhase)
{
	
	//snapCameraToPosition(std::pair<int, int>{ 15, 15 });

	switch (m_currentBattleWindow)
	{
	case BattleWindow::eCombat:
	{
		if (m_pauseButton->GetSpritesheet()->GetFrameRect(0).Translated(
			m_pauseButton->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y)))//if you press the pause button
		{
			m_currentBattleWindow = BattleWindow::ePause;//enables the pause menu
		}

		if (m_chickenButton->GetSpritesheet()->GetFrameRect(0).Translated(
			m_chickenButton->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y)))//if you press the pause button
		{
			animationStartTime = HAPI_Sprites.GetTime();
			shipSelected = true;
			playAnimation = true;
		}
		else
		{
			shipSelected = false;
		}

		// ryan phase button stuff is here
		if (!m_AIInPlay && m_endPhaseButtons->GetSpritesheet()->GetFrameRect(0).Translated(
			m_endPhaseButtons->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y)))
		{
			if (currentBattlePhase == BattlePhase::Movement)
			{
				GameEventMessenger::broadcast(GameEvent::eEndMovementPhaseEarly);
				//end movement phase
			}
			else if (currentBattlePhase == BattlePhase::Attack)
			{
				GameEventMessenger::broadcast(GameEvent::eEndAttackPhaseEarly);
				//end attack phase	
			}
		}
		break;
	}
	case BattleWindow::ePause:
	{
		if (m_resumeButton->GetSpritesheet()->GetFrameRect(0).Translated(
			m_resumeButton->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y)))//if you press the resume button
		{
			m_currentBattleWindow = BattleWindow::eCombat;//disables the pause menu
		}
		else if (m_quitButton->GetSpritesheet()->GetFrameRect(0).Translated(
			m_quitButton->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y)))//if you press the resume button
		{

			GameEventMessenger::getInstance().broadcast(GameEvent::eResetBattle);
			//m_currentBattleWindow = BattleWindow::ePostBattle;//disables the pause menu
		}
		break;
	}
	case BattleWindow::ePostBattle:
	{
		if (m_doneButton->GetSpritesheet()->GetFrameRect(0).Translated(m_doneButton->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y)))//if you press the pause button
		{
			GameEventMessenger::getInstance().broadcast(GameEvent::eResetBattle);
		}
		break;
	}
	}
}

void BattleGUI::OnMouseMove(const HAPI_TMouseData& mouseData, BattlePhase currentBattlePhase)
{
	switch (m_currentBattleWindow)
	{
	case BattleWindow::eCombat:
	{
		if (m_pauseButton->GetSpritesheet()->GetFrameRect(0).Translated(m_pauseButton->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y)))//checks if mouse is over button
		{
			m_pauseButton->SetFrameNumber(1);//changes the buttons sprite to hover sprite
		}
		else if (m_pauseButton->GetFrameNumber() != 0)//if mouse is not over the button and the button has the hover sprite
		{
			m_pauseButton->SetFrameNumber(0);// sets it to the default sprite
		}

		// ryan the phase button stuff is here
		if (m_endPhaseButtons->GetSpritesheet()->GetFrameRect(0).Translated(m_endPhaseButtons->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y)))//checks if mouse is over the phase button
		{
			if (currentBattlePhase == BattlePhase::Movement)
			{
				m_endPhaseButtons->SetFrameNumber(1);
				
			}
			else if (currentBattlePhase == BattlePhase::Attack)
			{
				
				m_endPhaseButtons->SetFrameNumber(3);
			}
		}
		else if (m_endPhaseButtons->GetFrameNumber() != 0 && m_endPhaseButtons->GetFrameNumber() != 2)//checks if the mouse is no longer over the phase button
		{
			if (currentBattlePhase == BattlePhase::Movement)
			{
				m_endPhaseButtons->SetFrameNumber(0);
			}
			else if (currentBattlePhase == BattlePhase::Attack)
			{
				m_endPhaseButtons->SetFrameNumber(2);
			}
		}


		//moves the sprites when the mouse is on the edge of the screen
		//only checks when mouse moves. if mouse doesnt move, it knows its still in the same spot and will keep scrolling without checking
		pendingCameraMovement = VectorF{ 0,0 };

		if (mouseData.x < 100)
		{
			pendingCameraMovement += VectorF{ -1,0 };
		}
		else if (mouseData.x > 1820)
		{
			pendingCameraMovement += VectorF{ 1,0 };
		}


		//if (mouseData.y < 100)

		
		if (mouseData.y < 50)
		{
			pendingCameraMovement += VectorF{ 0,-1 };
		}
		else if (mouseData.y > 980)
		{
			pendingCameraMovement += VectorF{ 0,1 };
		}
		break;
	}
	case BattleWindow::ePause:
	{
		if (m_resumeButton->GetSpritesheet()->GetFrameRect(0).Translated(m_resumeButton->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y)))
		{
			m_resumeButton->SetFrameNumber(1);
		}
		else if (m_resumeButton->GetFrameNumber() != 0)
		{
			m_resumeButton->SetFrameNumber(0);
		}

		if (m_quitButton->GetSpritesheet()->GetFrameRect(0).Translated(m_quitButton->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y)))
		{
			m_quitButton->SetFrameNumber(1);
		}
		else if (m_quitButton->GetFrameNumber() != 0)
		{
			m_quitButton->SetFrameNumber(0);
		}
		break;
	}
	case BattleWindow::ePostBattle:
	{
		if (m_doneButton->GetSpritesheet()->GetFrameRect(0).Translated(m_doneButton->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y)))
		{
			m_doneButton->SetFrameNumber(1);
		}
		else if (m_doneButton->GetFrameNumber() != 0)
		{
			m_doneButton->SetFrameNumber(0);
		}
		break;
	}
	}
}

void BattleGUI::setMaxCameraOffset(std::pair<int, int> maxCameraOffset)
{
	m_maxCameraOffset = std::pair<int, int>(maxCameraOffset.first * 24 - 820, maxCameraOffset.second * 28 - 400);
	if (m_maxCameraOffset.first < 0)
	{
		m_maxCameraOffset.first = 0;
	}
	if (m_maxCameraOffset.second < 0)
	{
		m_maxCameraOffset.second = 0;
	}
}

void BattleGUI::onBattleReset()
{
	m_currentBattleWindow = BattleWindow::eCombat;
	winningFaction = "";
	m_AIInPlay = false;

	m_battleIcons->GetTransformComp().SetPosition({ 510, 890 });
	m_endPhaseButtons->GetTransformComp().SetPosition({ 40, 900 });
	m_pauseButton->GetTransformComp().SetPosition({ 1790, 30 });
	//m_chickenButton->GetTransformComp().SetPosition({ 1610, 840 });
	m_CompassBackGround->GetTransformComp().SetOriginToCentreOfFrame();
	m_CompassBackGround->GetTransformComp().SetPosition({ 100, 100 });
	m_CompassPointer->GetTransformComp().SetOrigin({ 21.5f,60 });
	m_CompassPointer->GetTransformComp().SetPosition({ 100, 100 });
	m_activeFactionToken->GetTransformComp().SetOriginToCentreOfFrame();
	m_activeFactionToken->GetTransformComp().SetPosition({ 960,55 });// position just temp can be adjusted as needed postiiton 900/100

	//pauseMenu
	m_resumeButton->GetTransformComp().SetPosition({ 818, 387 });
	m_quitButton->GetTransformComp().SetPosition({ 818, 517 });
	//postBattle
	m_doneButton->GetTransformComp().SetPosition({ 820, 800 });
}


std::string BattleGUI::getWinningFactionName()
{
	std::string factionString{ " " };
	if (m_activeFactionToken->GetFrameNumber() == FactionName::eRed)
	{
		factionString = "Red";
	}
	if (m_activeFactionToken->GetFrameNumber() == FactionName::eGreen)
	{
		factionString = "Green";
	}
	if (m_activeFactionToken->GetFrameNumber() == FactionName::eBlue)
	{
		factionString = "Blue";
	}
	if (m_activeFactionToken->GetFrameNumber() == FactionName::eYellow)
	{
		factionString = "Yellow";
	}
	return factionString;
}

void BattleGUI::onBlueWin()
{
	winningFaction = getWinningFactionName();
	m_currentBattleWindow = BattleWindow::ePostBattle;
}

void BattleGUI::onGreenWin()
{
	winningFaction = getWinningFactionName();
	m_currentBattleWindow = BattleWindow::ePostBattle;
}

void BattleGUI::onYellowWin()
{
	winningFaction = getWinningFactionName();
	m_currentBattleWindow = BattleWindow::ePostBattle;
}

void BattleGUI::onRedWin()
{
	winningFaction = getWinningFactionName();
	m_currentBattleWindow = BattleWindow::ePostBattle;
}

void BattleGUI::onEnteringMovementPhase()
{
	m_endPhaseButtons->SetFrameNumber(0);
}

void BattleGUI::onEnteringAttackPhase()
{
	m_endPhaseButtons->SetFrameNumber(2);
}

void BattleGUI::onUnableToSkipPhase()
{
}

void BattleGUI::onEnteredAITurn()
{
	m_AIInPlay = true;
}

void BattleGUI::onLeftAITurn()
{
	m_AIInPlay = false;
}

void BattleGUI::snapCameraToPosition(std::pair<int, int> snapLocation)//snaps the camera to be centered on a given tile
{
	int xPos = snapLocation.first * 24 - 480;
	int yPos = snapLocation.second  * 28 - 270;
	//Ensure values are within bounds
	xPos = std::max(-120, xPos);
	xPos = std::min(m_maxCameraOffset.first, xPos);
	yPos = std::max(-100, yPos);
	yPos = std::min(m_maxCameraOffset.second, yPos);

	m_cameraPositionOffset.first = xPos;
	m_cameraPositionOffset.second = yPos;
	CameraPositionOffset.first = xPos;
	CameraPositionOffset.second = yPos;
}

bool BattleGUI::isPaused()
{
	if (m_currentBattleWindow == ePause)
		return true;
	return false;
}
