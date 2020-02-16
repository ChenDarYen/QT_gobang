#include "board.h"
using std::vector;

Coord::Coord(int x, int y) : x(x), y(y) {}

bool operator==(const Coord &c1, const Coord &c2)
{
  return c1.x == c2.x && c1.y == c2.y;
}

Board::Board() : occupied(361, -1) {}

void Board::init()
{
  step_count = 0;
  for(auto it = occupied.begin(), end = occupied.end(); it != end; ++it)
    *it = -1;
}

bool Board::occupy(int player, Coord coord)
{
  if(!valid_coord(coord) || occupied[coord_trans(coord)] != -1)
    return false;
  if(player != 0 && player != 1)
    return false;

  occupied[coord_trans(coord)] = player;
  ++step_count;

  return true;
}

void Board::remove(Coord coord)
{
  if(valid_coord(coord))
    if (occupied[coord_trans(coord)] != -1)
    {
      occupied[coord_trans(coord)] = -1;
      --step_count;
    }
}

bool Board::valid_coord(Coord coord) const
{
  if(coord.x < 1 || coord.x > 19 || coord.y < 1 || coord.y > 19)
    return false;

  return true;
}

int Board::coord_trans(Coord coord) const
{
  return (coord.y - 1) * 19 + coord.x - 1;
}

const vector<int> *Board::board() const
{
  return &occupied;
}

int Board::step() const
{
  return step_count;
}

