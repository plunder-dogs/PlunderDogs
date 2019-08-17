#include "UILayerManager.h"
#include <assert.h>

UILayerManager::UILayerManager()
	: m_uiLayers(),
	m_currentLayer(nullptr)
{
	m_uiLayers[static_cast<int>(eGameState::eMainMenu)].reserve(size_t(3));

}

void UILayerManager::switchToLayer(eGameState gameState)
{
	auto iter = m_uiLayers.find(gameState);
	assert(iter != m_uiLayers.cend());

	m_currentLayer = (*iter->second);
}

void UILayerManager::renderCurrentLayer(sf::RenderWindow & window)
{
	assert(m_currentLayer);
}

void UILayerManager::updateCurrentLayer(sf::IntRect mouseRect)
{
	assert(m_currentLayer);
}
