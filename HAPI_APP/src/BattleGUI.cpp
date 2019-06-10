#include "BattleGUI.h"
#include "Utilities/Utilities.h"
#include "Utilities/XMLParser.h"
#include "GameEventMessenger.h"
#include "Battle.h"

BattleGUI::BattleGUI()
	: m_maxCameraOffset(0, 0)
{	
}

sf::Vector2i BattleGUI::getCameraPositionOffset() const
{
	return m_cameraPositionOffset;
}

void BattleGUI::update()
{
	//camera pan
	if (pendingCameraMovement != sf::Vector2f(0, 0))
	{
		CameraPositionOffset.x += pendingCameraMovement.x;//translates the camera position
		CameraPositionOffset.y += pendingCameraMovement.y;

		if (CameraPositionOffset.x < -120)//checks for if its reached any of the 4 boundries, need to change it to a width variable
		{
			CameraPositionOffset.x = -120;
		}
		else if (CameraPositionOffset.x > m_maxCameraOffset.x)
		{
			CameraPositionOffset.x = m_maxCameraOffset.x;
		}
		else
		{
			CameraPositionOffset.x += pendingCameraMovement.x;
		}

		if (CameraPositionOffset.y < -100)
		{
			CameraPositionOffset.y = -100;
		}
		else if (CameraPositionOffset.y > m_maxCameraOffset.y)
		{
			CameraPositionOffset.y = m_maxCameraOffset.y;
		}
		else
		{
			CameraPositionOffset.y += pendingCameraMovement.y;
		}

		m_cameraPositionOffset = CameraPositionOffset;
		//m_battle.setMapDrawOffset(CameraPositionOffset);//TODO: CREATE A FUNCTION FOR THIS IN BATTLE THAT'S CALLED INSTEAD
	}	
}

void BattleGUI::setMaxCameraOffset(sf::Vector2i maxCameraOffset)
{
	m_maxCameraOffset = sf::Vector2i(maxCameraOffset.x * 24 - 820, maxCameraOffset.y * 28 - 400);
	if (m_maxCameraOffset.x < 0)
	{
		m_maxCameraOffset.x = 0;
	}
	if (m_maxCameraOffset.y < 0)
	{
		m_maxCameraOffset.y = 0;
	}
}

void BattleGUI::handleInput(sf::RenderWindow & window, const sf::Event & currentEvent)
{
}

void BattleGUI::onMouseMove(sf::Vector2i mousePosition)
{
	//moves the sprites when the mouse is on the edge of the screen
	//only checks when mouse moves. if mouse doesnt move, it knows its still in the same spot and will keep scrolling without checking
	pendingCameraMovement = sf::Vector2f(0, 0);

	if (mousePosition.x < 100)
	{
		pendingCameraMovement += sf::Vector2f{ -1,0 };
	}
	else if (mousePosition.x > 1820)
	{
		pendingCameraMovement += sf::Vector2f{ 1,0 };
	}

	if (mousePosition.y < 50)
	{
		pendingCameraMovement += sf::Vector2f{ 0 , -1 };
	}
	else if (mousePosition.y > 980)
	{
		pendingCameraMovement += sf::Vector2f{ 0, 1 };
	}	
}
