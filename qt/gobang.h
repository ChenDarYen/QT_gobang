#ifndef GOBANGMODEL_H
#define GOBANGMODEL_H

#include <vector>
#include "board.h"

class MainWindow;
class BoardView;
class AIMCTS;
class AIController;
struct Direction;

class GobangBase
{
public:
  GobangBase(BoardBase *board);
  virtual const std::vector<int> *board() const;
  virtual bool place_chess(Coord coord) = 0;
  virtual ~GobangBase() = default;

protected:
  bool _victory(Coord coord) const;
  bool _victory_dir(Coord coord, Direction dir) const; // dir can only be {1, 0}, {0 ,1}, {1, 1}, {-1, 1}

  int _player{1};
  BoardBase *_board{nullptr};
};

class GobangSimulate : public GobangBase
{
public:
  GobangSimulate();
  const std::vector<int> *board() const override;
  bool place_chess(Coord coord) override;
  void set(BoardBase *board);
  std::vector<unsigned> actions() const;
  ~GobangSimulate();

  int step() { return _board->step(); }

private:
  SimulateBoard *_board{nullptr};
};

class Gobang : public GobangBase
{
public:
  static Gobang *get();
  const std::vector<int> *board() const override;
  bool place_chess(Coord coord = {-1, -1}) override;
  void play(int code = 0);
  void latest_move(Coord *black_move, Coord *white_move) const;

  void setAI(std::shared_ptr<AIMCTS> AI1, std::shared_ptr<AIMCTS> AI2 = nullptr);
  void setWindow(MainWindow *window);
  void setBoardView(BoardView *board_view);
  void setAIController(AIController *controller);
  ~Gobang();

private:
  Gobang();

  ActualBoard *_board{nullptr};
  bool _player_tern{false};
  Coord _latest_black_move{0, 0};
  Coord _latest_white_move{0, 0};
  std::shared_ptr<AIMCTS> _AI_black{nullptr};
  std::shared_ptr<AIMCTS> _AI_white{nullptr};

  // views
  MainWindow *_window{nullptr};
  BoardView *_board_view{nullptr};

  // controller
  AIController *_ai_controller{nullptr};
};

#endif // GOBANGMODEL_H
