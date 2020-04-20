import numpy as np
import torch
import torch.nn as nn
import utils

BATCH_SIZE = 50
LR = .001
GAMMA = .9
MEMORY_CAPACITY = 1000


class BaseModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.pad = nn.ConstantPad2d(padding=2, value=3)  # 3 means border
        self.conv1 = nn.Conv2d(
            in_channels=3,  # input size (3, 15+4, 15+4)
            out_channels=30,
            kernel_size=5,  # output size (30, 15, 15)
        )
        self.conv2 = nn.Conv2d(
            in_channels=30,  # input size (30, 15, 15)
            out_channels=60,
            kernel_size=3,  # output size (60, 13, 13)
        )
        self.conv3 = nn.Conv2d(
            in_channels=60,  # input size (60, 13, 13)
            out_channels=120,
            kernel_size=3,  # output size (120, 11, 11)
        )
        self.b_norm1 = nn.BatchNorm2d(30)
        self.b_norm2 = nn.BatchNorm2d(60)
        self.b_norm3 = nn.BatchNorm2d(120)
        self.relu = nn.ReLU()

    def forward(self, x):
        x = self.pad(x)
        x = self.relu(self.b_norm1(self.conv1(x)))
        x = self.relu(self.b_norm2(self.conv2(x)))
        x = self.relu(self.b_norm3(self.conv3(x)))
        return x


class ValueModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.fc_value = nn.Linear(120*11*11, 40)
        self.output_value = nn.Linear(40, 1)

    def forward(self, x):
        x = self.relu(self.fc_value(x))
        x = self.output_value(x)
        return x


class ProbModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.output_prob = nn.Linear(120*11*11, 15*15)

    def forward(self, x):
        x = self.output_prob(x)
        return x


class Model(BaseModel, ValueModel, ProbModel):
    def __init__(self):
        super().__init__()

    def forward(self, x):
        x = BaseModel.forward(self, x).view(-1, 120*11*11)
        return ValueModel.forward(self, x), ProbModel.forward(self, x)


class NeuralNetwork:
    def __init__(self, state_file_path=None, memory_path=None, count=0):
        self.net = Model()
        if state_file_path:
            self.net.load_state_dict(torch.load(state_file_path))

        self.optimizer = torch.optim.SGD(self.net.parameters(), lr=LR, momentum=.9, weight_decay=1e-4)
        self.loss_mse = nn.MSELoss()

        # each experience contains board, distribution, player, winner, and index
        if memory_path:
            self.memory = np.load(memory_path)
        else:
            self.memory = np.zeros((MEMORY_CAPACITY, 15*15*2+3), dtype=np.float32)

        self.memory_counter = int(np.max(self.memory[:, 15*15*2+2]))
        if self.memory_counter != 0:
            self.memory_counter += 1

        self.counter = count

    def count(self):
        self.counter += 1
        return self.counter

    def save_state(self, path):
        torch.save(self.net.state_dict(), path)

    def save_memory(self, path):
        np.save(path, self.memory)

    def store_experience(self, board, dist, player, winner):
        index = self.memory_counter % MEMORY_CAPACITY
        self.memory[index, :] = np.hstack((board.reshape((1, 15*15)),
                                           dist.reshape((1, 15*15)),
                                           [[player, winner, self.memory_counter]]))
        self.memory_counter += 1

    def learn(self):
        memory_size = min(MEMORY_CAPACITY, self.memory_counter)

        sample_index = np.random.choice(memory_size, BATCH_SIZE)
        batch_memory = self.memory[sample_index, :]
        batch_state = batch_memory[:, :15*15]
        batch_player = batch_memory[:, 2*15*15]
        batch_input = torch.stack([utils.trans_to_input(batch_state[i].reshape(15, 15), batch_player[i])
                                   for i in range(BATCH_SIZE)]).view(BATCH_SIZE, 3, 15, 15)
        batch_dist = torch.from_numpy(batch_memory[:, 15*15:2*15*15])
        batch_winner = torch.from_numpy(batch_memory[:, 2*15*15+1]).unsqueeze(dim=1)

        value, prob = self.net(batch_input)
        batch_output = nn.functional.softmax(prob, dim=1)

        mse = self.loss_mse(value, batch_winner)
        entropy_cross_loss = -torch.mean(torch.sum(batch_dist * torch.log(batch_output), 1))
        loss = mse + entropy_cross_loss

        self.optimizer.zero_grad()
        loss.backward()
        self.optimizer.step()

        return mse.item(), entropy_cross_loss.item()

    def eval(self, x):
        with torch.no_grad():
            value, output = self.net(x)
        return value, nn.functional.softmax(output, dim=1)
