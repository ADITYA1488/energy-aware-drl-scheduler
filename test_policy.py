
import numpy as np
from stable_baselines3 import PPO

model = PPO.load("ppo_edge_scheduler.zip", device="cpu")

observations = [
    [0.0, 0.0, 0.0],
    [0.2, 0.3, 0.0],
    [0.5, 0.5, 1.0],
    [0.9, 0.8, 1.0],
    [1.0, 1.0, 1.0],
]

print("Python PPO reference actions:")

for obs in observations:
    action, _ = model.predict(
        np.asarray(obs, dtype=np.float32),
        deterministic=True
    )
    print(f"{obs} -> action {int(action)}")
