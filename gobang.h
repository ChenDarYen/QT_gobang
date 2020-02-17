#ifndef GOBANGMODEL_H
#define GOBANGMODEL_H

#include <vector>
#include "board.h"

class MainWindow;
class BoardView;
class AI;
class AIController;
struct Direction;

class Gobang
{
public:
  static Gobang *get();
  ~Gobang();
  void play(int winner = -1);
  void place_chess(bool is_AI, Coord coord = {0, 0});
  const std::vector<int> *board() const;
  void latest_move(Coord *AI_move, Coord *player_move) const;

  void setWindow(MainWindow *window);
  void setBoardView(BoardView *board_view);
  void setAIController(AIController *controller);

private:
  Gobang();
  bool _victory(Coord coord) const;
  bool _victory_dir(Coord coord, Direction dir) const; // dir can only be {1, 0}, {0 ,1}, {1, 1}, {-1, 1}
  bool _player_tern{false};
  int _computer{0};
  Coord _latest_player_move{0, 0};
  Coord _latest_AI_move{0, 0};
  Actual_Board *_board{nullptr};
  AI *_AI{nullptr};

  // views
  MainWindow *_window{nullptr};
  BoardView *_board_view{nullptr};

  // controller
  AIController *_ai_controller{nullptr};
};

#endif // GOBANGMODEL_H
