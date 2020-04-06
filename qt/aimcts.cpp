#include "aimcts.h"
#include <climits>
#include <cmath>
#include <QDebug>

using std::tuple;
using std::vector;
using std::get;

float C_PUCT = .1;
int SIMULATION_ITER = 400;

// Node

Node::Node(Edge *edge_from, int player): edge_from(edge_from),
                                         player(player) {}

void Node::add_child(Coord coord, float prior)
{
  edges_away[coord] = new Edge(this, prior);
}

Node *Node::get_child(Coord coord) const
{
  auto child = get<0>(edges_away.at(coord)->get());
  return child;
}

void Node::backup(float value)
{
  counter += 1;
  if(edge_from)
    edge_from->backup(value);
}

Coord Node::choose_action() const
{
  Coord choice;
  float max_tmp = INT_MIN;

  for(auto &[action, edge] : edges_away)
    if(edge->counter)
    {
      float tmp = edge->value / edge->counter;
      if(tmp > max_tmp)
      {
        max_tmp = tmp;
        choice = action;
      }
    }

  return choice;
}

tuple<Coord, Node*, bool> Node::policy() const
{
  float ucb_max = INT_MIN;
  Edge *choose_edge;
  Coord choose_action;

  for(auto &[action, edge] : edges_away)
  {
    float ucb = edge->ucb();
    if(ucb > ucb_max)
    {
      ucb_max = ucb;
      choose_edge = edge;
      choose_action = action;
    }
  }

  auto [choose_node, expand] = choose_edge->get();
  return {choose_action, choose_node, expand};
}

Node::~Node()
{
  for(auto [_, edge] : edges_away)
    delete edge;
}

bool Node::_compare_coord(const Coord &c1, const Coord &c2)
{
  if(c1.x == c2.x)
    return c1.y > c2.y;
  return c1.x > c2.x;
}

// Edge

Edge::Edge(Node *node_from, float prior) : node_from(node_from),
                                           prior(prior) {}

tuple<Node*, bool> Edge::get()
{
  if(node_away)
    return {node_away, false};
  else
  {
    node_away = new Node(this, -node_from->player);
    return {node_away, true};
  }
}

void Edge::backup(float value)
{
  this->value += value;
  ++counter;
  node_from->backup(-value);
}

float Edge::ucb() const
{
  float q = counter ? value / counter : 0;
  return q + C_PUCT * prior * sqrt(node_from->counter) / (counter+1);
}

Edge::~Edge()
{
  delete node_away;
}

// AIMCTS

AIMCTS::AIMCTS(const std::string &model_state_dest)
{
  _curr_node = new Node(nullptr, 1);
  _model = torch::jit::load(model_state_dest);
}

void AIMCTS::reset()
{
  delete _curr_node;
  _curr_node = new Node(nullptr, 1);
}

void AIMCTS::step(Coord action)
{
  auto node_ = _curr_node->get_child(action);
  node_->edge_from->node_away = nullptr;
  node_->edge_from = nullptr;
  delete  _curr_node;  // release memory
  _curr_node = node_;
}

void AIMCTS::simulation()
{
  for(int i = 0; i < SIMULATION_ITER; ++i)
  {
    _simulate_game.set(ActualBoard::get());
    auto node = _curr_node;
    auto *board = _simulate_game.board();
    bool expand = false, end = false;

    while(!expand && !end)
    {
      if(node->edges_away.size() == 0)
      {
        vector<torch::jit::IValue> input{_trans_to_input(board, node->player)};
        auto prob = torch::softmax(_model.forward(input).toTuple()->elements()[1].toTensor(), 1)
                    .data_ptr<float>();
        for(auto &&action_idx : _simulate_game.actions())
          node->add_child(Coord(action_idx%15, action_idx/15), prob[action_idx]);
      }
      auto p = node->policy();
      node = get<1>(p);
      expand = get<2>(p);;

      end = _simulate_game.place_chess(get<0>(p));
    }

    if(end)
      node->backup(1);
    else if(expand)
    {
      vector<torch::jit::IValue> input{_trans_to_input(board, node->player)};
      float value = *_model.forward(input).toTuple()->elements()[0].toTensor().data_ptr<float>();
      node->backup(value);
    }
  }
}

Coord AIMCTS::select_point()
{
  simulation();
  Coord action = _curr_node->choose_action();
  step(action);
  return action;
}

AIMCTS::~AIMCTS()
{
  delete  _curr_node;
}

torch::jit::IValue AIMCTS::_trans_to_input(const std::vector<int> *board, int player) const
{
  float features[3*15*15] = {};
  for(int i = 0; i < 225; ++i)
  {
    features[i] = ((*board)[i] == player);
    features[225+i] = ((*board)[i] == -player);
    features[450+i] = (player == 1);
  }

  auto input = torch::tensor(at::ArrayRef<float>(features)).view({1, 3, 15, 15});
  return input;
}
