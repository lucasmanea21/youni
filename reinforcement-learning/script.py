import base64
import pickle
import numpy as np
import random


# Game Environment
class SnakeEnvironment:
    def __init__(self, board_size=20):  # Changed board size to 20
        self.board_size = board_size
        self.reset()

    def reset(self):
        self.board = np.zeros((self.board_size, self.board_size))
        self.snake1_position = [(self.board_size // 2, self.board_size // 2)]
        self.snake2_position = [(self.board_size // 2 - 1, self.board_size // 2)]
        self._update_board()
        return self.board.flatten()

    def _update_board(self):
        self.board = np.zeros((self.board_size, self.board_size))
        for pos in self.snake1_position:
            self.board[pos] = 1
        for pos in self.snake2_position:
            self.board[pos] = 2

    def step(self, action):
        head1 = self.snake1_position[0]
        head2 = self.snake2_position[0]

        if action == 0:
            new_head1 = (head1[0] - 1, head1[1])
        elif action == 1:
            new_head1 = (head1[0] + 1, head1[1])
        elif action == 2:
            new_head1 = (head1[0], head1[1] - 1)
        else:
            new_head1 = (head1[0], head1[1] + 1)

        # Check for crash or self-collision BEFORE updating snake's position on the board
        if (new_head1[0] < 0 or new_head1[1] < 0 or new_head1[0] >= self.board_size or new_head1[1] >= self.board_size
                or self.board[new_head1] == 1 or self.board[new_head1] == 2):
            return self.board.flatten(), -5, True  # Higher penalty for crashing

        # Update the snake's position now that we've checked for collisions
        self.snake1_position = [new_head1] + self.snake1_position[:-1]
        self._update_board()

        # Check if the snake is getting too close to the wall
        if 0 in new_head1 or self.board_size - 1 in new_head1:
            return self.board.flatten(), -2, False  # Penalty for getting close to the wall

        return self.board.flatten(), 1, False  # Reward for moving in open space



# DQN Model
class DQN:
    def __init__(self, input_size=400, output_size=4, learning_rate=0.001):  # Changed input size to 400
        self.weights1 = np.random.randn(input_size, 64) / np.sqrt(input_size)
        self.weights2 = np.random.randn(64, output_size) / np.sqrt(64)
        self.learning_rate = learning_rate

    def predict(self, state):
        z1 = np.dot(state, self.weights1)
        a1 = np.tanh(z1)
        z2 = np.dot(a1, self.weights2)
        return z2

    def update(self, state, target, action):
        z1 = np.dot(state, self.weights1)
        a1 = np.tanh(z1)
        z2 = np.dot(a1, self.weights2)
        
        delta2 = z2
        delta2[action] = target - delta2[action]
        d_weights2 = np.outer(a1, delta2)
        
        delta1 = (1 - a1**2) * np.dot(self.weights2, delta2)
        d_weights1 = np.outer(state, delta1)
        
        d_weights1 = np.clip(d_weights1, -1, 1)
        d_weights2 = np.clip(d_weights2, -1, 1)
        
        self.weights1 += self.learning_rate * d_weights1
        self.weights2 += self.learning_rate * d_weights2

# Snake Agent
class SnakeAgent:
    def __init__(self, memory_size=2000):
        self.dqn = DQN(400, 4)  # Changed input size to 400
        self.memory = []
        self.memory_size = memory_size
        self.gamma = 0.99

    def choose_action(self, state, epsilon):
        if np.random.uniform(0, 1) < epsilon:
            return np.random.choice([0, 1, 2, 3])
        else:
            q_values = self.dqn.predict(state)
            return np.argmax(q_values)

    def train(self, batch_size=32):
        if len(self.memory) < batch_size:
            return
        batch = random.sample(self.memory, batch_size)
        for state, action, reward, next_state, done in batch:
            if done:
                target = reward
            else:
                target = reward + self.gamma * np.max(self.dqn.predict(next_state))
            self.dqn.update(state, target, action)
            
    def store_memory(self, experience):
        if len(self.memory) >= self.memory_size:
            self.memory.pop(0)
        self.memory.append(experience)

# Training Loop
def train_agent_simplified(episodes=10000, batch_size=16, epsilon_start=1.0, epsilon_end=0.01, epsilon_decay=0.995, train_frequency=3):
    env = SnakeEnvironment()
    agent = SnakeAgent()
    
    for episode in range(episodes):
        state = env.reset()
        episode_reward = 0
        step_count = 0
        
        while True:
            action = agent.choose_action(state, epsilon_start)
            next_state, reward, done = env.step(action)
            agent.store_memory((state, action, reward, next_state, done))
            
            step_count += 1
            if step_count % train_frequency == 0:
                agent.train(batch_size)
            
            episode_reward += reward
            state = next_state

            if (episode % 10 == 0):
                print("Episode", episode, "reward", episode_reward)
            
            if done:
                break
        
        epsilon_start = max(epsilon_end, epsilon_start * epsilon_decay)

    return agent

# Train the agent
trained_agent = train_agent_simplified()

print(trained_agent.dqn.weights1)

print("Training completed!")

# Serialize the trained agent's weights and biases
weights_data = {
    'weights1': trained_agent.dqn.weights1,
    'weights2': trained_agent.dqn.weights2
}

serialized_weights = pickle.dumps(weights_data)
encoded_weights = base64.b64encode(serialized_weights).decode('utf-8')

# Creating the standalone Python script
script_content = f"""#!/usr/bin/env python3
import numpy as np
import base64
import pickle

# Decoding the trained agent's weights and biases
encoded_weights = '{encoded_weights}'
decoded_weights = base64.b64decode(encoded_weights)
weights_data = pickle.loads(decoded_weights)

class DQN:
    def __init__(self):
        self.weights1 = weights_data['weights1']
        self.weights2 = weights_data['weights2']

    def predict(self, state):
        z1 = np.dot(state, self.weights1)
        a1 = np.tanh(z1)
        z2 = np.dot(a1, self.weights2)
        return z2

class SnakeAgent:
    def __init__(self):
        self.dqn = DQN()

    def choose_action(self, state, epsilon=0):
        q_values = self.dqn.predict(state)
        return np.argmax(q_values)

def get_best_move(state):
    if isinstance(state, list) or len(state.shape) == 2:
        state = np.array(state).flatten()
    agent = SnakeAgent()
    action = agent.choose_action(state)
    return action

if __name__ == '__main__':
    # Read the game state from snake.in
    with open("snake.in", "r") as f:
        state_data = f.readlines()[:-1]  # Exclude the last line
        state = np.array([list(map(int, line.strip())) for line in state_data])
    
    # Get the best move
    action = get_best_move(state)

    # Write the action to snake.out
    with open("snake.out", "w") as f:
        f.write(str(action))
"""
# Saving the script to a file
script_filename = './rl/snake_bot.py'
with open(script_filename, 'w') as f:
    f.write(script_content)

script_filename