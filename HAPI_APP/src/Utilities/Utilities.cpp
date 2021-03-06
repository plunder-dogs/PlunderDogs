#include "Utilities.h"
#include <random>
#include "../Map.h"
#include <algorithm>

int Utilities::getRandomNumber(int min, int max)
{
	static std::random_device rd;  //Will be used to obtain a seed for the random number engine
	static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> dis(min, max);

	return dis(gen);
}

bool Utilities::isPositionFound(const std::vector<const Tile*>& tileArea, sf::Vector2i position)
{
	auto cIter = std::find_if(tileArea.cbegin(), tileArea.cend(), [position](const auto& tile) { return tile->m_tileCoordinate == position; });
	return cIter != tileArea.cend();
}

int Utilities::numbOfPositionFound(const std::vector<const Tile*>& tileArea, sf::Vector2i position)
{
	int i = 0;
	for (int i = 0; i < tileArea.size(); ++i)
	{
		if (tileArea[i]->m_tileCoordinate == position)
		{
			++i;
		}
	}

	return i;
}

eDirection Utilities::getOppositeDirection(eDirection direction)
{
	eDirection oppositeDirection;
	switch (direction)
	{
	case eDirection::eNorth:
		oppositeDirection = eDirection::eSouth;
		break;
	case eDirection::eNorthEast:
		oppositeDirection = eDirection::eSouthWest;
		break;
	case eDirection::eSouthEast:
		oppositeDirection = eDirection::eNorthWest;
		break;
	case eDirection::eSouth:
		oppositeDirection = eDirection::eNorth;
		break;
	case eDirection::eSouthWest:
		oppositeDirection = eDirection::eNorthEast;
		break;
	case eDirection::eNorthWest:
		oppositeDirection = eDirection::eSouthEast;
		break;
	}

	return oppositeDirection;
}

double Math::radsToDeg(double angle)
{
	return 180 * (angle / Math::PI);
}

bool Math::isFacingDifferentTile(double i)
{
	return i > 20;
}

sf::Vector2i Math::coordToHexPos(sf::Vector2i coord)
{
	const float xPos = static_cast<float>(coord.x * 12);
	const float yPos = static_cast<float>((2 * coord.y - (coord.x & 1)) * 7);
	return sf::Vector2i(static_cast<int>(xPos), static_cast<int>(yPos));
}

std::pair<double, eDirection> Math::calculateDirection(sf::Vector2i startLocation, sf::Vector2i endLocation)
{
	std::pair<double, double> distance{ endLocation.x - startLocation.x, endLocation.y - startLocation.y }; // Calculates the distance vector of the line between the start and end point.
	// Calculating the magnitude of the line between the start and end point.
	double magnitude{ 0 };
	if (distance.first || distance.second)
	{
		magnitude = sqrt(distance.first * distance.first + distance.second * distance.second);
	}
	std::pair<double, eDirection> returnVariable{ magnitude, eDirection::eNorth };

	if (magnitude)
	{
		if (distance.first == 0)
		{
			if (distance.second > 0)
			{
				returnVariable.second = eDirection::eSouth;
				return returnVariable;
			}
			else returnVariable.second = eDirection::eNorth;
			return returnVariable;
		}
		double angle;
		if (distance.first > 0)																					// Calculating the angle between the verticle and the distance vector.
		{
			angle = 90 + Math::radsToDeg(atan(distance.second / distance.first));
		}
		else
		{
			angle = 270 + Math::radsToDeg(atan(distance.second / distance.first));
		}
		int tmp{ (int)(angle + 30) / 60 };																		// This calculation is just to manipulate the angle into a number that makes the switch case more convenient.

		switch (tmp)
		{
		case 0: returnVariable.second = eDirection::eNorth;
			break;
		case 1: returnVariable.second = eDirection::eNorthEast;
			break;
		case 2: returnVariable.second = eDirection::eSouthEast;
			break;
		case 3: returnVariable.second = eDirection::eSouth;
			break;
		case 4: returnVariable.second = eDirection::eSouthWest;
			break;
		case 5: returnVariable.second = eDirection::eNorthWest;
			break;
		}
	}
	return returnVariable;
}

std::pair<double, eDirection> Math::calculateDirection(const Tile* startLocation, const Tile* endLocation)
{
	if (!startLocation || !endLocation)
		return { 0.0 , eDirection::eNorth };
	// Calculating the distance vector of the line between the start and end point
	sf::Vector2i startPos{ Math::coordToHexPos(startLocation->m_tileCoordinate) };
	sf::Vector2i endPos{ Math::coordToHexPos(endLocation->m_tileCoordinate) };
	sf::Vector2i distance{ endPos.x - startPos.x, endPos.y - startPos.y };
	// Calculating the magnitude of the line between the start and end point.
	double magnitude{ sqrt(distance.x * distance.x + distance.y * distance.y) };
	std::pair<double, eDirection> returnVariable{ magnitude, eDirection::eNorth };

	if (magnitude)
	{
		if (distance.x == 0)
		{
			if (distance.y > 0)
			{
				//Changed from SouthEast
				returnVariable.second = eDirection::eSouth;
				return returnVariable;
			}
			//Changed from NorthWest
			else returnVariable.second = eDirection::eNorth;
			return returnVariable;
		}
		double angle;
		if (distance.x > 0)	                                        // Calculating the angle between the verticle and the distance vector.
		{
			angle = 90 + Math::radsToDeg(atan(distance.y / distance.x));
		}
		else
		{
			angle = 270 + Math::radsToDeg(atan(distance.y / distance.x));
		}
		int tmp{ (int)(angle + 30) / 60 };								// This calculation is just to manipulate the angle into a number that makes the switch case more convenient.

		switch (tmp)
		{
		case 0: returnVariable.second = eDirection::eNorth;
			break;
		case 1: returnVariable.second = eDirection::eNorthEast;
			break;
		case 2: returnVariable.second = eDirection::eSouthEast;
			break;
		case 3: returnVariable.second = eDirection::eSouth;
			break;
		case 4: returnVariable.second = eDirection::eSouthWest;
			break;
		case 5: returnVariable.second = eDirection::eNorthWest;
			break;
		}
	}
	return returnVariable;
}