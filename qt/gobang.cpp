#include <QString>
#include <QApplication>
#include "gobang.h"
#include "board.h"
#include "ai.h"
#include "aimcts.h"
#include "aicontroller.h"
#include "direction.h"
#include "mainwindow.h"
#include "boardview.h"
#include "placeevent.h"

#include <QDebug>
#include <QDir>

using std::vector;
using std::make_shared;

// GobangBase

GobangBase::GobangBase(BoardBase *board) : _board(board) {}

const vector<int> *GobangBase::board() const
{
  return _board->board();
}

bool GobangBase::_victory(Coord coord) const
{
  // vertical connection
  if(_victory_dir(coord, {1, 0}))
    return true;

  // horizontal connection
  if(_victory_dir(coord, {0, 1}))
    return true;

  // positive diagonal connection
  if(_victory_dir(coord, {1, 1}))
    return true;

  // negative diagonal connection
  if(_victory_dir(coord, {-1, 1}))
    return true;

  return false;
}

bool GobangBase::_victory_dir(Coord coord, Direction dir) const
{
  int player = (*_board->board())[BoardBase::coord_trans(coord)];

  Coord front_coord = coord - dir;
  bool connec = true;
  while(connec)
  {
    if(_board->valid_coord(front_coord) &&
       (*_board->board())[_board->coord_trans(front_coord)] == player)
      front_coord = front_coord - dir;
    else
      connec = false;
  }

  Coord back_coord = coord + dir;
  connec = true;
  while(connec)
  {
    if(_board->valid_coord(back_coord) &&
       (*_board->board())[_board->coord_trans(back_coord)] == player)
      back_coord = back_coord + dir;
    else
      connec = false;
  }

  return back_coord.y - front_coord.y - 1 >= 5 ||
         back_coord.x - front_coord.x - 1 >= 5;
}

// GobangSimulate

GobangSimulate::GobangSimulate() : GobangBase(nullptr)
{
  auto board = new SimulateBoard;
  GobangBase::_board = _board = board;
}

const vector<int> *GobangSimulate::board() const
{
  return _board->board();
}

bool GobangSimulate::place_chess(Coord coord)
{
  _board->occupy(_player, coord);
  return _victory(coord);
}

void GobangSimulate::set(BoardBase *board)
{
  _board->set(board);
  _player = _board->step() % 2 ? -1 : 1;
}

vector<unsigned> GobangSimulate::actions() const
{
  return _board->actions();
}

GobangSimulate::~GobangSimulate()
{
  delete _board;
}

// Gobang
Gobang::Gobang() : GobangBase(nullptr)
{
  GobangBase::_board = _board = ActualBoard::get();
}

const vector<int> *Gobang::board() const
{
  return _board->board();
}

Gobang::~Gobang()
{
  delete _board;
}

Gobang *Gobang::get()
{
  static auto gobang = Gobang();
  return &gobang;
}

void Gobang::play(int code)
{
  QString content;
  // code 0 means new game, 1 means somebody win, 2 means tie
  switch(code)
  {
    case 0:
      break;

    case 1:
    {
      int winner = -_player;
      if(winner == 1)
      {
        if(_AI_black)
          content = "Black win!\n";
        else
          content = "You win!\n";
      }
      else
      {
        if(_AI_white)
          content = "White win!\n";
        else
          content = "You win!\n";
      }
      break;
    }

    case 2:
      content = "Game tie!\n";
      break;
  }
  content += "Player vs. AI or AI vs. AI?";

  int mode = 0;
  // _window->showModeDialog(&mode, content);

  (_board)->init();
  _latest_black_move.x = _latest_black_move.y = -1;
  _latest_white_move.x = _latest_white_move.y = -1;

  _board_view->update();

  // mode 0 means AI vs. AI, 1 means player is black, 2 means player is white
  auto AI = make_shared<AIMCTS>("../../../model/model_500.pt");
  switch(mode)
  {
    case 0:
      setAI(AI, AI);
      break;

    case 1:
      setAI(nullptr, AI);
      break;

    case 2:
      setAI(AI, nullptr);
      break;
  }

  if(_AI_black)
  {
    _player_tern = false;
    QApplication::postEvent(_ai_controller, new PlaceEvent);
  }
  else
    _player_tern = true;
}

void Gobang::latest_move(Coord *black_move, Coord *white_move) const
{
  *black_move = _latest_black_move;
  *white_move = _latest_white_move;
}

bool Gobang::place_chess(Coord coord)
{
  if(coord.x == -1)
  {
    coord = _player == 1 ? _AI_black->select_point() : _AI_white->select_point();
    qDebug() << "AI place at " << coord.x << ", " << coord.y;
  }
  else
  {
    if(!BoardBase::valid_coord(coord) ||
          !_player_tern ||
          (*_board->board())[_board->coord_trans(coord)] != 0)
      return false;
    else
      (_AI_black ? _AI_black : _AI_white)->step(coord);
  }

  _board->occupy(_player, coord);

  if(_player == 1)
  {
    _latest_black_move = coord;
    if(_AI_white)
      _player_tern = false;
  }
  else
  {
    _latest_white_move = coord;
    if(_AI_black)
      _player_tern = false;
  }

  _player *= -1;

  _board_view->repaint();

  if(_victory(coord))
  {
    play(1); // somebody win
    return true;
  }
  else if(_board->step() == 15*15)
  {
    play(2); // tie game
    return true;
  }
  else
  {
    if(!_player_tern)
      QApplication::postEvent(_ai_controller, new PlaceEvent);  // it's AI's tern

    return false;
  }
}

void Gobang::setAI(std::shared_ptr<AIMCTS> AI1, std::shared_ptr<AIMCTS> AI2)
{
  _AI_black = AI1;
  _AI_white = AI2;
}

void Gobang::setWindow(MainWindow *window)
{
  _window = window;
}

void Gobang::setBoardView(BoardView *board_view)
{
  _board_view = board_view;
}

void Gobang::setAIController(AIController *controller)
{
  _ai_controller = controller;
}
