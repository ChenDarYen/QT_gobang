#include <cmath>
#include "board.h"
#include "direction.h"
using std::vector;
using std::pow;

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

// Actual_Board
Actual_Board::Actual_Board() : Simple_Board() {}

void Actual_Board::init()
{
  step_count = 0;
  for(auto it = occupied.begin(), end = occupied.end(); it != end; ++it)
    *it = -1;
}

// Test_Board
Test_Board::Test_Board(int owner) : Simple_Board(),
                                    _owner(owner),
                                    _position_h(361, 0) {}

Test_Board::Test_Board(int owner, const Simple_Board &simple) :
  Simple_Board(),
  _owner(owner),
  _position_h(361, 0)
{
  occupied = *simple.board();
  step_count = simple.step();

  // initialize the positions' heuristic
  for(int x = 1; x <= 19; ++x)
    for(int y = 1; y <= 19; ++y)
      if(occupied[coord_trans({x, y})] == -1)
        _position_h[coord_trans({x, y})] = _coord_heuristic({x, y});
}

bool Test_Board::occupy(int player, Coord coord)
{
  if(!Simple_Board::occupy(player, coord))
    return false;

  _position_h[coord_trans(coord)] = 0;
  _update_position_h(coord);

  return true;
}

void Test_Board::remove(Coord coord)
{
  if(valid_coord(coord))
    if (occupied[coord_trans(coord)] != -1)
    {
      occupied[coord_trans(coord)] = -1;
      _update_position_h(coord);
      --step_count;
    }
}

void Test_Board::_update_position_h(Coord coord)
{
  _update_position_h_dir(coord, {1, 0});
  _update_position_h_dir(coord, {0, 1});
  _update_position_h_dir(coord, {1, 1});
  _update_position_h_dir(coord, {-1, 1});
}

void Test_Board::_update_position_h_dir(Coord coord, Direction dir)
{
  Coord stunt = coord + dir;
  if(valid_coord(stunt))
  {
    if(occupied[coord_trans(stunt)] == -1)
    {
      _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);

      stunt = stunt + dir;
      if(valid_coord(stunt))
      {
        if(occupied[coord_trans(stunt)] == -1)
          _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);
        else
        {
          int target = occupied[coord_trans(stunt)];
          stunt = stunt + dir;
          while(valid_coord(stunt) && occupied[coord_trans(stunt)] == target)
            stunt = stunt + dir;
          if(valid_coord(stunt) && occupied[coord_trans(stunt)] == -1)
            _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);
        }
      }

    }
    else
    {
      int target = occupied[coord_trans(stunt)];
      stunt = stunt + dir;
      while(valid_coord(stunt) && occupied[coord_trans(stunt)] == target)
        stunt = stunt + dir;
      if(valid_coord(stunt) && occupied[coord_trans(stunt)] == -1)
        _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);
    }
  }

  stunt = coord - dir;
  if(valid_coord(stunt))
  {
    if(occupied[coord_trans(stunt)] == -1)
    {
      _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);

      stunt = stunt - dir;
      if(valid_coord(stunt))
      {
        if(occupied[coord_trans(stunt)] == -1)
          _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);
        else
        {
          int target = occupied[coord_trans(stunt)];
          stunt = stunt - dir;
          while(valid_coord(stunt) && occupied[coord_trans(stunt)] == target)
            stunt = stunt - dir;
          if(valid_coord(stunt) && occupied[coord_trans(stunt)] == -1)
            _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);
        }
      }

    }
    else
    {
      int target = occupied[coord_trans(stunt)];
      stunt = stunt - dir;
      while(valid_coord(stunt) && occupied[coord_trans(stunt)] == target)
        stunt = stunt - dir;
      if(valid_coord(stunt) && occupied[coord_trans(stunt)] == -1)
        _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);
    }
  }
}

int Test_Board::_coord_heuristic(Coord coord) const
{
  return _coord_heuristic_dir(coord, {1, 0}) + _coord_heuristic_dir(coord, {0, 1}) +
         _coord_heuristic_dir(coord, {1, 1}) + _coord_heuristic_dir(coord, {-1, 1});
}

int Test_Board::_coord_heuristic_dir(Coord coord, Direction dir) const
{
  // consider both block and connect chesses
  return _critical(coord, dir, false) + _critical(coord, dir, true);
}

int Test_Board::_critical(Coord coord, Direction dir, bool connec) const
{
  int score = 0;
  int target = !connec ? _owner : !_owner;

  int i = 0, chance = 2;
  int shape_int = 0;
  Coord stunt_coord = coord - dir;
  // front of direction
  while(i < 5 && chance)
  {
    if(!valid_coord(stunt_coord) ||
       (occupied[coord_trans(stunt_coord)] == target))
    {
      shape_int += pow(10, i); // 1 means block
      break;
    }
    else if(occupied[coord_trans(stunt_coord)] == -1)
    {
      shape_int += pow(10, i) * 2; // 2 means blank
      --chance;
    }

    ++i;
    stunt_coord = stunt_coord - dir;
  }

  score += _shape_h(shape_int);

  // back of direction
  i = 0;
  chance = 2;
  shape_int = 0;
  stunt_coord = coord + dir;
  while(i < 5 && chance)
  {
    if(!valid_coord(stunt_coord) ||
       (occupied[coord_trans(stunt_coord)] == target))
    {
      shape_int += pow(10, i); // 1 means block
      break;
    }
    else if(occupied[coord_trans(stunt_coord)] == -1)
    {
      shape_int += pow(10, i) * 2; // 2 means blank
      --chance;
    }

    ++i;
    stunt_coord = stunt_coord + dir;
  }

  score += _shape_h(shape_int);

  int multiple = 10;
  if(score >= 14)
    multiple = 2000;
  else if(score >= 10)
    multiple = 300;
  else if(score >= 7)
    multiple = 50;

  score *= multiple;

  int diff = multiple / 2;

  return !connec ? score : score ? score + diff : 0; // connec is more important then block
}

int Test_Board::_shape_h(int shape) const
{
  switch(shape)
  {
    case 10: case 102:
      return 2;

    case 202:
      return 3;

    case 220:
      return 4;

    case 100: case 1002: case 1020:
      return 6;

    case 2002: case 2020:
      return 7;

    case 2200:
      return 8;

    case 1000: case 10200: case 10020: case 10002:
      return 10;

    case 20002: case 20020: case 20200:
      return 11;

    case 22000:
      return 12;

    case 10000: case 20000:
      return 14;

    default:
      return 0;
  }
}
