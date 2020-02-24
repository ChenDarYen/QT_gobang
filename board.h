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

  static inline bool valid_coord(Coord coord)
  {
    return coord.x < 1 || coord.x > 19 || coord.y < 1 || coord.y > 19 ? false : true;
  }
  static inline int coord_trans(Coord coord)
  {
    return (coord.y - 1) * 19 + coord.x - 1;
  }

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

class Actual_Board : public Simple_Board
{
public:
  Actual_Board();
  void init();
};

struct Direction;

class Test_Board : public Simple_Board
{
public:
  Test_Board(int owner);
  Test_Board(int owner, const Simple_Board &simple);
  bool occupy(int player, Coord coord) override;
  void remove(Coord coord);

  inline int get_heuristic(Coord coord) const { return _position_h[coord_trans(coord)]; }

private:
  void _update_position_h(Coord coord); // update the heuristic influence by the coordinate
  void _update_position_h_dir(Coord coord, Direction dir);
  int _coord_heuristic(Coord coord) const;
  int _coord_heuristic_dir(Coord coord, Direction dir) const; // dir can only be {1, 0}, {0 ,1}, {1, 1}, {-1, 1}
  int _critical(Coord coord, Direction dir, bool connec) const; // dir can only be {1, 0}, {0 ,1}, {1, 1}, {-1, 1}
  int _shape_h(int shape) const;
  int _owner{0};
  std::vector<int> _position_h;
};

#endif // BOARDMODEL_H
