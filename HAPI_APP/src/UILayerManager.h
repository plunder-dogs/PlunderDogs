#pragma once

#include "Utilities/NonCopyable.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <memory>
#include "Global.h"

struct UIComponentBase
{
	UIComponentBase(sf::Vector2i position, sf::Vector2i size)
	{}

	sf::IntRect rect;
};

struct UIComponentTextBox : public UIComponentBase
{

};

struct UIComponentButton : public UIComponentBase
{

};

class UILayerManager : private NonCopyable
{
public:
	UILayerManager();

	void switchToLayer(eGameState gameState);

	void renderCurrentLayer(sf::RenderWindow& window);
	void updateCurrentLayer(sf::IntRect mouseRect);


private:
	std::array<std::vector<std::unique_ptr<UIComponentBase>>, static_cast<size_t>(eGameState::Total)> m_uiLayers;
	UIComponentBase* m_currentLayer;
	
};
