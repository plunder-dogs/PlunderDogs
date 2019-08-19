#include "UILayer.h"
#include <assert.h>
#include <algorithm>

constexpr float COMPONENT_VISIBILITY_DURATION = 2.0f;

UILayer::UILayer()
	: m_buttons(),
	m_textBoxes(),
	m_images(),
	m_timedVisibleButton(nullptr),
	m_timedVisibleTextBox(nullptr),
	m_componentVisiblityTimer(COMPONENT_VISIBILITY_DURATION, false)
{}

const UIComponentButton & UILayer::getButton(eUIComponentName name) const
{
	auto cIter = std::find_if(m_buttons.cbegin(), m_buttons.cend(), [name](const auto& button) { return button.name == name; });
	assert(cIter != m_buttons.cend());
	
	return (*cIter);
}

void UILayer::onComponentIntersect(sf::IntRect mouseRect)
{
	for (auto& button : m_buttons)
	{
		if (button.changeOnIntersect)
		{
			if (!button.currentlyIntersected && mouseRect.intersects(button.AABB))
			{
				button.sprite.incrementFrameID();
				button.currentlyIntersected = true;
				break;
			}
			else if (button.currentlyIntersected && !mouseRect.intersects(button.AABB))
			{
				button.sprite.incrementFrameID();
				button.currentlyIntersected = false;
				break;
			}
		}
	}
}

void UILayer::onComponentIntersect(sf::IntRect mouseRect, UIComponentIntersectionDetails & intersectionDetails)
{
	for (const auto& textBox : m_textBoxes)
	{
		if (mouseRect.intersects(textBox.AABB))
		{
			intersectionDetails.m_intersected = true;
			intersectionDetails.m_name = textBox.name;
			intersectionDetails.m_type = eUIComponentType::eTextBox;
			break;
		}
	}

	if (!intersectionDetails.isIntersected())
	{
		for (auto& button : m_buttons)
		{
			if (mouseRect.intersects(button.AABB))
			{
				button.sprite.incrementFrameID();

				intersectionDetails.m_intersected = true;
				intersectionDetails.m_name = button.name;
				intersectionDetails.m_type = eUIComponentType::eButton;
				break;
			}
		}
	}
}

void UILayer::activateTimedVisibilityComponent(eUIComponentName componentName, eUIComponentType componentType)
{
	switch (componentType)
	{
	case eUIComponentType::eButton:
	{
		auto iter = std::find_if(m_buttons.begin(), m_buttons.end(), [componentName](const auto& button) { return button.name == componentName; });
		assert(iter != m_buttons.end());
		iter->visible = true;
		m_timedVisibleButton = &(*iter);
		m_componentVisiblityTimer.setActive(true);
		return;
	}

	case eUIComponentType::eTextBox:
	{
		auto iter = std::find_if(m_textBoxes.begin(), m_textBoxes.end(), [componentName](const auto& textBox) { return textBox.name == componentName; });
		assert(iter != m_textBoxes.end());
		iter->visible = true;
		m_timedVisibleTextBox = &(*iter);
		m_componentVisiblityTimer.setActive(true);
		return;
	}
	}

	assert(false);
}

void UILayer::setComponentVisibility(eUIComponentName componentName, eUIComponentType componentType, bool visible)
{
	switch (componentType)
	{
	case eUIComponentType::eButton:
	{
		auto iter = std::find_if(m_buttons.begin(), m_buttons.end(), [componentName](const auto& button) { return button.name == componentName; });
		assert(iter != m_buttons.end());
		iter->visible = visible;
		return;
	}

	case eUIComponentType::eTextBox:
	{
		auto iter = std::find_if(m_textBoxes.begin(), m_textBoxes.end(), [componentName](const auto& textBox) { return textBox.name == componentName; });
		assert(iter != m_textBoxes.end());
		iter->visible = visible;
		return;
	}
	}

	assert(false);
}

void UILayer::setComponentFrameID(eUIComponentName componentName, eUIComponentType componentType, int frameID)
{
	switch (componentType)
	{
	case eUIComponentType::eButton :
	{
		auto iter = std::find_if(m_buttons.begin(), m_buttons.end(), [componentName](const auto& button) { return button.name == componentName; });
		assert(iter != m_buttons.end());
		iter->sprite.setFrameID(frameID);
		return;
	}
		
	case eUIComponentType::eImage :
	{
		auto iter = std::find_if(m_images.begin(), m_images.end(), [componentName](const auto& image) { return image.name == componentName; });
		assert(iter != m_images.end());
		iter->sprite.setFrameID(frameID);
		return;
	}
	}

	assert(false);
}

void UILayer::setButtons(std::vector<UIComponentButton>&& buttons)
{
	assert(m_buttons.empty());
	m_buttons = std::move(buttons);
}

void UILayer::setTextBoxes(std::vector<UIComponentTextBox>&& textBoxes)
{
	assert(m_textBoxes.empty());
	m_textBoxes = std::move(textBoxes);
}

void UILayer::setImages(std::vector<UIComponentImage>&& images)
{
	assert(m_images.empty());
	m_images = std::move(images);
}

void UILayer::resetButtons()
{
	for (auto& button : m_buttons)
	{
		button.currentlyIntersected = false;
		button.sprite.setFrameID(0);
	}
}

void UILayer::update(float deltaTime)
{
	m_componentVisiblityTimer.update(deltaTime);
	if (m_componentVisiblityTimer.isExpired())
	{
		m_componentVisiblityTimer.reset();
		m_componentVisiblityTimer.setActive(false);

		if (m_timedVisibleButton)
		{
			m_timedVisibleButton->visible = false;
			m_timedVisibleButton = nullptr;
		}
		
		if (m_timedVisibleTextBox)
		{
			m_timedVisibleTextBox->visible = false;
			m_timedVisibleTextBox = nullptr;
		}
	}
}

void UILayer::render(sf::RenderWindow & window) const
{
	for (const auto& image : m_images)
	{
		image.sprite.render(window);
	}

	for (const auto& button : m_buttons)
	{
		if (button.visible)
		{
			button.sprite.render(window);
		}
	}

	for (const auto& textBox : m_textBoxes)
	{
		if (textBox.visible)
		{
			window.draw(textBox.text);
		}
	}
}