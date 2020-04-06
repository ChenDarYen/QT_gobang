import mcts
import network
import utils
import pickle
from multiprocessing import Pool, Manager
from multiprocessing.managers import BaseManager
from functools import partial
import time

STATE_SAVE_FOLDER = 'state/'
BASE = 0


class LossRecord:
    def __init__(self):
        self.record = []

    def add(self, record):
        self.record.append(record)

    def save(self, path):
        file = open(path, 'wb')
        pickle.dump(self.record, file)
        file.close()

    def size(self):
        return len(self.record)


def train(_, network, lock, loss_record):
    game_index = network.count()
    print('{} game start'.format(game_index))

    tree = mcts.MCTS(network, game_index)
    game_begin_time = int(time.time())
    tree.game()

    learn_begin_time = int(time.time())

    lock.acquire()
    print('{} learn start'.format(game_index))

    mse_total, cross_entropy_total = 0, 0
    for _ in range(100):
        mse, cross_entropy = network.learn()
        mse_total += mse
        cross_entropy_total += cross_entropy
    loss_record.add([mse_total/100, cross_entropy_total/100])
    print([mse_total/100, cross_entropy_total/100])

    if loss_record.size() % 50 == 0:
        network.save_state("{}state_{}.pkl".format(STATE_SAVE_FOLDER, loss_record.size()))
        network.save_memory("memory.npy")
        loss_record.save("loss_record_{}.pkl".format(BASE))

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
    BaseManager.register('LossRecord', callable=LossRecord)
    manager = BaseManager()
    manager.start()

    neural_network = manager.NN()
    loss_record = manager.LossRecord()
    # neural_network.memory = np.load(memory.npy)

    m = Manager()
    lock = m.Lock()

    partial_train = partial(train,
                            network=neural_network,
                            lock=lock,
                            loss_record=loss_record)

    pool = Pool()
    pool.map(partial_train, range(500))
