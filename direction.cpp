#include "direction.h"
#include "board.h"

Direction::Direction(int x, int y) : x(x), y(y) {}

Coord operator+(const Coord &coord, const Direction &dir)
{
  return {coord.x + dir.x, coord.y + dir.y};
}

Coord operator-(const Coord &coord, const Direction &dir)
{
  return {coord.x - dir.x, coord.y - dir.y};
}
