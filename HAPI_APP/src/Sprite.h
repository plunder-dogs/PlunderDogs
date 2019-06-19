#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

struct FrameDetails;
struct Texture;
class Map;
class Sprite
{
public:
	Sprite(bool active = true);
	Sprite(const Texture& texture, sf::Vector2i startingPosition, bool active = true);
	Sprite(std::unique_ptr<Texture>& texture, bool active = true);
	Sprite(std::unique_ptr<Texture>& texture, sf::Vector2f startingPosition, bool active = true);

	sf::Vector2f getSize() const;
	sf::Vector2i getPosition() const;
	const FrameDetails& getCurrentFrameDetails() const;
	bool isActive() const;
	bool isAnimationCompleted() const;
	int getCurrentFrameID() const;

	void setFrameID(int frameID);
	void incrementFrameID();
	void setPosition(sf::Vector2i newPosition);
	void setRotation(float angle);
	void rotate(float angle);
	void setOriginAtCenter();
	void setScale(sf::Vector2f scale);
	void render(sf::RenderWindow& window);
	void render(sf::RenderWindow& window, const Map& map);
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