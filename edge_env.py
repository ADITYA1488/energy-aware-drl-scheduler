import gymnasium as gym
from gymnasium import spaces
import numpy as np

class EdgeSchedulerEnv(gym.Env):
    metadata = {"render_modes": []}

    def __init__(self):
        super().__init__()
        self.observation_space = spaces.Box(
            low=np.array([0.0, 0.0, 0.0], dtype=np.float32),
            high=np.array([1.0, 1.0, 1.0], dtype=np.float32),
            dtype=np.float32
        )
        self.action_space = spaces.Discrete(4)
        self.reset()

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        self.queue_load = np.float32(np.random.uniform(0.1, 0.9))
        self.temp = np.float32(0.3)
        self.core = np.float32(0.0)
        return np.array([self.queue_load, self.temp, self.core], dtype=np.float32), {}

    def step(self, action):
        configs = {
            0: {"power": 0.2, "cap": 0.2, "temp_inc": 0.01},
            1: {"power": 0.5, "cap": 0.4, "temp_inc": 0.02},
            2: {"power": 1.2, "cap": 0.7, "temp_inc": 0.05},
            3: {"power": 2.5, "cap": 1.0, "temp_inc": 0.10},
        }
        cfg = configs[int(action)]
        
        processed = cfg["cap"]
        self.queue_load = np.float32(np.clip(self.queue_load + np.random.uniform(0.1, 0.4) - processed, 0.0, 1.0))
        self.temp = np.float32(np.clip(self.temp + cfg["temp_inc"] - 0.02, 0.0, 1.0))
        self.core = np.float32(1.0 if action >= 2 else 0.0)
        
        penalty_power = cfg["power"] * 0.4
        penalty_latency = self.queue_load * 1.5
        penalty_thermal = 2.0 if self.temp > 0.8 else 0.0
        reward = float(-(penalty_power + penalty_latency + penalty_thermal))
        
        obs = np.array([self.queue_load, self.temp, self.core], dtype=np.float32)
        info = {"power": cfg["power"], "queue": float(self.queue_load), "temp": float(self.temp)}
        return obs, reward, False, False, info
        
