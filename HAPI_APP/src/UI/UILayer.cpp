#include "UILayer.h"
#include <assert.h>
#include <algorithm>

UILayer::UILayer()
	: m_buttons(),
	m_textBoxes(),
	m_images()
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

void UILayer::setImages(std::vector<Sprite>&& images)
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

void UILayer::render(sf::RenderWindow & window) const
{
	for (const auto& image : m_images)
	{
		image.render(window);
	}

	for (const auto& button : m_buttons)
	{
		button.sprite.render(window);
	}

	for (const auto& textBox : m_textBoxes)
	{
		window.draw(textBox.text);
	}
}