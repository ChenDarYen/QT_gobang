import numpy as np
import game
import utils
import sys
import os

C_PUCT = .1
SIMULATION_ITER = 400


class Node:
    def __init__(self, edge, player):
        self.edge_from = edge
        self.player = player
        self.edges_away = {}
        self.counter = 0

    def add_child(self, action, priority):
        self.edges_away[action] = Edge(self, action, priority)

    def get_child(self, action):
        child, _ = self.edges_away[action].get()
        return child

    def backup(self, value):
        self.counter += 1
        if self.edge_from:
            self.edge_from.backup(value)

    def distribution(self, train=True):
        dist = np.zeros((15, 15))
        actions_pool = []
        actions_dist = []
        for edge in self.edges_away.values():
            if edge.counter != 0:
                dist[edge.action[0], edge.action[1]] = edge.counter
                actions_pool.append(edge.action)
                actions_dist.append(edge.counter)

        denominator = np.sum(dist)
        dist /= denominator

        actions_dist = [value/denominator for value in actions_dist]

        if train:
            np.random.seed(int.from_bytes(os.urandom(4), byteorder='little'))
            # explore under linear combination of dirichlet distribution
            length = len(actions_pool)
            p = .8*np.array(actions_dist) + .2*np.random.dirichlet(.2*np.ones(length))
            action_idx = np.random.choice(length, p=p)
            action = actions_pool[action_idx]
        else:
            action = actions_pool[int(np.argmax(actions_dist))]

        return action, dist

    def policy(self):  # choose action under UCB
        ucb_max = -sys.maxsize
        choose_edge = None
        for edge in self.edges_away.values():
            if edge.ucb() > ucb_max:
                ucb_max = edge.ucb()
                choose_edge = edge
        choose_node, expand = choose_edge.get()
        return choose_edge.action, choose_node, expand


class Edge:
    # let action be a tuple, so we can use it on key of dictionary
    def __init__(self, node_from, action: tuple, priority):
        self.node_from = node_from
        self.action = action
        self.priority = priority
        self.counter = 0
        self.value = 0.0
        self.node_to = None

    def get(self):
        self.counter += 1
        if self.node_to:
            return self.node_to, False
        else:
            self.node_to = Node(self, -self.node_from.player)
            return self.node_to, True

    def backup(self, value):
        self.value += value
        self.node_from.backup(-value)

    def ucb(self):
        q = self.value/self.counter if self.counter else 0
        return q + C_PUCT * self.priority * np.sqrt(self.node_from.counter) / (self.counter+1)


class MCTS:
    def __init__(self, neural_network, index):
        self.curr_node = Node(None, 1)
        self.nn = neural_network
        self.main_game, self.simulate_game = game.Gobang(), game.Gobang()
        self.index = index

    def renew(self):
        self.curr_node = Node(None, 1)
        self.main_game.new_game()

    def step(self, action):
        node_ = self.curr_node.get_child(action)
        node_.edge_from = None  # it's no need, release memory
        return node_

    def simulation(self):
        for _ in range(SIMULATION_ITER):
            self.simulate_game.set(boarder_board=self.main_game.curr_board(True),
                                   curr_player=self.main_game.curr_player,
                                   steps=self.main_game.steps)

            node = self.curr_node
            state = self.simulate_game.curr_board()
            expand, end = False, False
            while not end and not expand:
                if len(node.edges_away) == 0:
                    _, state_prob = self.nn.eval(utils.trans_to_input(state, self.simulate_game.curr_player))
                    for action in self.simulate_game.actions():
                        node.add_child(action=(action[0], action[1]),
                                       priority=state_prob[0, action[0]*15 + action[1]])

                choose_action, node, expand = node.policy()
                end, state = self.simulate_game.place_chess(choose_action)

            if end:
                node.backup(1)
            elif expand:
                v, _ = self.nn.eval(utils.trans_to_input(state, self.simulate_game.curr_player))
                node.backup(v)

    def game(self):
        end = False
        states = []
        dists = []
        state = np.zeros((15, 15))
        while not end:
            self.simulation()

            states.append(state)
            action, dist = self.curr_node.distribution()
            dists.append(dist)

            end, state = self.main_game.place_chess(action)
            self.curr_node = self.step(action)

        if self.main_game.steps % 2 == 1:
            winner = 1
        else:
            winner = -1

        player = 1

        for i in range(len(states)):
            self.nn.store_experience(states[i], dists[i], player, winner)
            player *= -1
