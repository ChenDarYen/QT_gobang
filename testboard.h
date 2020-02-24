#ifndef TEST_BOARD_H
#define TEST_BOARD_H

#include "board.h"

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

#endif // TEST_BOARD_H
