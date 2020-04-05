import numpy as np
import torch


def trans_to_input(state, curr_player):
    if curr_player == 1:
        f1 = np.array(state > 0)
        f2 = np.array(state < 0)
        f3 = np.ones((15, 15))
    else:
        f1 = np.array(state < 0)
        f2 = np.array(state > 0)
        f3 = np.ones((15, 15))

    return torch.unsqueeze(torch.from_numpy(np.stack([f1, f2, f3])).float(), 0)


def compute_time(begin_time, end_time):
    mins, secs = (end_time - begin_time) // 60, (end_time - begin_time) % 60
    return mins, secs
