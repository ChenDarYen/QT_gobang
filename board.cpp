#include "board.h"
using std::vector;

// Coord
Coord::Coord(int x, int y) : x(x), y(y) {}

bool operator==(const Coord &c1, const Coord &c2)
{
  return c1.x == c2.x && c1.y == c2.y;
}

// Board_base
Board_Base::Board_Base() : occupied(361, -1) {} // 19 * 19 = 361

// Simple_Board
Simple_Board::Simple_Board() : Board_Base() {}

bool Simple_Board::occupy(int player, Coord coord)
{
  if(!valid_coord(coord) || occupied[coord_trans(coord)] != -1)
    return false;
  if(player != 0 && player != 1)
    return false;

  occupied[coord_trans(coord)] = player;
  ++step_count;

  return true;
}

// Test_Board
Test_Board::Test_Board() : Simple_Board() {}

Test_Board::Test_Board(const Simple_Board &simple) : Simple_Board()
{
  occupied = *simple.board();
  step_count = simple.step();
}

void Test_Board::remove(Coord coord)
{
  if(valid_coord(coord))
    if (occupied[coord_trans(coord)] != -1)
    {
      occupied[coord_trans(coord)] = -1;
      --step_count;
    }
}

// Actual_Board
Actual_Board::Actual_Board() : Simple_Board() {}

void Actual_Board::init()
{
  step_count = 0;
  for(auto it = occupied.begin(), end = occupied.end(); it != end; ++it)
    *it = -1;
}
