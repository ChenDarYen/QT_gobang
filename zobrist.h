#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <vector>
#include <cstdint>

struct Coord;

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

#endif // ZOBRIST_H
