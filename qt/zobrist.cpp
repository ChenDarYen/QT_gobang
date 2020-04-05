#include <boost/random.hpp>
#include <climits>
#include "zobrist.h"
#include "board.h"

using boost::mt19937;
using boost::uniform_int;
using boost::variate_generator;

Zobrist::Zobrist()
{
  mt19937 generator;
  uniform_int<uint32_t>dist(1, 0xffffffff);
  variate_generator<mt19937&, uniform_int<uint32_t>> random(generator, dist);

  for(int i = 0; i < 225; ++i)
  {
    _self.push_back(random());
    _opponent.push_back(random());
  }

  _key = random();
}

void Zobrist::update(int is_self, Coord coord)
{
  if(BoardBase::valid_coord(coord))
    _key ^= (is_self ? _self[BoardBase::coord_trans(coord)] :
                       _opponent[BoardBase::coord_trans(coord)]);
}

