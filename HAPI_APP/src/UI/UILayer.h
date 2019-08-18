#pragma once

#include "../Utilities/NonCopyable.h"
#include "UIComponents.h"
#include "../Timer.h"
#include <SFML/Graphics.hpp>
#include <vector>

class UILayer : private NonCopyable
{
public:
	UILayer();

	const UIComponentButton& getButton(eUIComponentName name) const;

	void onComponentIntersect(sf::IntRect mouseRect);
	void onComponentIntersect(sf::IntRect mouseRect, UIComponentIntersectionDetails& intersectionDetails);

	void activateTimedVisibilityComponent(eUIComponentName componentName, eUIComponentType componentType);
	
	void setButtons(std::vector<UIComponentButton>&& buttons);
	void setTextBoxes(std::vector<UIComponentTextBox>&& textBoxes);
	void setImages(std::vector<Sprite>&& images);

	void resetButtons();

	void update(float deltaTime);
	void render(sf::RenderWindow& window) const;

private:
	std::vector<UIComponentButton> m_buttons;
	std::vector<UIComponentTextBox> m_textBoxes;
	std::vector<Sprite> m_images;

	UIComponentButton* m_timedVisibleButton;
	UIComponentTextBox* m_timedVisibleTextBox;
	Timer m_componentVisiblityTimer;
};