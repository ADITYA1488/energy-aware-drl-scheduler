
from collections import Counter
import numpy as np
from stable_baselines3 import PPO

model = PPO.load("ppo_edge_scheduler_trained.zip", device="cpu")

action_counts = Counter()
non_default = []

# Observation:
# [queue, temperature, previous_core, workload_intensity, deadline_pressure]
for queue in [0.05, 0.30, 0.60, 0.90, 1.0]:
    for temp in [0.10, 0.50, 0.80, 0.95]:
        for workload in [0.20, 0.60, 1.0]:
            for pressure in [0.10, 0.50, 0.90]:
                for core in [0.0, 1.0]:
                    obs = np.array(
                        [queue, temp, core, workload, pressure],
                        dtype=np.float32,
                    )

                    action, _ = model.predict(
                        obs, deterministic=True
                    )
                    action = int(action)
                    action_counts[action] += 1

                    if action != 1:
                        non_default.append(
                            (obs.tolist(), action)
                        )

total = sum(action_counts.values())

print("POLICY GRID TEST")
print(f"Total states tested: {total}")

for action in range(4):
    count = action_counts[action]
    print(
        f"Action {action}: {count} states "
        f"({100 * count / total:.2f}%)"
    )

print(f"\nStates not choosing action 1: {len(non_default)}")

for obs, action in non_default[:20]:
    print(f"State={obs} -> action={action}")
