#ifndef AI_H
#define AI_H

#include <vector>
#include <map>
#include <tuple>
#include "board.h"

class Zobrist;
class TestBoard;
struct Direction;

struct Action
{
  Action(int p = 0, Coord coord = {0, 0});
  int priority;
  Coord coord;
};

struct Chess_Shape
{
  int amount{0};
  bool alive{true};
};

class AIBase
{
public:
  AIBase() = default;
  virtual Coord select_point() = 0;
  virtual ~AIBase() = default;

protected:
  inline int _latest_player(BoardBase *board) const { return board->step() % 2 ? 1 : -1; }
};

class AINegaScout : public AIBase
{
public:
  AINegaScout(unsigned depth = 6); // in this programing, depth can only be even
  Coord select_point() override;
  ~AINegaScout();

private:
  int _nega_scout(TestBoard *board,
                  int alpha, int beta,
                  unsigned depth,
                  Coord coord = {0, 0});
  int _nega_scout_killer(TestBoard *board,
                  int alpha, int beta,
                  unsigned depth,
                  Coord coord = {0, 0});
  bool _terminal_test(TestBoard *board, Coord coord) const;
  bool _terminal_test_dir(TestBoard *board, Coord coord, Direction dir) const; // dir can only be {1, 0}, {0 ,1}, {1, 1}, {-1, 1}
  std::vector<Action> _actions(TestBoard *board) const;
  std::vector<Action> _actions_killer(TestBoard *board) const;
  int _heuristic(TestBoard *board, unsigned depth);
  bool _analysis_shape(TestBoard *board, Coord coord, Direction dir,
                      int *blank_prefix, bool *player_prefix, bool *opponent_prefix);
  Chess_Shape _analysis_shape_line(TestBoard *board, Coord coord, Direction dir, int blank_prefix) const; // dir can only be {1, 0}, {0 ,1}, {1, 1}, {-1, 1}
  unsigned _max_depth{0};
  unsigned _killer_depth{0};
  unsigned _breadth{0};
  Coord _selection{0, 0};
  std::vector<std::vector<int>> _player_shapes;
  std::vector<std::vector<int>> _opponent_shapes;
  Zobrist *_zobrist{nullptr};
  std::map<int, std::tuple<int, int>> _cache; // tuple contains score and steps
};


#endif // AI_H
