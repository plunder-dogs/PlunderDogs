#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

struct FrameDetails;
class Texture;
class Map;
class Sprite
{
public:
	Sprite(bool active = true);
	Sprite(std::unique_ptr<Texture>& texture, bool active = true);
	Sprite(std::unique_ptr<Texture>& texture, sf::Vector2f startingPosition, bool active = true);

	sf::Vector2i getPosition() const;
	const FrameDetails& getCurrentFrameDetails() const;
	bool isActive() const;
	int getCurrentFrameID() const;
	
	void setFrameID(int frameID);
	void setPosition(sf::Vector2i newPosition);
	void setPosition(sf::Vector2i newPosition, const Map& map);
	void setScale(sf::Vector2f scale);
	void render(sf::RenderWindow& window);
	void render(sf::RenderWindow& window, const Map& map);
	void render(sf::RenderWindow& window, const Map& map, sf::Vector2i position);
	void setTexture(std::unique_ptr<Texture>& texture, int frameID = 0);
	void activate();
	void deactivate();

private:
	const Texture* m_texture;
	sf::Sprite m_sprite;
	sf::Vector2i m_position;
	int m_currentFrameID;
	bool m_isActive;
};