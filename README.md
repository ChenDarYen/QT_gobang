# QT_gobang
下載：(.dmg)
---
[v1.2](https://github.com/ChenDarYen/QT_gobang/releases/download/v1.2/gobang1.2.dmg) / 
[v1.1](https://github.com/ChenDarYen/QT_gobang/releases/download/v1.1/gobang1.1.dmg) / 
[v1.0](https://github.com/ChenDarYen/QT_gobang/releases/download/v1.0/gobang.dmg)

簡介
---
在足夠好的 order 之下使用 Nega Scout Search 的五子棋 AI 程式。  
設計算殺搭配 Zobrist hashing table 在可接受的時間內完成六層全局搜尋加上六層算殺，提升棋力。  
使用 QT 做 GUI 開發，達成簡易的 MVC 模式。

版本
---
### v1.2

> 使用 Zobrist hashing table 避免重複搜尋，提升效率。  
> 提升效率後，便能於合理時間內加入算殺，大幅提升搜尋深度。
  
Zobrist hashing 使用 _self 和 _opponent 兩個 vector 各自儲存棋盤大小個隨機數，最好能是 64bit 以避免碰撞，這裡只使用了 32bit。_key 同樣初始唯一隨機數。  
update 函數內使用 XOR 能使 _key 只與盤面相關，和落子順序無關。  

當搜尋結束時局面仍未定，進行更深度的算殺，只考慮能形成比相等或更強於活三落點。  
一共進行兩次，一次為自己尋找，一次提防對手。

###### zobrist.h
```c++
class Zobrist
{
public:
  Zobrist();
  void update(int is_self, Coord coord);

  inline int key() const { return _key; }
  std::vector<uint32_t> _self;
  std::vector<uint32_t> _opponent;

private:
  uint32_t _key;
};
```
###### zobrist.cpp
```c++
using boost::mt19937;
using boost::uniform_int;
using boost::variate_generator;

Zobrist::Zobrist()
{
  mt19937 generator;
  uniform_int<uint32_t>dist(1, 0xffffffff);
  variate_generator<mt19937&, uniform_int<uint32_t>> random(generator, dist);

  for(int i = 0; i < 361; ++i)
  {
    _self.push_back(random());
    _opponent.push_back(random());
  }

  _key = random();
}

void Zobrist::update(int is_self, Coord coord)
{
  if(Board_Base::valid_coord(coord))
    _key ^= (is_self ? _self[Board_Base::coord_trans(coord)] :
                       _opponent[Board_Base::coord_trans(coord)]);
}
```
更新 nega scout search：
+ 只儲存勝負已訂的盤面，因為勝負未定的盤面分數會隨搜尋深度而不能預期。  
+ 搜尋時先確認當前盤面是否已被搜尋過且勝負已定。  
+ 每變更一步更新 _key 一次。
+ 在深度為 _max_depth 和 _max_depth - 1 時如勝負未定，分別進行一次算殺。

###### ai.cpp
```c++
int AI::_nega_scout(Test_Board *board, int alpha, int beta, unsigned depth, Coord coord)
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
    board->occupy(_player(board), a.coord);
    _zobrist->update(is_self, a.coord);

    int w = -_nega_scout(board, -b, -alpha, depth + 1, a.coord);

    /*
     * we need to check if the search with a narrow window prunes to much
     * if it realy does, search with the regular window again
     *
     * when depth is _max_depth or _max_depth - 1
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
```

### v1.1

> 介面上標示最後落子的位置，提高操作方便性。  
> 記憶 heuristic ，棋盤發生變更時只做微量的更新，不再需要重新全局計算。  
> 改善 heuristic 實作，增加 pruning 的數量。

_critical 函數用於判斷在一方向上連接或阻斷的 heuristic。  
使用數字代表棋型，在利用函數 _shape_h 得到分數。
###### tastboard.cpp
```c++
bool Test_Board::occupy(int player, Coord coord)
{
  if(!Simple_Board::occupy(player, coord))
    return false;

  _position_h[coord_trans(coord)] = 0;
  _update_position_h(coord);

  return true;
}

void Test_Board::remove(Coord coord)
{
  if(valid_coord(coord))
    if (occupied[coord_trans(coord)] != -1)
    {
      occupied[coord_trans(coord)] = -1;
      _position_h[coord_trans(coord)] = _coord_heuristic(coord);
      _update_position_h(coord);
      --step_count;
    }
}

int Test_Board::_critical(Coord coord, Direction dir, bool connec) const
{
  int target = !connec ? _owner : !_owner;

  int i = 0, chance = 2;
  int shape_front = 0;
  Coord stunt_coord = coord - dir;
  // front of direction
  while(i < 4 && chance)
  {
    if(!valid_coord(stunt_coord) ||
       (occupied[coord_trans(stunt_coord)] == target))
    {
      shape_front += pow(10, i); // 1 means block
      break;
    }
    else if(occupied[coord_trans(stunt_coord)] == -1)
    {
      shape_front += pow(10, i) * 2; // 2 means blank
      --chance;
    }

    ++i;
    stunt_coord = stunt_coord - dir;
  }

  if(shape_front == 0) // already form a 活四 or 沖四
    return 10000;

  // back of direction
  i = 0;
  chance = 2;
  stunt_coord = coord + dir;
  int shape_back = 3; // 3 means the coord's position
  while(i < 4 && chance)
  {
    shape_back *= 10;

    if(!valid_coord(stunt_coord) ||
       (occupied[coord_trans(stunt_coord)] == target))
    {
      shape_back += 1;
      break;
    }
    else if(occupied[coord_trans(stunt_coord)] == -1)
    {
      shape_back += 2;
      --chance;
    }

    ++i;
    stunt_coord = stunt_coord + dir;
  }

  if(shape_back == 30000)
    return 10000;

  int shape = shape_front * pow(10, ceil(log10(shape_back))) + shape_back;

  return _shape_h(shape);
}
```
