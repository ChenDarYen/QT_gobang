#include <QString>
#include <QApplication>
#include "gobang.h"
#include "board.h"
#include "ai.h"
#include "aicontroller.h"
#include "direction.h"
#include "mainwindow.h"
#include "boardview.h"
#include "placeevent.h"

using std::vector;

Gobang::Gobang() : _board(new Actual_Board), _AI(new AI) {}

Gobang::~Gobang()
{
  delete _board;
  delete _AI;
}

Gobang *Gobang::get()
{
  static auto gobang = Gobang();
  return &gobang;
}

void Gobang::play(int winner)
{
  // winner is -1 means first game, 0 means player, 1 means AI, 2 means tie
  QString content;
  if(winner == -1)
    content = "You want to go first or second?";
  else if(winner == 1)
    content = "AI win!\nYou want to go first or second?";
  else if(winner == 0)
    content = "You win!\nYou want to go first or second?";
  else
    content = "Game tie!\nYou want to go first or second?";

  int mode = 0;
  _window->showModeDialog(&mode, content);

  _board->init();
  _board_view->update();

  _computer = mode == 0 ? 1 : 0;

  if(mode == 0)
    _player_tern = true;
  else
    QApplication::postEvent(_ai_controller, new PlaceEvent);
}

const vector<int> *Gobang::board() const
{
  return _board->board();
}

void Gobang::place_chess(bool is_AI, Coord coord)
{
  if(is_AI)
  {
    coord = _AI->select_point(_board);
    _player_tern = true;
  }
  else
  {
    if(!_player_tern || //  check if it's player's tern
       !_board->valid_coord(coord) || // check if place int this coordinate is legal
       (*_board->board())[_board->coord_trans(coord)] != -1)
      return;
    _player_tern = false;
  }

  _board->occupy(is_AI ? _computer : !_computer, coord);
  _board_view->repaint();

  if(_victory(coord))
    play(is_AI); // somebody win
  else if(_board->step() == 361)
    play(2); // tie game
  else if(!is_AI)
    QApplication::postEvent(_ai_controller, new PlaceEvent); // the game is not fished, it's AI's tern
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

bool Gobang::_victory(Coord coord) const
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

bool Gobang::_victory_dir(Coord coord, Direction dir) const
{
  int player = (_board->step() - 1) % 2;

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
