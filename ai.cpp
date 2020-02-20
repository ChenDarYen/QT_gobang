#include "ai.h"
#include "direction.h"
#include <string>
#include <cmath>

using std::shared_ptr;
using std::make_shared;
using std::vector;
using std::string;
using std::max;
using std::min;
int INF = 9998;

Action::Action(int p, Coord coord) : priority(p), coord(coord) {}

AI::AI(unsigned depth) : _max_depth(depth),
                         _player_shapes(3, vector<int>(2, 0)),
                         _opponent_shapes(2, vector<int>(2, 0)) {}

Coord AI::select_point(Actual_Board *board)
{
  Test_Board test_board(_player(board), *board); // creat a test board by actual board
  _nega_scout(&test_board, -INF, INF, 1);
  return _selection;
}

int AI::_nega_scout(Test_Board *board, int alpha, int beta, unsigned depth, Coord coord)
{
  if(depth > 1 && _terminal_test(board, coord)) // in depth 1, terminal test is not necessary
    return -10000 + depth; // consider the urgentness

  if(depth > _max_depth)
    return -_heuristic(board);

  _breadth = 40 * ((_max_depth - depth + 1) / static_cast<double>(_max_depth));

  Coord selec;
  int v = -INF;
  int b = beta;
  for(auto &&a : _actions(board))
  {
    board->occupy(_player(board), a.coord);

    int w = -_nega_scout(board, -b, -alpha, depth + 1, a.coord);
    if(alpha < w && w < beta && beta != b)
      w = -_nega_scout(board, -beta, -alpha, depth + 1, a.coord);

    board->remove(a.coord);

    if(w > v)
    {
      v = w;
      selec = a.coord;
    }

    /*
     * beta is initialized by INF = 9998 in depth 1
     * so if AI find an act lead to terminal, the pruning occur
     * since exis no better move
    */
    if(v >= beta)
      break;

    alpha = max(alpha, v);
    b = alpha + 1;
  }

  if(depth == 1)
    _selection = selec;

  return v;
}

bool AI::_terminal_test(Test_Board *board, Coord coord) const
{
  // vertical connection
  if(_terminal_test_dir(board, coord, {1, 0}))
    return true;

  // horizontal connection
  if(_terminal_test_dir(board, coord, {0, 1}))
    return true;

  // positive diagonal connection
  if(_terminal_test_dir(board, coord, {1, 1}))
    return true;

  // negative diagonal connection
  if(_terminal_test_dir(board, coord, {-1, 1}))
    return true;

  return false;
}

bool AI::_terminal_test_dir(Test_Board *board, Coord coord, Direction dir) const
{
  int player = (board->step() - 1) % 2;

  Coord front_coord = coord - dir;
  bool connec = true;
  while(connec)
  {
    if(board->valid_coord(front_coord) &&
       (*board->board())[board->coord_trans(front_coord)] == player)
      front_coord = front_coord - dir;
    else
      connec = false;
  }

  Coord back_coord = coord + dir;
  connec = true;
  while(connec)
  {
    if(board->valid_coord(back_coord) &&
       (*board->board())[board->coord_trans(back_coord)] == player)
      back_coord = back_coord + dir;
    else
      connec = false;
  }

  return back_coord.y - front_coord.y - 1 >= 5 ||
         back_coord.x - front_coord.x - 1 >= 5;
}

vector<Action> AI::_actions(Test_Board *board) const
{
  vector<Action> actions;

  // when board is empty
  if(!board->step())
  {
    Action act;
    act.coord = {10, 10};
    actions.push_back(act);
    return actions;
  }

  // get valuable actions
  for(int x = 1; x <= 19; ++x)
    for(int y = 1; y <= 19; ++y)
      if(int p = board->get_heuristic({x, y}))
        actions.emplace_back(Action(p, {x, y}));

  // sort actions by priority
  std::sort(actions.begin(), actions.end(),
            [](const Action &lhs, const Action &rhs)
            {
              return lhs.priority > rhs.priority;
            });

  // limit the amount of actions
  if(actions.size() > _breadth)
    actions.resize(_breadth);
  return actions;
}

