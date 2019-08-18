#pragma once

#include "../Utilities/NonCopyable.h"
#include "UIComponents.h"
#include <SFML/Graphics.hpp>
#include <vector>

class UILayer : private NonCopyable
{
public:
	UILayer();

	void onComponentIntersect(sf::IntRect mouseRect);
	void onComponentIntersect(sf::IntRect mouseRect, UIComponentIntersectionDetails& intersectionDetails);
	
	void setButtons(std::vector<UIComponentButton>&& buttons);
	void setTextBoxes(std::vector<UIComponentTextBox>&& textBoxes);
	void setImages(std::vector<Sprite>&& images);


	void resetButtonsFrameID();

	void render(sf::RenderWindow& window);

private:
	std::vector<UIComponentButton> m_buttons;
	std::vector<UIComponentTextBox> m_textBoxes;
	std::vector<Sprite> m_images;
};