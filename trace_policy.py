
from collections import Counter, defaultdict
import numpy as np
from stable_baselines3 import PPO
from edge_env import EdgeSchedulerEnv

model = PPO.load("ppo_edge_scheduler_trained.zip", device="cpu")
env = EdgeSchedulerEnv(max_episode_steps=200)

counts = Counter()
groups = defaultdict(Counter)
examples = []

for seed in range(100, 150):
    obs, _ = env.reset(seed=seed)

    while True:
        action, _ = model.predict(obs, deterministic=True)
        action = int(action)

        queue, temp, core, workload, pressure = obs
        counts[action] += 1

        # Group decisions by deadline pressure
        if pressure < 1 / 3:
            group = "Low pressure"
        elif pressure < 2 / 3:
            group = "Medium pressure"
        else:
            group = "High pressure"

        groups[group][action] += 1

        if action != 1 and len(examples) < 15:
            examples.append(
                (obs.copy(), action)
            )

        obs, reward, terminated, truncated, info = env.step(action)

        if terminated or truncated:
            break

env.close()

total = sum(counts.values())
print(f"Total decisions: {total}")

print("\nOverall action distribution:")
for action in range(4):
    print(
        f"Action {action}: {counts[action]} "
        f"({100 * counts[action] / total:.2f}%)"
    )

print("\nActions by deadline pressure:")
for group in ["Low pressure", "Medium pressure", "High pressure"]:
    row = groups[group]
    n = sum(row.values())
    print(f"\n{group} (n={n})")
    for action in range(4):
        print(
            f"  Action {action}: {row[action]} "
            f"({100 * row[action] / max(n, 1):.2f}%)"
        )

print("\nFirst decisions that differ from action 1:")
for obs, action in examples:
    print(
        f"queue={obs[0]:.2f}, temp={obs[1]:.2f}, "
        f"workload={obs[3]:.2f}, pressure={obs[4]:.2f}"
        f" -> action {action}"
    )