int AI::_heuristic(Test_Board *board)
{
  // init data of chess shape
  for(int i = 0; i < 3; ++i)
    for(int j = 0; j < 2; ++j)
      _player_shapes[i][j] = 0;

  for(int i = 0; i < 2; ++i)
    for(int j = 0; j < 2; ++j)
      _opponent_shapes[i][j] = 0;

  int blank_preifix;
  bool player_prefix, opponent_prefix;

  // analysis chess shape in each horizontal lines
  for(int y = 1; y <= 19; ++y)
  {
    blank_preifix = 0;
    player_prefix = opponent_prefix = true;
    for(int x = 1; x <= 19 - 1; ++x)
    {
      int h = _analysis_shape(board, {x, y}, {1, 0}, &blank_preifix, &player_prefix, &opponent_prefix);
      if(h)
        return h;
    }
  }

  // analysis chess shape in each vertical lines
  for(int x = 1; x <= 19; ++x)
  {
    blank_preifix = 0;
    player_prefix = opponent_prefix = true;
    for(int y = 1; y <= 19 - 1; ++y)
    {
      int h = _analysis_shape(board, {x, y}, {0, 1}, &blank_preifix, &player_prefix, &opponent_prefix);
      if(h)
        return h;
    }
  }

  // analysis chess shape in each positive diagonal lines
  for(int i = 1; i <= 19 * 2 - 1; ++i)
  {
    blank_preifix = 0;
    player_prefix = opponent_prefix = true;
    for(int j = 1; j <= (i <= 19 ? 19 - i : 2 * 19 - i - 1); ++j)
    {
      int x = 0, y = 0;
      if(i <= 19)
      {
        x = i + j - 1;
        y = j;
      }
      else
      {
        x = j;
        y = i - 19 + j;
      }
      int h = _analysis_shape(board, {x, y}, {1, 1}, &blank_preifix, &player_prefix, &opponent_prefix);
      if(h)
        return h;
    }
  }

  // analysis chess shape in each negative diagonal lines
  for(int i = 1; i <= 19 * 2 - 1; ++i)
  {
    blank_preifix = 0;
    player_prefix = opponent_prefix = true;
    for(int j = 1; j <= (i <= 19 ? 19 - i : 2 * 19 - i - 1); ++j)
    {
      int x = 0, y = 0;
      if(i <= 19)
      {
        x = 19 - i - j + 2;
        y = j;
      }
      else
      {
        x = 19 - j + 1;
        y = i - 19 + j;
      }
      int h = _analysis_shape(board, {x, y}, {-1, 1}, &blank_preifix, &player_prefix, &opponent_prefix);
      if(h)
        return h;
    }
  }

  // consider all chess shapes on the board
  // player has 活四 or has 沖四 more then one
  if(_player_shapes[2][1] || _player_shapes[2][0] > 1)
    return 9030;
  // player has 沖四 and 活三
  if(_player_shapes[2][0] && _player_shapes[1][1])
    return 9020;
  // player do not have 沖四, but opponent has 活三
  if(!_player_shapes[2][0] && _opponent_shapes[1][1])
    return -9010;
  // player has 活三 more then one, and opponent do not have 活三 or 眠三
  if(_player_shapes[1][1] > 1 && !_opponent_shapes[1][0] && !_opponent_shapes[1][1])
    return 9000;

  int h = 0;
  if(_player_shapes[2][0]) // 沖四
    h += 2000;

  if(_player_shapes[1][1] > 1) // double 活三
    h += 5000;
  else if(_player_shapes[1][1]) // single 活三
    h += 100;

  if(_opponent_shapes[1][1] > 1)
    h -= 5000;
  else if(_opponent_shapes[1][1])
    h -= 2500;

  h += (_player_shapes[1][0] - _opponent_shapes[1][0]) * 10; // 眠三
  h += (_player_shapes[0][1] - _opponent_shapes[0][1]) * 4; // 活二
  h += (_player_shapes[0][0] - _opponent_shapes[0][0]) * 2; // 眠二

  return h;
}

