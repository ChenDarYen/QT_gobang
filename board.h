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

class Board
{
public:
  Board();
  void init();
  bool occupy(int player, Coord coord);
  void remove(Coord coord);
  bool valid_coord(Coord coord) const;
  int coord_trans(Coord coord) const;
  const std::vector<int> *board() const;
  int step() const;

private:
  int step_count{0};
  std::vector<int> occupied;
};

#endif // BOARDMODEL_H
