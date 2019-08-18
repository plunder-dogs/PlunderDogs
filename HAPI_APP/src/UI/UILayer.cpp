#include "UILayer.h"
#include <assert.h>

UIComponentIntersectionDetails::UIComponentIntersectionDetails(bool intersected)
	: m_intersected(intersected),
	m_type(),
	m_name(),
	m_frameID(0)
{}

UIComponentIntersectionDetails::UIComponentIntersectionDetails(bool intersected, int frameID)
	: m_intersected(intersected),
	m_type(),
	m_name(),
	m_frameID(frameID)
{}

bool UIComponentIntersectionDetails::isIntersected() const
{
	return m_intersected;
}

int UIComponentIntersectionDetails::getFrameID() const
{
	assert(m_intersected);
	return m_frameID;
}

eUIComponentType UIComponentIntersectionDetails::getComponentType()
{
	assert(m_intersected);
	assert(m_type == eUIComponentType::eButton);
	return m_type;
}

eUIComponentName UIComponentIntersectionDetails::getComponentName()
{
	assert(m_intersected);
	return m_name;
}

//UI Layer
UILayer::UILayer()
	: m_buttons(),
	m_textBoxes(),
	m_images()
{}

UIComponentIntersectionDetails UILayer::getIntersectionDetails(sf::IntRect mouseRect)
{
	UIComponentIntersectionDetails intersectionDetails(false);
	for (auto& textBox : m_textBoxes)
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

	return intersectionDetails;
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

void UILayer::resetButtonsFrameID()
{
	for (auto& button : m_buttons)
	{
		button.sprite.setFrameID(0);
	}
}

void UILayer::render(sf::RenderWindow & window)
{
	for (auto& image : m_images)
	{
		image.render(window);
	}

	for (auto& button : m_buttons)
	{
		button.sprite.render(window);
	}

	for (auto& textBox : m_textBoxes)
	{
		window.draw(textBox.text);
	}
}