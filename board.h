#ifndef BOARDMODEL_H
#define BOARDMODEL_H

#include <vector>

struct Coord
{
  Coord(int x = 0, int y = 0);
  int x;
  int y;
};

bool operator==(const Coord &c1, const Coord &c2);

class Board_Base
{
public:
  virtual ~Board_Base() = default;
  virtual bool occupy(int player, Coord coord) = 0;

  inline const std::vector<int> *board() const { return &occupied; }
  inline int step() const { return step_count; }
  inline bool valid_coord(Coord coord) const
  {
    return coord.x < 1 || coord.x > 19 || coord.y < 1 || coord.y > 19 ? false : true;
  }
  inline int coord_trans(Coord coord) const { return (coord.y - 1) * 19 + coord.x - 1; }

protected:
  Board_Base();
  int step_count{0};
  std::vector<int> occupied;
};

class Simple_Board : public Board_Base
{
public:
  Simple_Board();
  bool occupy(int player, Coord coord) override;
};

class Test_Board : public Simple_Board
{
public:
  Test_Board();
  Test_Board(const Simple_Board &simple);
  void remove(Coord coord);
};

class Actual_Board : public Simple_Board
{
public:
  Actual_Board();
  void init();
};

#endif // BOARDMODEL_H
