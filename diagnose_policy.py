
from collections import Counter
import numpy as np
from stable_baselines3 import PPO
from edge_env import EdgeSchedulerEnv

model = PPO.load("ppo_edge_scheduler_trained.zip", device="cpu")
env = EdgeSchedulerEnv(max_episode_steps=200)

action_counts = Counter()
episode_rewards = []

for seed in range(100, 150):
    obs, _ = env.reset(seed=seed)
    total_reward = 0.0

    while True:
        action, _ = model.predict(obs, deterministic=True)
        action = int(action)
        action_counts[action] += 1

        obs, reward, terminated, truncated, info = env.step(action)
        total_reward += float(reward)

        if terminated or truncated:
            break

    episode_rewards.append(total_reward)

env.close()

total_actions = sum(action_counts.values())

print("PPO ACTION DISTRIBUTION")
print("-" * 35)

for action in range(4):
    count = action_counts[action]
    percentage = 100 * count / total_actions
    print(
        f"Action {action}: {count:5d} times "
        f"({percentage:6.2f}%)"
    )

print("\nMean episode reward:", round(np.mean(episode_rewards), 3))
print("Total decisions:", total_actions)
