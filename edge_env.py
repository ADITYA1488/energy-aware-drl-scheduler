
import gymnasium as gym
from gymnasium import spaces
import numpy as np


class EdgeSchedulerEnv(gym.Env):
    """
    Software simulation of energy-aware adaptive scheduling.

    Observation:
        0: queue load
        1: temperature
        2: previous selected core class
        3: incoming workload intensity
        4: deadline pressure

    Actions:
        0: energy-saving mode
        1: balanced mode
        2: performance mode
        3: high-performance mode

    All power and thermal values are simulated, not measured.
    """

    metadata = {"render_modes": []}

    def __init__(self, max_episode_steps=200):
        super().__init__()

        self.max_episode_steps = max_episode_steps
        self.elapsed_steps = 0

        self.observation_space = spaces.Box(
            low=np.zeros(5, dtype=np.float32),
            high=np.ones(5, dtype=np.float32),
            dtype=np.float32,
        )

        self.action_space = spaces.Discrete(4)

        # Simulated power cost, service capacity, heat generation
        self.action_configs = {
            0: (0.20, 0.18, 0.005),
            1: (0.50, 0.32, 0.015),
            2: (1.20, 0.48, 0.035),
            3: (2.50, 0.65, 0.070),
        }

        self.queue_load = 0.0
        self.temperature = 0.3
        self.core = 0
        self.incoming_work = 0.25
        self.deadline_pressure = 0.5
        self.dropped_work = 0.0

    def _observation(self):
        return np.array(
            [
                self.queue_load,
                self.temperature,
                float(self.core),
                self.incoming_work / 0.65,
                self.deadline_pressure,
            ],
            dtype=np.float32,
        )

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)

        self.elapsed_steps = 0
        self.queue_load = float(self.np_random.uniform(0.05, 0.65))
        self.temperature = float(self.np_random.uniform(0.20, 0.40))
        self.core = 0
        self.incoming_work = float(self.np_random.uniform(0.20, 0.60))
        self.deadline_pressure = float(self.np_random.uniform(0.10, 0.90))
        self.dropped_work = 0.0

        return self._observation(), {}

    def step(self, action):
        if not self.action_space.contains(action):
            raise ValueError(f"Invalid action: {action}")

        power, capacity, heat = self.action_configs[int(action)]

        # Work arriving during this time step
        incoming = self.incoming_work

        # Work served and work remaining in the queue
        available_work = self.queue_load + incoming
        completed_work = min(available_work, capacity)
        remaining_work = available_work - completed_work

        # Overflow is recorded and penalized instead of silently discarded
        self.dropped_work = max(0.0, remaining_work - 1.0)
        self.queue_load = min(1.0, remaining_work)

        # Simple normalized thermal model with cooling
        self.temperature = float(
            np.clip(
                self.temperature
                + heat
                + 0.025 * incoming
                - 0.025,
                0.0,
                1.0,
            )
        )

        self.core = 1 if action >= 2 else 0

        # Workload changes gradually, with occasional bursts
        
        if self.np_random.random() < 0.30:
            target_work = float(
                self.np_random.uniform(0.45, 0.65)
            )
        else:
            target_work = float(
                self.np_random.uniform(0.20, 0.45)
            )
        
        self.incoming_work = float(
            np.clip(
                0.75 * self.incoming_work + 0.25 * target_work,
                0.08,
                0.65,
            )
        )

        # Higher pressure means work is more urgent
        pressure_change = float(self.np_random.uniform(-0.12, 0.12))
        self.deadline_pressure = float(
            np.clip(
                0.85 * self.deadline_pressure + pressure_change,
                0.0,
                1.0,
            )
        )

        self.elapsed_steps += 1

        thermal_penalty = max(0.0, self.temperature - 0.70) * 2.0
        queue_penalty = 1.2 * self.queue_load
        deadline_penalty = (
            1.5 * self.deadline_pressure * self.queue_load
        )
        overflow_penalty = 4.0 * self.dropped_work

        reward = (
            1.5 * completed_work
            - 0.35 * power
            - queue_penalty
            - deadline_penalty
            - thermal_penalty
            - overflow_penalty
        )

        terminated = False
        truncated = self.elapsed_steps >= self.max_episode_steps

        info = {
            "simulated_power_factor": power,
            "completed_work": completed_work,
            "queue_load": self.queue_load,
            "temperature": self.temperature,
            "deadline_pressure": self.deadline_pressure,
            "dropped_work": self.dropped_work,
        }

        return (
            self._observation(),
            float(reward),
            terminated,
            truncated,
            info,
        )
