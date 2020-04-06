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

    def add_child(self, action, prior):
        self.edges_away[action] = Edge(self, action, prior)

    def get_child(self, action):
        child, _ = self.edges_away[action].get()
        return child

    def backup(self, value):
        self.counter += 1
        if self.edge_from:
            self.edge_from.backup(value)

    def distribution(self, step):
        # set the temperature, which is used to controls the degree of exploration
        if step < 20:
            tau = 1
        else:
            tau = 1/step

        dist = np.zeros((15, 15))
        actions_pool = []
        actions_dist = []
        for edge in self.edges_away.values():
            if edge.counter != 0:
                tmp = np.float_power(edge.counter, 1 / tau)
                dist[edge.action[0], edge.action[1]] = tmp
                actions_pool.append(edge.action)
                actions_dist.append(tmp)

        denominator = np.sum(dist)
        dist /= denominator
        actions_dist = [value/denominator for value in actions_dist]

        np.random.seed(int.from_bytes(os.urandom(4), byteorder='little'))
        action_idx = np.random.choice(len(actions_pool), p=actions_dist)
        action = actions_pool[action_idx]

        return action, dist

    def policy(self, add_noise=False):  # choose action under UCT
        ucb_max = -sys.maxsize
        choose_edge = None

        if add_noise:
            noise = np.random.dirichlet(.2*np.ones(len(self.edges_away)))
        else:
            noise = np.zeros(len(self.edges_away))

        i = 0
        for edge in self.edges_away.values():
            ucb = edge.ucb(noise[i])
            i += 1
            if ucb > ucb_max:
                ucb_max = ucb
                choose_edge = edge

        choose_node, expand = choose_edge.get()
        return choose_edge.action, choose_node, expand


class Edge:
    # let action be a tuple, so we can use it be the key of dictionary
    def __init__(self, node_from, action: tuple, prior):
        self.node_from = node_from
        self.action = action
        self.prior = prior
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

    def ucb(self, noise=0):
        q = self.value/self.counter if self.counter else 0

        if noise:
            return q + C_PUCT * (.8*self.prior + .2*noise) * np.sqrt(self.node_from.counter) / (self.counter+1)
        else:
            return q + C_PUCT * self.prior * np.sqrt(self.node_from.counter) / (self.counter + 1)


class MCTS:
    def __init__(self, neural_network, index):
        self.curr_node = Node(None, 1)
        self.nn = neural_network
        self.main_game, self.simulate_game = game.Gobang(), game.Gobang()
        self.index = index

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
                                       prior=state_prob[0, action[0]*15 + action[1]])

                # add noise when select in root node
                choose_action, node, expand = node.policy(node == self.curr_node)

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
        step = 0
        while not end:
            self.simulation()

            states.append(state)
            action, dist = self.curr_node.distribution(step)
            dists.append(dist)

            end, state = self.main_game.place_chess(action)
            self.curr_node = self.step(action)
            step += 1
            print('{} place'.format(self.index))

        if self.main_game.steps % 2 == 1:
            winner = 1
        else:
            winner = -1

        player = 1

        for i in range(len(states)):
            self.nn.store_experience(states[i], dists[i], player, winner)
            player *= -1
