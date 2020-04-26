import mcts
import network_resnet
import utils
import pickle
import numpy as np
import os
from multiprocessing import Pool, Manager
from multiprocessing.managers import BaseManager
from functools import partial
import time

STATE_SAVE_FOLDER = 'state_resnet/'
TRAINING_STEP = 200
SAVE_INTERVAL = 25


class LossRecord:
    def __init__(self, record_path=None):
        if record_path:
            file = open(record_path, 'rb')
            self.record = pickle.load(file)
            file.close()
        else:
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
    np.random.seed(int.from_bytes(os.urandom(4), byteorder='little'))
    for _ in range(TRAINING_STEP):
        mse, cross_entropy = network.learn()
        mse_total += mse
        cross_entropy_total += cross_entropy
    loss_record.add([mse_total/TRAINING_STEP, cross_entropy_total/TRAINING_STEP])
    print([mse_total/TRAINING_STEP, cross_entropy_total/TRAINING_STEP])

    if loss_record.size() % SAVE_INTERVAL == 0:
        print('save')
        network.save_state("{}state_{}.pkl".format(STATE_SAVE_FOLDER, loss_record.size()))
        network.save_memory("memory_resnet.npy")
        loss_record.save("loss_record_resnet.pkl")

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
    BaseManager.register('NN_resnet', callable=network_resnet.NeuralNetwork)
    BaseManager.register('LossRecord', callable=LossRecord)
    manager = BaseManager()
    manager.start()

    # loss_record = manager.LossRecord()
    # neural_network = manager.NN_resnet()
    loss_record = manager.LossRecord('loss_record_resnet.pkl')
    neural_network = manager.NN_resnet('state_resnet/state_575.pkl', 'memory_resnet.npy', loss_record.size())

    m = Manager()
    lock = m.Lock()

    partial_train = partial(train,
                            network=neural_network,
                            lock=lock,
                            loss_record=loss_record)

    pool = Pool()
    pool.map(partial_train, range(1000))
