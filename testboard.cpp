#include "testboard.h"
#include "direction.h"
#include <cmath>

using std::vector;
using std::pow;

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
      _position_h[coord_trans(coord)] = _coord_heuristic(coord);
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
      {
        _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);
        stunt = stunt + dir;
        while(valid_coord(stunt) && occupied[coord_trans(stunt)] == target)
          stunt = stunt + dir;
        if(valid_coord(stunt) && occupied[coord_trans(stunt)] == -1)
          _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);
      }
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
      {
        _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);
        stunt = stunt - dir;
        while(valid_coord(stunt) && occupied[coord_trans(stunt)] == target)
          stunt = stunt - dir;
        if(valid_coord(stunt) && occupied[coord_trans(stunt)] == -1)
          _position_h[coord_trans(stunt)] = _coord_heuristic(stunt);
      }
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
  int target = !connec ? _owner : !_owner;

  int i = 0, chance = 2;
  int shape_front = 0;
  Coord stunt_coord = coord - dir;
  // front of direction
  while(i < 4 && chance)
  {
    if(!valid_coord(stunt_coord) ||
       (occupied[coord_trans(stunt_coord)] == target))
    {
      shape_front += pow(10, i); // 1 means block
      break;
    }
    else if(occupied[coord_trans(stunt_coord)] == -1)
    {
      shape_front += pow(10, i) * 2; // 2 means blank
      --chance;
    }

    ++i;
    stunt_coord = stunt_coord - dir;
  }

  if(shape_front == 0) // already form a 活四 or 沖四
    return 10000;

  // back of direction
  i = 0;
  chance = 2;
  stunt_coord = coord + dir;
  int shape_back = 3; // 3 means the coord's position
  while(i < 4 && chance)
  {
    shape_back *= 10;

    if(!valid_coord(stunt_coord) ||
       (occupied[coord_trans(stunt_coord)] == target))
    {
      shape_back += 1;
      break;
    }
    else if(occupied[coord_trans(stunt_coord)] == -1)
    {
      shape_back += 2;
      --chance;
    }

    ++i;
    stunt_coord = stunt_coord + dir;
  }

  if(shape_back == 30000)
    return 10000;

  int shape = shape_front * pow(10, ceil(log10(shape_back))) + shape_back;

  return _shape_h(shape);
}

