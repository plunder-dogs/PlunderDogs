#pragma once

#include "Global.h"
#include <cmath>    //asin()
#include <utility>

struct Tile;

namespace MouseSelection
{
constexpr double PI = 3.14159265;

double radsToDeg(double angle);

sf::Vector2i coordToHexPos(sf::Vector2i coord);

//returns a pair, first is the magnitude of the line from the start point to the end point, the y is the closest eDirection.  If the start point is the same as the end point it 
std::pair<double, eDirection> calculateDirection(sf::Vector2i startLocation, sf::Vector2i endLocation);

//returns a pair, x is the magnitude of the line from the start point to the end point, the y is the closest eDirection.
std::pair<double, eDirection> calculateDirection(const Tile* startLocation, const Tile* endLocation);
}