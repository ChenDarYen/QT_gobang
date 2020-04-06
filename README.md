# QT_gobang
下載：(.dmg)
---
[v1.2](https://github.com/ChenDarYen/QT_gobang/releases/download/v1.2/gobang1.2.dmg) / 
[v1.1](https://github.com/ChenDarYen/QT_gobang/releases/download/v1.1/gobang1.1.dmg) / 
[v1.0](https://github.com/ChenDarYen/QT_gobang/releases/download/v1.0/gobang.dmg)

簡介
---
模仿 AlphaGo Zero 訓練電腦學習五子棋，取代 v1 中使用 Nega Scout Search 的 AI。  
主要用 Pytorch 建構神經網路和訓練，為了加速訓練使用了 multiprocessing。  
c++ 使用 libtorch 調用訓練後輸出的模型。

主要架構包含了 MCTS(Monte Carlo tree search) 和 Deep Neural Network，這裡的 DNN 輸出當前局面的預測價值 (value) 和動作機率 (policy)。  
透過 DNN 主導的 MCTS 不斷的 self-play 蒐集 training data 訓練 DNN 優化兩個輸出目標。

<p align="center">
  <img src="https://github.com/ChenDarYen/gobang_drl/blob/master/structure.png" width="700px">
</p>

DNN
---
這裡設計的神經網路輸入為 3 \* 15  \* 15 的矩陣，包含了一個當前局面自己的棋子，一個對手的棋子，和一個代表了當前的 player。 
先透過數個卷積層提取盤面的 features 後再分別丟進兩個各別的全連接層輸出 value 和 policy。

#### Experience Replay
Self-play 中的每一步都可以生成一筆訓練資料，儲存進容量限制的 memory 中。  
訓練資料的形式為 (s<sub>t</sub>, π<sub>t</sub>, p<sub>t</sub>, z<sub>t</sub>)，s<sub>t</sub> 代表當時的局面，π<sub>t</sub> 代表當時 MCTS 的動作機率分佈，p<sub>t</sub> 代表當前的玩家，z<sub>t</sub> 代表當局結束時的贏家，黑勝為 1，白勝為 0。

可以發現一局中的連續的狀態與狀態只相差了一顆棋子卻擁有相同的贏家，這造成了高度的 correlation（相似的狀態有同樣的回歸目標），容易導致價值函數出現 overfitting。  
因此我們採用 experience replay，隨機抽取固定個數的訓練資料生成一個訓練 batch，以降低 correlation。  
另外因為價值函數和策略函數共用了一部份的 NN，也讓 overfitting 不再出現。


#### Loss Function
Loss funciton 為 MSE 和 cross-entropy loss 的合。

<p align="center">
  <img src="https://github.com/ChenDarYen/gobang_drl/blob/master/loss_function.png" width="300px">
</p>

第三項為 L2 正則項，這裡設置常數 c 為 1e-4。  
MSE 用於優化 value，cross-entropy loss 用於優化 policy。

MCTS
---
MCTS 包括了四個步驟：

<ul>
  <li>Selection 選擇：從根節點開始持續選取子結點直到抵達葉節點，葉節點可能為終局或子節點尚未展開。</li>
  <li>Expansion 展開：從葉節點的子節點中選取數個展開。</li>
  <li>Simulaion 模擬：從根節點開始迅速的模擬出結果。</li>
  <li>Backpropagation 反向傳播：用模擬結果更新路徑上的節點訊息。</li>
</ul>

#### UCT
再處理 MAB 上選擇了 UCT(Upper Confidence Bound for tree) ，比 ε-greedy 更能充分運用已取得的統計訊息。其值為邊（action）的 Q 值和其 UCB 的加總。  
Q 值為過往 MCTS 每一次反向傳播的平均值。  
UCB 代表了對 Q 值的信賴上界，信賴上界越大表示越沒有信心確認 Q 值的準確，也將更容易被選到。

<p align="center">
  <img src="https://github.com/ChenDarYen/gobang_drl/blob/master/ucb.png" width="300px">
</p>

N 表示過往被選擇的次數，次數越低 UCB 越高。  
P 為動作被選擇的先驗機率，在根節點上會加上一些 noise 增加探索的廣度。

#### Expansion
在 Zero 中強調不使用額外資訊的強化學習，因此 tree-policy 簡化直接展開所有可能的子節點。  
擴充時會以神經網路給出的機率初始化動作的 prior。

#### Backpropagation
如果模擬的終點為終局，backup 的值為 1。  
如果終點為尚未 expansion 的節點，backup 值由神經網路給出。

#### Policy
在經過一定程度的模擬後（這裡固定為 400 次），policy 將使用蒐集到的統計值給出選點。

於 s<sub>0</sub> 的狀態下選擇 a 的機率為:

<p align="center">
  <img src="https://github.com/ChenDarYen/gobang_drl/blob/master/policy.png" width="300px">
</p>

τ 代表了控制探索程度的溫度，這裡每局的前 20 步固定為 1，超過的設為 1 / 步數。

程式碼
---
下面是部分的程式碼。
#### 神經網路
BaseModel 為價值函數和策略函數共用的部分。

###### network.py
```python
class Model(BaseModel, ValueModel, ProbModel):
    def __init__(self):
        super().__init__()

    def forward(self, x):
        x = BaseModel.forward(self, x).view(-1, 120*11*11)
        return ValueModel.forward(self, x), ProbModel.forward(self, x)


class NeuralNetwork:
    def __init__(self, state_file_path=None):
        self.net = Model()
        if state_file_path:
            self.net.load_state_dict(torch.load(state_file_path))

        self.optimizer = torch.optim.SGD(self.net.parameters(), lr=LR, momentum=.9, weight_decay=1e-4)
        self.loss_mse = nn.MSELoss()

        # each experience contains board, distribution, player, winner
        self.memory = np.zeros((MEMORY_CAPACITY, 15*15*2+2), dtype=np.float32)
        self.memory_counter = 0

    def store_experience(self, board, dist, player, winner):
        index = self.memory_counter % MEMORY_CAPACITY
        self.memory[index, :] = np.hstack((board.reshape((1, 15*15)), dist.reshape((1, 15*15)), [[player, winner]]))
        self.memory_counter += 1

    def learn(self):
        memory_size = min(MEMORY_CAPACITY, self.memory_counter)

        sample_index = np.random.choice(memory_size, BATCH_SIZE)
        batch_memory = self.memory[sample_index, :]
        batch_state = batch_memory[:, :15*15]
        batch_player = batch_memory[:, 2*15*15:2*15*15+1]
        batch_input = torch.stack([utils.trans_to_input(batch_state[i].reshape(15, 15), batch_player[i])
                                   for i in range(BATCH_SIZE)]).view(BATCH_SIZE, 3, 15, 15)
        batch_dist = torch.from_numpy(batch_memory[:, 15*15:2*15*15])
        batch_winner = torch.from_numpy(batch_memory[:, 2*15*15+1:])

        value, prob = self.net(batch_input)
        batch_output = nn.functional.softmax(prob, dim=1)

        mse = self.loss_mse(value, batch_winner)
        entropy_cross_loss = -torch.mean(torch.sum(batch_dist * torch.log(batch_output), 1))
        loss = mse + entropy_cross_loss

        self.optimizer.zero_grad()
        loss.backward()
        self.optimizer.step()

        return mse.item(), entropy_cross_loss.item()

    def eval(self, x):  # use in MCTS
        with torch.no_grad():
            value, output = self.net(x)
        return value, nn.functional.softmax(output, dim=1)
```

#### MCTS
包含三個部分：Node、Edge 和 MCTS 主體。

###### mcts.py
```python
class Node:
    def __init__(self, edge, player):
        self.edge_from = edge
        self.player = player
        self.edges_away = {}
        self.counter = 0

    def add_child(self, action, prior):  # create new Edge without new child Node
        self.edges_away[action] = Edge(self, action, prior)

    def get_child(self, action):
        child, _ = self.edges_away[action].get()
        return child

    def backup(self, value):  # backpropagation
        self.counter += 1
        if self.edge_from:
            self.edge_from.backup(value)

    def distribution(self, step):  # get distribution and select point
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

        # since the program is run in multiprocessing, we need to set the seed
        np.random.seed(int.from_bytes(os.urandom(4), byteorder='little'))  
        action_idx = np.random.choice(len(actions_pool), p=actions_dist)
        action = actions_pool[action_idx]

        return action, dist

    def policy(self, add_noise=False):  # choose action under UCT
        ucb_max = -sys.maxsize
        choose_edge = None

        if add_noise:
            noise = np.random.dirichlet(.2*np.ones(len(self.edges_away)))  # use Dirichlet create noise
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

        if self.main_game.steps % 2 == 1:
            winner = 1
        else:
            winner = -1

        player = 1

        for i in range(len(states)):
            self.nn.store_experience(states[i], dists[i], player, winner)
            player *= -1
```
