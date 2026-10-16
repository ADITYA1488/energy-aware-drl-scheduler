
import numpy as np
from stable_baselines3 import PPO
from edge_env import EdgeSchedulerEnv

SEEDS = range(100, 150)
MODEL_PATH = "ppo_edge_scheduler_trained.zip"


def heuristic(obs):
    queue, temp, core, workload, pressure = obs

    # Avoid aggressive modes when temperature is high.
    if temp >= 0.75:
        return 0 if queue < 0.25 and workload < 0.45 else 1

    # Urgent or heavy workloads need more capacity.
    if queue >= 0.65 or workload >= 0.80 or pressure >= 0.80:
        return 3 if temp < 0.60 else 2

    # Moderate demand: use intermediate capacity.
    if queue >= 0.35 or workload >= 0.65 or pressure >= 0.65:
        return 2

    # Save power when demand and urgency are low.
    if queue <= 0.15 and workload <= 0.40 and pressure <= 0.40:
        return 0

    return 1


def run_episode(policy, seed, model):
    env = EdgeSchedulerEnv(max_episode_steps=200)
    obs, _ = env.reset(seed=seed)

    totals = {
        "reward": 0.0,
        "power": 0.0,
        "completed": 0.0,
        "dropped": 0.0,
        "queue": 0.0,
        "temperature": 0.0,
        "action3": 0,
    }
    steps = 0

    while True:
        if policy == "PPO":
            action, _ = model.predict(obs, deterministic=True)
            action = int(action)
        elif policy == "Always action 1":
            action = 1
        else:
            action = heuristic(obs)

        obs, reward, terminated, truncated, info = env.step(action)

        totals["reward"] += float(reward)
        totals["power"] += info["simulated_power_factor"]
        totals["completed"] += info["completed_work"]
        totals["dropped"] += info["dropped_work"]
        totals["queue"] += info["queue_load"]
        totals["temperature"] += info["temperature"]
        totals["action3"] += int(action == 3)
        steps += 1

        if terminated or truncated:
            break

    env.close()

    return {
        "reward": totals["reward"],
        "power": totals["power"] / steps,
        "completed": totals["completed"],
        "dropped": totals["dropped"],
        "queue": totals["queue"] / steps,
        "temperature": totals["temperature"] / steps,
        "action3_pct": 100 * totals["action3"] / steps,
    }


def main():
    model = PPO.load(MODEL_PATH, device="cpu")
    policies = ["PPO", "Always action 1", "Adaptive heuristic"]
    results = {}

    for policy in policies:
        results[policy] = [
            run_episode(policy, seed, model)
            for seed in SEEDS
        ]

    metrics = [
        ("reward", "Reward"),
        ("power", "Power/step"),
        ("completed", "Completed work"),
        ("dropped", "Dropped work"),
        ("queue", "Mean queue"),
        ("temperature", "Mean temp"),
        ("action3_pct", "Action 3 %"),
    ]

    print("\nMean results over 50 identical seeds")
    print("-" * 112)
    print(f"{'Policy':<22}" + "".join(f"{label:>14}" for _, label in metrics))
    print("-" * 112)

    for policy in policies:
        rows = results[policy]
        values = [
            np.mean([row[key] for row in rows])
            for key, _ in metrics
        ]
        print(
            f"{policy:<22}"
            + "".join(f"{value:>14.3f}" for value in values)
        )

    print("\nAll power and workload quantities are simulated, not hardware measurements.")


if __name__ == "__main__":
    main()

