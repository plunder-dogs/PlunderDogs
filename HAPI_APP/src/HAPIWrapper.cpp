#include "HAPIWrapper.h"
#include "Utilities/Utilities.h"

bool HAPI_Wrapper::isTranslated(std::unique_ptr<Sprite>& sprite, const HAPI_TMouseData & mouseData, int frameRect)
{
	return sprite->GetSpritesheet()->GetFrameRect(frameRect).Translated(
		sprite->GetTransformComp().GetPosition()).Contains(HAPISPACE::RectangleI(mouseData.x, mouseData.x, mouseData.y, mouseData.y));
}

std::unique_ptr<Sprite> HAPI_Wrapper::makeSprite(const std::string & spriteName)
{
	return std::unique_ptr<Sprite>(HAPI_Sprites.MakeSprite(Utilities::getDataDirectory() + spriteName));
}

std::unique_ptr<Sprite> HAPI_Wrapper::makeSprite(const std::string & spriteName, int numFrames)
{
	return std::unique_ptr<Sprite>(HAPI_Sprites.MakeSprite(Utilities::getDataDirectory() + spriteName, numFrames));
}

std::unique_ptr<Sprite> HAPI_Wrapper::loadSprite(const std::string & spriteName)
{
	return std::unique_ptr<Sprite>(HAPI_Sprites.LoadSprite(Utilities::getDataDirectory() + spriteName));
}

std::shared_ptr<SpriteSheet> HAPI_Wrapper::loadSpriteSheet(const std::string & name)
{
	return std::shared_ptr<SpriteSheet>(HAPI_Sprites.LoadSpritesheet(Utilities::getDataDirectory() + name));
}

std::shared_ptr<SpriteSheet> HAPI_Wrapper::makeSpriteSheet(const std::string & name)
{
	return std::shared_ptr<SpriteSheet>(HAPI_Sprites.MakeSpritesheet(Utilities::getDataDirectory() + name));
}

std::shared_ptr<SpriteSheet> HAPI_Wrapper::makeSpriteSheet(const std::string & name, int numFrames)
{
	return std::shared_ptr<SpriteSheet>(HAPI_Sprites.MakeSpritesheet(Utilities::getDataDirectory() + name, numFrames));
}

std::pair<int, int> HAPI_Wrapper::getMouseLocation()
{
	return std::pair<int, int>(HAPI_Sprites.GetMouseData().x, HAPI_Sprites.GetMouseData().y);
}

void HAPI_Wrapper::render(std::unique_ptr<Sprite>& sprite)
{
	sprite->Render(SCREEN_SURFACE);
}

void HAPI_Wrapper::setPosition(std::unique_ptr<Sprite>& sprite, VectorF position)
{
	sprite->GetTransformComp().SetPosition(position);
}

void HAPI_Wrapper::clearScreen()
{
	SCREEN_SURFACE->Clear();
}

void HAPI_Wrapper::addWindow(const std::string & name, const RectangleI & rect)
{
	UI.AddWindow(name, rect);
}
