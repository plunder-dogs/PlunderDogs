#pragma once

#include "Global.h"
#include <cmath>    //asin()
#include <utility>

struct Tile;

namespace MouseSelection
{
constexpr double PI = 3.14159265;

double radsToDeg(double angle);

std::pair<int, int> coordToHexPos(std::pair<int, int> coord);

//returns a pair, first is the magnitude of the line from the start point to the end point, the second is the closest eDirection.  If the start point is the same as the end point it 
std::pair<double, eDirection> calculateDirection(std::pair<int, int> startLocation, std::pair<int, int> endLocation);

//returns a pair, first is the magnitude of the line from the start point to the end point, the second is the closest eDirection.
std::pair<double, eDirection> calculateDirection(const Tile* startLocation, const Tile* endLocation);
}