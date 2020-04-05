#include "ai.h"
#include "direction.h"
#include "zobrist.h"
#include "gobang.h"
#include "testboard.h"
#include <string>
#include <cmath>

using std::shared_ptr;
using std::make_shared;
using std::vector;
using std::string;
using std::max;
using std::min;

const int INF = 10000;
const int WINNING_POINT = 9000;

Action::Action(int p, Coord coord) : priority(p), coord(coord) {}

AINegaScout::AINegaScout(unsigned depth) : AIBase(),
                                           _max_depth(depth),
                                           _killer_depth(depth + 6),
                                           _player_shapes(3, vector<int>(2, 0)),
                                           _opponent_shapes(2, vector<int>(2, 0)),
                                           _zobrist(new Zobrist) {}

AINegaScout::~AINegaScout()
{
  delete _zobrist;
}

Coord AINegaScout::select_point()
{
  // creat a test board by actual board
  TestBoard test_board(_latest_player(ActualBoard::get()), *ActualBoard::get());

  // update key of Zobrist hashing
  Coord latest_self_move, latest_opponent_move;
  Gobang::get()->latest_move(&latest_self_move, &latest_opponent_move);
  _zobrist->update(true, latest_self_move);
  _zobrist->update(false, latest_opponent_move);

  // search a point
  _nega_scout(&test_board, -INF, INF, 1);
  return _selection;
}

int AINegaScout::_nega_scout(TestBoard *board, int alpha, int beta, unsigned depth, Coord coord)
{
  if(depth > 1)// in depth 1, check repeat and terminal test is not necessary
  {
    if(auto p = _cache.find(_zobrist->key());
       p != _cache.end())
    {
      auto [score, steps] = (*p).second;
      return score > 0 ? score + (board->step() - depth - steps + 1) :
                         score - (board->step() - depth - steps + 1);
    }

    else if(_terminal_test(board, coord))
      return -INF + depth; // consider the urgentness
  }

  if(depth > _max_depth)
    return -_heuristic(board, depth);

  _breadth = 40 * ((_max_depth - depth + 1) / static_cast<double>(_max_depth));

  Coord selec;
  int v = -INF;
  int b = beta;
  bool is_self = depth % 2;
  for(auto &&a : _actions(board))
  {
    // update key of Zobrist hashing when occupy or remove
    board->occupy(_latest_player(board), a.coord);
    _zobrist->update(is_self, a.coord);

    int w = -_nega_scout(board, -b, -alpha, depth + 1, a.coord);

    /*
     * we need to check if the search with a narrow window prunes to much
     * if it realy does, search with the regular window again
     *
     * when depth is _max_depth
     * and the game is still has suspence, try to search a killer
    */
    if(depth != _max_depth)
    {
      if(alpha < w && w < beta &&
         beta != b)
        w = -_nega_scout(board, -beta, -alpha, depth + 1, a.coord);
    }
    else if(abs(w) < WINNING_POINT &&
            alpha < -WINNING_POINT && beta > -WINNING_POINT)
    {
      if(int w_killer = -_nega_scout_killer(board, -beta, -alpha, depth + 1, a.coord);
         w_killer < -WINNING_POINT)
        w = w_killer;
    }

    if(depth == _max_depth - 1 && abs(w) < WINNING_POINT &&
       alpha < -WINNING_POINT && beta > -WINNING_POINT)
      if(int w_opponent_killer = -_nega_scout_killer(board, -beta, -alpha, depth + 1, a.coord);
         w_opponent_killer < -WINNING_POINT)
        w = w_opponent_killer; // consider the urgentness

    // only save nonsuspence situations
    if(abs(w) >= WINNING_POINT)
      _cache[_zobrist->key()] = {-w, board->step() - depth}; // need to take negative again

    board->remove(a.coord);
    _zobrist->update(is_self, a.coord);

    if(w > v)
    {
      v = w;
      selec = a.coord;
    }

    /*
     * if v is greater or equal to 9000
     * it means this move lead winnimg
     * since it's not necessary to check others moves, prune it!
    */
    if(v >= beta || (depth == 1 && v >= 9000))
      break;

    alpha = max(alpha, v);
    b = alpha + 1;
  }

  if(depth == 1)
    _selection = selec;

  return v;
}

int AINegaScout::_nega_scout_killer(TestBoard *board, int alpha, int beta, unsigned depth, Coord coord)
{
  if(auto p = _cache.find(_zobrist->key());
     p != _cache.end())
  {
    auto [score, steps] = (*p).second;
    return score > 0 ? score + (board->step() - depth - steps + 1) :
                       score - (board->step() - depth - steps + 1);
  }

  if(_terminal_test(board, coord))
    return -10000 + depth;

  if(depth > _killer_depth)
    return -_heuristic(board, depth);

  int v = -INF;
  for(auto &&a : _actions_killer(board))
  {
    board->occupy(_latest_player(board), a.coord);
    _zobrist->update(depth % 2, a.coord);

    int w = -_nega_scout_killer(board, -beta, -alpha, depth + 1, a.coord);

    if(abs(w) >= WINNING_POINT)
      _cache[_zobrist->key()] = {-w, board->step() - depth};

    board->remove(a.coord);
    _zobrist->update(depth % 2, a.coord);

    if(w > v)
      v = w;

    if(v >= beta || v >= 9000)
      break;

    alpha = max(alpha, v);
  }

  return v != -INF ? v : 0; // in killer search 0 is harmless
}