int AI::_analysis_shape(Test_Board *board, Coord coord, Direction dir,
                         int *blank_prefix, bool *player_prefix, bool *opponent_prefix)
{
  int player = (board->step() - 1) % 2;

  int owner = (*board->board())[board->coord_trans({coord})];
  if(owner == -1) // coodinate is a blank
  {
    ++*blank_prefix;
    *player_prefix = *opponent_prefix = true;
  }
  else if(owner == player) // coordinate is occupied by the player
  {
    if(*opponent_prefix) // in order to avoid analysising repeatedly
    {
      Chess_Shape shape = _analysis_shape_line(board, coord, dir, *blank_prefix);
      if(shape.amount > 1)
        ++_player_shapes[shape.amount - 2][shape.alive];
    }
    *blank_prefix = 0;
    *player_prefix = true;
    *opponent_prefix = false;
  }
  else // coordinate was occupied by the opponent
  {
    if(*player_prefix)
    {
      Chess_Shape shape = _analysis_shape_line(board, coord, dir, *blank_prefix);

      if(shape.amount == 4) // if opponent has 活四 or 沖四, the player must loose
        return shape.alive ? -9050 : -9040;

      if(shape.amount > 1)
        ++_opponent_shapes[shape.amount - 2][shape.alive];
    }
    *blank_prefix = 0;
    *player_prefix = false;
    *opponent_prefix = true;
  }

  return 0; // return 0 means the heuristic is not already be determined
}

Chess_Shape AI::_analysis_shape_line(Test_Board *board, Coord coord, Direction dir, int blank_prefix) const
{
  int player = (*board->board())[board->coord_trans(coord)];
  int opponent = player == 1 ? 0 : 1;

  int shape_int = 1;
  int i = 2;
  bool blocked = false;
  coord = coord + dir;
  while(i++ <= 5 && !blocked)
  {
    if(!board->valid_coord(coord) ||
       (*board->board())[board->coord_trans(coord)] == opponent)
      blocked = true;
    else
    {
      shape_int = shape_int << 1;
      shape_int += (*board->board())[board->coord_trans(coord)] == player ? 1 : 0;
    }

    coord = coord + dir;
  }

  Chess_Shape shape;
  switch(shape_int)
  {
    case 0b10001:
      shape.amount = 2;
      shape.alive = false;
      break;

    case 0b11:
      if(blank_prefix >= 3)
      {
        shape.amount = 2;
        shape.alive = false;
      }
      break;

    case 0b110:
      if(blank_prefix >= 2)
      {
        shape.amount = 2;
        shape.alive = true;
      }
      break;

    case 0b1100:
      if(blank_prefix > 0)
      {
        shape.amount = 2;
        shape.alive = blank_prefix >= 2 ? true : false;
      }
      break;

    case 0b11000:
      shape.amount = 2;
      shape.alive = blank_prefix > 0 ? true : false;
      break;

    case 0b101:
      if(blank_prefix >= 2)
      {
        shape.amount = 2;
        shape.alive = false;
      }
      break;

    case 0b1010:
      if(blank_prefix > 0)
      {
        shape.amount = 2;
        shape.alive = true;
      }
      break;

    case 0b10100:
      shape.amount = 2;
      shape.alive = blank_prefix > 0 ? true : false;
      break;

    case 0b1001:
      if(blank_prefix > 0)
      {
        shape.amount = 2;
        shape.alive = false;
      }
      break;

    case 0b10010:
      shape.amount = 2;
      shape.alive = blank_prefix > 0 ? true : false;
      break;

    case 0b111:
      if(blank_prefix >= 2)
      {
        shape.amount = 3;
        shape.alive = false;
      }
      break;

    case 0b1110:
      if(blank_prefix > 0)
      {
        shape.amount = 3;
        shape.alive = blank_prefix >= 2 ? true : false;
      }
      break;

    case 0b11100:
      shape.amount = 3;
      shape.alive = blank_prefix > 0 ? true : false;
      break;

    case 0b1101: case 0b1011:
      if(blank_prefix > 0)
      {
        shape.amount = 3;
        shape.alive = false;
      }
      break;

    case 0b11010: case 0b10110:
      shape.amount = 3;
      shape.alive = blank_prefix > 0 ? true : false;
      break;

    case 0b11001: case 0b10101:
      shape.amount = 3;
      shape.alive = false;
      break;

    case 0b1111:
      if(blank_prefix > 0)
      {
        shape.amount = 4;
        shape.alive = false;
      }
      break;

    case 0b11110:
      shape.amount = 4;
      shape.alive = blank_prefix > 0 ? true : false;
      break;

    case 0b11101: case 0b11011:
      shape.amount = 4;
      shape.alive = false;
      break;

    default:
      break;
  }

  return shape;
}

