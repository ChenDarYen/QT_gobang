#ifndef DIRECTION_H
#define DIRECTION_H

struct Coord;

struct Direction
{
  Direction(int x = 0, int y = 0);
  int x;
  int y;
};

Coord operator+(const Coord &coord, const Direction &dir);

Coord operator-(const Coord &coord, const Direction &dir);

#endif // DIRECTION_H
