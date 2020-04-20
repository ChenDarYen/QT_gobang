import numpy as np
import torch
import torch.nn as nn
import utils

BATCH_SIZE = 32
FEATURES = 30
LR = 0.01
GAMMA = .9
DELTA = 1e-7
MEMORY_CAPACITY = 1000


class ResBlock(nn.Module):
    def __init__(self, inchannel, outchannel, stride=1):
        super().__init__()
        self.conv1 = nn.Conv2d(inchannel, outchannel, kernel_size=3, padding=1, stride=stride, bias=False)
        self.conv2 = nn.Conv2d(outchannel, outchannel, kernel_size=3, padding=1, stride=1, bias=False)
        self.bn = nn.BatchNorm2d(outchannel)
        self.relu = nn.ReLU(inplace=True)

        # change the shape of input, make it can add to output
        self.shortcut = nn.Sequential()
        if stride != 1 or inchannel != outchannel:
            self.shortcut = nn.Sequential(
                nn.Conv2d(inchannel, outchannel, kernel_size=1, stride=stride, bias=False),
                self.bn
            )

    def forward(self, x):
        residual = x
        out = self.relu(self.bn(self.conv1(x)))
        out = self.bn(self.conv2(out))
        out += self.shortcut(residual)
        out = self.relu(out)

        return out


class Extractor(nn.Module):
    def __init__(self):
        super().__init__()
        self.inchannel = FEATURES
        self.pad = nn.ConstantPad2d(padding=1, value=3)
        self.conv = nn.Sequential(
            nn.Conv2d(3, FEATURES, kernel_size=3, bias=False),
            nn.BatchNorm2d(FEATURES),
            nn.ReLU(inplace=True)
        )

        self.residual = self._make_layer(FEATURES, 15)

    def _make_layer(self, outchannel, num_blocks, block=ResBlock, stride=1):
        strides = np.ones(num_blocks, dtype=int)
        strides[0] = stride

        layers = []
        for stride in strides:
            layers.append(block(self.inchannel, outchannel, stride=stride))
            self.inchannel = outchannel

        return nn.Sequential(*layers)

    def forward(self, x):
        out = self.conv(self.pad(x))
        out = self.residual(out)

        return out


class PolicyModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.conv_pol = nn.Conv2d(30, 1, kernel_size=1)
        self.bn_pol = nn.BatchNorm2d(1)
        self.fc_pol = nn.Linear(15*15, 15*15)

    def forward(self, x):
        out = torch.relu(self.bn_pol(self.conv_pol(x))).view(-1, 15*15)
        out = self.fc_pol(out)

        return out


class ValueModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.conv_val = nn.Conv2d(30, 1, kernel_size=1)
        self.bn_val = nn.BatchNorm2d(1)
        self.fc_val = nn.Sequential(
            nn.Linear(15*15, 768),
            nn.Linear(768, 1)
        )

    def forward(self, x):
        out = torch.relu(self.bn_val(self.conv_val(x))).view(-1, 15*15)
        out = torch.tanh(self.fc_val(out))

        return out


class Model(Extractor, ValueModel, PolicyModel):
    def __init__(self):
        super().__init__()

    def forward(self, x):
        features = Extractor.forward(self, x)
        return ValueModel.forward(self, features), PolicyModel.forward(self, features)


class NeuralNetwork:
    def __init__(self, state_file_path=None, memory_path=None, count=0):
        self.net = Model()
        if state_file_path:
            self.net.load_state_dict(torch.load(state_file_path))

        self.optimizer = torch.optim.SGD(self.net.parameters(), lr=LR, momentum=GAMMA, weight_decay=1e-4)
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
        batch_output = torch.softmax(prob, dim=1)

        mse = self.loss_mse(value, batch_winner)
        entropy_cross_loss = -torch.mean(torch.sum(batch_dist * torch.log(batch_output+DELTA), 1))
        loss = mse + entropy_cross_loss

        self.optimizer.zero_grad()
        loss.backward()
        self.optimizer.step()

        return mse.item(), entropy_cross_loss.item()

    def eval(self, x):
        with torch.no_grad():
            value, output = self.net(x)
        return value, torch.softmax(output, dim=1)
