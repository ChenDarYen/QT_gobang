#ifndef MCTS_H
#define MCTS_H

#include "ai.h"
#include "gobang.h"
#include <string>
#include <tuple>
#include <vector>
#include <map>
#include <torch/script.h>

class Edge;
class AIMCTS;

class Node
{
  friend class AIMCTS;
  friend class Edge;

public:
  Node(Edge *edge_from, int player);
  void add_child(Coord coord, float prior);
  Node *get_child(Coord coord) const;
  void backup(float value);
  Coord choose_action() const;
  std::tuple<Coord, Node*, bool> policy() const;
  ~Node();

private:
  static bool _compare_coord(const Coord &c1, const Coord &c2);
  Edge *edge_from{nullptr};
  std::map<Coord, Edge*, decltype(_compare_coord)*> edges_away{_compare_coord};
  int player{0};
  int counter{0};
};

class Edge
{
  friend class AIMCTS;
  friend class Node;

public:
  Edge(Node *node_from, float priority);
  std::tuple<Node*, bool> get();
  void backup(float value);
  float ucb() const;
  ~Edge();

private:
  Node *node_from{nullptr};
  Node *node_away{nullptr};
  float prior{0};
  float value{0};
  int counter{0};
};

class AIMCTS : public AIBase
{
public:
  AIMCTS(const std::string &model_state_dest);
  void reset();
  void step(Coord action);
  void simulation();
  Coord select_point() override;
  ~AIMCTS();

private:
  torch::jit::IValue _trans_to_input(const std::vector<int> *board, int player) const;

  torch::jit::script::Module _model;
  Node *_curr_node{nullptr};
  GobangSimulate _simulate_game;
};

#endif // AIMCTS_H
