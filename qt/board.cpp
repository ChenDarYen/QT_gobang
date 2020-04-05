#include <cmath>
#include "board.h"
#include <QDebug>
using std::vector;
using std::pow;

// Coord
Coord::Coord(int x, int y) : x(x), y(y) {}

bool operator==(const Coord &c1, const Coord &c2)
{
  return c1.x == c2.x && c1.y == c2.y;
}

// Boardbase
BoardBase::BoardBase() : occupied(225, 0) {} // 15 * 15 = 225

bool BoardBase::occupy(int player, Coord coord)
{
  if(!valid_coord(coord) || occupied[coord_trans(coord)] != 0)
    return false;
  if(player != 1 && player != -1)
    return false;

  occupied[coord_trans(coord)] = player;
  ++step_count;

  return true;
}

// SimulateBoard
SimulateBoard::SimulateBoard() : BoardBase() {}

void SimulateBoard::set(const BoardBase *board)
{
  occupied = *board->board();
  step_count = board->step();
}

vector<unsigned> SimulateBoard::actions() const
{
  vector<unsigned> ret;
  for(int i = 0; i < 225; ++i)
    if(occupied[i] == 0)
      ret.push_back(i);

  return ret;
}

// Actual_Board
ActualBoard::ActualBoard() : BoardBase() {}

ActualBoard *ActualBoard::get()
{
  static auto actual_board = ActualBoard();
  return &actual_board;
}

void ActualBoard::init()
{
  step_count = 0;
  for(auto it = occupied.begin(), end = occupied.end(); it != end; ++it)
    *it = 0;
}
