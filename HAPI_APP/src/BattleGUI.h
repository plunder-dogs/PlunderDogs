#pragma once

#include "Global.h"
#include <SFML/Graphics.hpp>

class BattleGUI
{
public:
	BattleGUI();

	sf::Vector2i getCameraPositionOffset() const;

	void update();
	//void OnMouseScroll could be added
	void setMaxCameraOffset(sf::Vector2i maxCameraOffset);
	void handleInput(sf::RenderWindow& window, const sf::Event& currentEvent);
	void onMouseMove(sf::Vector2i mousePosition);

private:
	sf::Vector2i m_maxCameraOffset;

	sf::Vector2f pendingCameraMovement;
	//VectorF pendingCameraMovement{ 0 };
	sf::Vector2i CameraPositionOffset;//camera offset that will be used by the map
	sf::Vector2i m_cameraPositionOffset;
};