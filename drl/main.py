import mcts
import network
import utils
from multiprocessing import Pool, Manager
from multiprocessing.managers import BaseManager
from functools import partial
import time

STATE_SAVE_FOLDER = 'state/'
BASE = 0


def train(_, network, lock):
    game_index = network.count()
    print('{} game start'.format(game_index))

    tree = mcts.MCTS(network, game_index)
    game_begin_time = int(time.time())
    tree.game()

    learn_begin_time = int(time.time())

    lock.acquire()
    print('{} learn start'.format(game_index))

    for _ in range(100):
        network.learn()

    if game_index % 50 == 0:
        network.save_state("{}state_{}.pkl".format(STATE_SAVE_FOLDER, game_index+int(BASE)))
        network.save_memory("memory.npy")

    print('{} learn end'.format(game_index))
    lock.release()

    print('{} game end'.format(game_index))

    learn_end_time = int(time.time())
    learn_min, learn_sec = utils.compute_time(learn_begin_time, learn_end_time)
    print('learning cost {} mins {} seconds'.format(learn_min, learn_sec))

    game_end_time = int(time.time())
    game_min, game_sec = utils.compute_time(game_begin_time, game_end_time)
    print('{} game cost {} mins {} seconds'.format(game_index, game_min, game_sec))


if __name__ == '__main__':
    BaseManager.register('NN', callable=network.NeuralNetwork)
    manager = BaseManager()
    manager.start()

    neural_network = manager.NN()
    # neural_network.memory = np.load(memory.npy)

    m = Manager()
    lock = m.Lock()

    partial_train = partial(train, network=neural_network, lock=lock)

    pool = Pool()
    pool.map(partial_train, range(500))
