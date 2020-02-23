#ifndef AI_H
#define AI_H

#include <vector>
#include "board.h"
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

class AI
{
public:
  AI(unsigned depth = 6); // in this programing, depth can only be even
  Coord select_point(Actual_Board *board);

private:
  int _nega_scout(Test_Board *board,
                  int alpha, int beta,
                  unsigned depth,
                  Coord coord = {0, 0});
  int _nega_scout_killer(Test_Board *board,
                  int alpha, int beta,
                  unsigned depth,
                  Coord coord = {0, 0});
  bool _terminal_test(Test_Board *board, Coord coord) const;
  bool _terminal_test_dir(Test_Board *board, Coord coord, Direction dir) const; // dir can only be {1, 0}, {0 ,1}, {1, 1}, {-1, 1}
  std::vector<Action> _actions(Test_Board *board) const;
  std::vector<Action> _actions_killer(Test_Board *board) const;
  int _heuristic(Test_Board *board);
  int _analysis_shape(Test_Board *board, Coord coord, Direction dir,
                int *blank_prefix, bool *player_prefix, bool *opponent_prefix);
  Chess_Shape _analysis_shape_line(Test_Board *board, Coord coord, Direction dir, int blank_prefix) const; // dir can only be {1, 0}, {0 ,1}, {1, 1}, {-1, 1}
  unsigned _max_depth{0};
  unsigned _killer_depth{0};
  unsigned _breadth{0};
  Coord _selection{0, 0};
  std::vector<std::vector<int>> _player_shapes;
  std::vector<std::vector<int>> _opponent_shapes;

  inline int _player(Board_Base *board) const { return board->step() % 2; }
};


#endif // AI_H
