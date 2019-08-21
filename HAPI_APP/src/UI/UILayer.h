#pragma once

#include "../Utilities/NonCopyable.h"
#include "UIComponents.h"
#include "../Timer.h"
#include <SFML/Graphics.hpp>
#include <vector>

class UILayer : private NonCopyable
{
public:
	UILayer(eGameState ownedGameState);

	const UIComponentButton& getButton(eUIComponentName name) const;
	eGameState getOwnedGameState() const;

	void onComponentIntersect(sf::Vector2i mousePosition);
	void onComponentIntersect(sf::Vector2i mousePosition, UIComponentIntersectionDetails& intersectionDetails);

	void activateTimedVisibilityComponent(eUIComponentName componentName, eUIComponentType componentType);
	void setComponentVisibility(eUIComponentName componentName, eUIComponentType componentType, bool visible);
	void setComponentFrameID(eUIComponentName componentName, eUIComponentType componentType, int frameID);
	
	void setButtons(std::vector<UIComponentButton>&& buttons);
	void setTextBoxes(std::vector<UIComponentTextBox>&& textBoxes);
	void setImages(std::vector<UIComponentImage>&& images);

	void resetButtons();

	void update(float deltaTime);
	void render(sf::RenderWindow& window) const;

private:
	const eGameState m_ownedGameState;
	std::vector<UIComponentButton> m_buttons;
	std::vector<UIComponentTextBox> m_textBoxes;
	std::vector<UIComponentImage> m_images;

	UIComponentButton* m_timedVisibleButton;
	UIComponentTextBox* m_timedVisibleTextBox;
	Timer m_componentVisiblityTimer;
};