int Test_Board::_shape_h(int shape) const
{
  switch(shape)
  {
    case 1000301: case 10003022: case 10003001: case 100030201: case 100030202:
    case 100030022: case 100030001: case 100030020: case 100030200: case 100030002:
    case 10003021: case 100030021:
    case 1030001: case 22030001: case 10030001: case 102030001: case 202030001:
    case 220030001: case 20030001: case 2030001: case 200030001: case 12030001:
    case 120030001:

    case 2000301: case 20003022: case 20003001: case 200030201: case 200030202:
    case 200030022: case 200030020: case 200030200: case 200030002: case 20003021:
    case 200030021:
    case 1030002: case 22030002: case 10030002: case 102030002: case 202030002:
    case 220030002: case 20030002: case 2030002: case 12030002: case 120030002:

    case 1003001: case 10030022: case 10030020: case 10030021:
    case 22003001: case 220030022: case 220030020: case 220030021:
    case 2003001: case 20030022: case 20030020: case 20030021:
    case 12003001: case 120030022: case 120030020: case 120030021:
      return 20000; // 沖四

    case 2000322: case 20003201: case 20003202: case 200032001: case 200032002:
    case 200032000: case 2000321:
    case 230002: case 200230002: case 100230002: case 20230002: case 10230002:
    case 2230002: case 1230002:

    case 12003022: case 120030201: case 120030202: case 120030200: case 12003021:
    case 22030021: case 102030021: case 202030021: case 2030021: case 12030021:

    case 22003022: case 220030201: case 220030202: case 220030200: case 22003021:
    case 22030022: case 102030022: case 202030022: case 2030022: case 12030022:

    case 2003022: case 2003021: case 20030202: case 20030201: case 20030200:
    case 22030020: case 12030020: case 202030020: case 102030020: case 2030020:

    case 2032002: case 2032001: case 2032000:
    case 200230200: case 100230200: case 230200:

    case 2003202: case 2003201: case 20032002: case 20032001: case 20032000:
    case 20230020: case 10230020: case 200230020: case 100230020: case 230020:

    case 2030202: case 2030201: case 2030200:
    case 202030200: case 102030200:

    case 102030201: case 102030202:
    case 202030201:

    case 202030202:
      return 3000; // 活三 and some critical point

    case 1000322: case 10003201: case 10003202: case 100032001: case 100032002:
    case 100032000: case 1000321:
    case 2230001: case 10230001: case 20230001: case 100230001: case 200230001:
    case 230001: case 1230001:

    case 231: case 2322: case 2321: case 23201: case 23202:
    case 232002: case 232001: case 232000: case 2301: case 23022:
    case 23021: case 23001: case 230022: case 230021: case 230202:
    case 230201:
    case 132000: case 2232000: case 1232000: case 10232000: case 20232000:
    case 200232000: case 100232000: case 1032000: case 22032000: case 12032000:
    case 10032000: case 220032000: case 120032000: case 202032000: case 102032000:

    case 2031: case 20322: case 20321: case 203202: case 203201:
    case 20301: case 203022: case 203021: case 203001:
    case 130200: case 2230200: case 1230200: case 20230200: case 10230200:
    case 1030200: case 22030200: case 12030200: case 10030200:

    case 20031: case 200322: case 200321: case 200301:
    case 100020: case 2230020: case 1230020: case 1030020:

    case 1003022: case 1003021: case 10030201: case 10030202:
    case 2203001: case 1203001: case 10203001: case 20203001:

    case 1003201: case 1003202: case 10032001: case 10032002:
    case 1023001: case 2023001: case 10023001: case 20023001:

    case 1030021: case 1030022: case 1030202: case 1030201: case 1032002:
    case 1032001:
    case 1200301: case 2200301: case 2020301: case 1020301: case 2002301:
    case 1002301:

    case 10023022: case 10023021: case 100230022: case 100230021: case 100230202:
    case 100230201:
    case 22032001: case 12032001: case 220032001: case 120032001: case 202032001:
    case 102032001:

    case 20023022: case 20023021: case 200230022: case 200230021: case 200230202:
    case 200230201:
    case 22032002: case 12032002: case 220032002: case 120032002: case 202032002:
    case 102032002:

    case 20203022: case 20203021:
    case 22030202: case 12030202:

    case 10203022: case 10203021:
    case 22030201: case 12030201:

    case 130002:
    case 200031:
      return 500; // 眠三

    case 1200322:
    case 2230021:

    case 2200321: case 2200322:
    case 1230022: case 2230022:

    case 2002321: case 2002322: case 20023201: case 20023202: case 200232002:
    case 200232001:
    case 1232002: case 2232002: case 10232002: case 20232002: case 100232002:

    case 2203022: case 2203021: case 2203202:
    case 1203022: case 2023022:

    case 2020322: case 2020321: case 20203202: case 20203201:
    case 2230202: case 1230202: case 20230202: case 10230202:

    case 1203202:
    case 2023021:
      return 100; // 活二

    case 1200321:
    case 1230021:

    case 100322: case 223001:

    case 1002322: case 1002321: case 10023202: case 10023201: case 100232001:
    case 2232001: case 1232001: case 20232001: case 10232001:

    case 1020322: case 1020321: case 10203202: case 10203201:
    case 2230201: case 1230201: case 20230201: case 10230201:

    case 103022:
    case 220301:

    case 20231: case 202322: case 202321: case 202301: case 2023202: case 2023201:
    case 13202: case 223202: case 123202: case 103202: case 1023202:

    case 1023022: case 1023021:
    case 2203201: case 1203201:

    case 1023201:
      return 10; // 眠二

    case 220322: case 220321:
    case 223022: case 123022:

    case 120322:
      return 4; // 活ㄧ

    case 22031:
    case 13022:

    case 202031:
    case 130202:

    case 10322:
    case 22301:

    case 102322:
    case 223201:
      return 2; // 眠一

    default:
      return 0;
  }
}
