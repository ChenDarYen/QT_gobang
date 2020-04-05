#ifndef BOARD_H
#define BOARD_H

#include <vector>

struct Coord
{
  Coord(int x = 0, int y = 0);
  int x;
  int y;
};

bool operator==(const Coord &c1, const Coord &c2);

class BoardBase
{
public:
  virtual ~BoardBase() = default;
  virtual bool occupy(int player, Coord coord);

  inline const std::vector<int> *board() const { return &occupied; }
  inline int step() const { return step_count; }

  static inline bool valid_coord(Coord coord)
  {
    return coord.x < 0 || coord.x >= 15 || coord.y < 0 || coord.y >= 15 ? false : true;
  }
  static inline int coord_trans(Coord coord)
  {
    return coord.y * 15 + coord.x;
  }

protected:
  BoardBase();
  int step_count{0};
  std::vector<int> occupied;
};

class SimulateBoard : public BoardBase
{
public:
  SimulateBoard();
  void set(const BoardBase *board);
  std::vector<unsigned> actions() const;
};

class ActualBoard : public BoardBase
{
public:
  static ActualBoard *get();
  void init();

private:
  ActualBoard();
};

#endif // BOARD_H