bool AINegaScout::_terminal_test(TestBoard *board, Coord coord) const
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

bool AINegaScout::_terminal_test_dir(TestBoard *board, Coord coord, Direction dir) const
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

vector<Action> AINegaScout::_actions(TestBoard *board) const
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
  for(int x = 1; x <= 15; ++x)
    for(int y = 1; y <= 15; ++y)
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

vector<Action> AINegaScout::_actions_killer(TestBoard *board) const
{
  vector<Action> actions;

  /*
   * in killer search we only consider
   * those points forming chess shape stronger or equal to 活三
   * whoose heuristic is at least 400
  */
  for(int x = 1; x <= 15; ++x)
    for(int y = 1; y <= 15; ++y)
      if(int p = board->get_heuristic({x, y}); p >= 400)
        actions.emplace_back(Action(p, {x, y}));

  std::sort(actions.begin(), actions.end(),
            [](const Action &lhs, const Action &rhs)
            {
              return lhs.priority > rhs.priority;
            });
  return actions;
}

int AINegaScout::_heuristic(TestBoard *board, unsigned depth)
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
  for(int y = 1; y <= 15; ++y)
  {
    blank_preifix = 0;
    player_prefix = opponent_prefix = true;
    for(int x = 1; x <= 15 - 1; ++x)
    {
      int h = _analysis_shape(board, {x, y}, {1, 0}, &blank_preifix, &player_prefix, &opponent_prefix);
      if(h)
        return -INF + depth + 1;
    }
  }

  // analysis chess shape in each vertical lines
  for(int x = 1; x <= 15; ++x)
  {
    blank_preifix = 0;
    player_prefix = opponent_prefix = true;
    for(int y = 1; y <= 15 - 1; ++y)
    {
      int h = _analysis_shape(board, {x, y}, {0, 1}, &blank_preifix, &player_prefix, &opponent_prefix);
      if(h)
        return -INF + depth + 1;
    }
  }

  // analysis chess shape in each positive diagonal lines
  for(int i = 1; i <= 15 * 2 - 1; ++i)
  {
    blank_preifix = 0;
    player_prefix = opponent_prefix = true;
    for(int j = 1; j <= (i <= 15 ? 15 - i : 2 * 15 - i - 1); ++j)
    {
      int x = 0, y = 0;
      if(i <= 15)
      {
        x = i + j - 1;
        y = j;
      }
      else
      {
        x = j;
        y = i - 15 + j;
      }
      int h = _analysis_shape(board, {x, y}, {1, 1}, &blank_preifix, &player_prefix, &opponent_prefix);
      if(h)
        return -INF + depth + 1;
    }
  }

  // analysis chess shape in each negative diagonal lines
  for(int i = 1; i <= 15 * 2 - 1; ++i)
  {
    blank_preifix = 0;
    player_prefix = opponent_prefix = true;
    for(int j = 1; j <= (i <= 15 ? 15 - i : 2 * 15 - i - 1); ++j)
    {
      int x = 0, y = 0;
      if(i <= 15)
      {
        x = 15 - i - j + 2;
        y = j;
      }
      else
      {
        x = 15 - j + 1;
        y = i - 15 + j;
      }
      int h = _analysis_shape(board, {x, y}, {-1, 1}, &blank_preifix, &player_prefix, &opponent_prefix);
      if(h)
        return -INF + depth + 1;
    }
  }

  // consider all chess shapes on the board
  // player has 活四 or has 沖四 more then one
  if(_player_shapes[2][1] || _player_shapes[2][0] > 1)
    return INF - depth - 2;
  // player has 沖四 and 活三
  if(_player_shapes[2][0] && _player_shapes[1][1])
    return INF - depth - 4;
  // player do not have 沖四, but opponent has 活三
  if(!_player_shapes[2][0] && _opponent_shapes[1][1])
    return -INF + depth + 3;
  // player has 活三 more then one, and opponent do not have 活三 or 眠三
  if(_player_shapes[1][1] > 1 && !_opponent_shapes[1][0] && !_opponent_shapes[1][1])
    return INF - depth - 4;

  int h = 0;
  if(_player_shapes[2][0]) // 沖四
    h += 2000;

  if(_player_shapes[1][1] > 1) // double 活三
    h += 5000;
  else if(_player_shapes[1][1]) // single 活三
    h += 1000;

  if(_opponent_shapes[1][1] > 1)
    h -= 5000;
  else if(_opponent_shapes[1][1])
    h -= 2500;

  h += (_player_shapes[1][0] - _opponent_shapes[1][0]) * 100; // 眠三
  h += (_player_shapes[0][1] - _opponent_shapes[0][1]) * 80; // 活二
  h += (_player_shapes[0][0] - _opponent_shapes[0][0]) * 2; // 眠二

  return h;
}

bool AINegaScout::_analysis_shape(TestBoard *board, Coord coord, Direction dir,
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
        return true;

      if(shape.amount > 1)
        ++_opponent_shapes[shape.amount - 2][shape.alive];
    }
    *blank_prefix = 0;
    *player_prefix = false;
    *opponent_prefix = true;
  }

  return 0; // return 0 means the heuristic is not already be determined
}

Chess_Shape AINegaScout::_analysis_shape_line(TestBoard *board, Coord coord, Direction dir, int blank_prefix) const
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

