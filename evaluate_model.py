
import numpy as np
from stable_baselines3 import PPO
from edge_env import EdgeSchedulerEnv

MODEL_PATH = "ppo_edge_scheduler_trained.zip"
SEEDS = range(100, 150)
MAX_STEPS = 200


def run_episode(env, policy, seed):
    obs, _ = env.reset(seed=seed)
    total_reward = 0.0
    power_cost = 0.0
    queue_sum = 0.0
    temperature_sum = 0.0
    steps = 0

    while steps < MAX_STEPS:
        if policy == "PPO":
            action, _ = model.predict(obs, deterministic=True)
            action = int(action)
        elif policy == "Always action 0":
            action = 0
        elif policy == "Always action 1":
            action = 1
        elif policy == "Always action 2":
            action = 2
        else:
            action = 3

        obs, reward, terminated, truncated, info = env.step(action)

        total_reward += float(reward)
        power_cost += float(info["simulated_power_factor"])
        queue_sum += float(info["queue_load"])
        temperature_sum += float(info["temperature"])
        steps += 1

        if terminated or truncated:
            break

    return {
        "reward": total_reward,
        "power_cost_per_step": power_cost / max(steps, 1),
        "mean_queue": queue_sum / max(steps, 1),
        "mean_temperature": temperature_sum / max(steps, 1),
        "steps": steps,
    }


def main():
    global model
    model = PPO.load(MODEL_PATH, device="cpu")
    policies = [
        "PPO",
        "Always action 0",
        "Always action 1",
        "Always action 2",
        "Always action 3",
    ]

    results = {policy: [] for policy in policies}

    for seed in SEEDS:
        for policy in policies:
            env = EdgeSchedulerEnv(max_episode_steps=MAX_STEPS)
            results[policy].append(run_episode(env, policy, seed))
            env.close()

    print("\nEvaluation across 50 identical episode seeds")
    print("-" * 91)
    print(
        f"{'Policy':<20} {'Mean reward':>13} {'Power cost/step':>17}"
        f" {'Mean queue':>13} {'Mean temp':>12}"
    )
    print("-" * 91)

    for policy in policies:
        rows = results[policy]
        print(
            f"{policy:<20}"
            f"{np.mean([r['reward'] for r in rows]):>13.3f}"
            f"{np.mean([r['power_cost_per_step'] for r in rows]):>17.3f}"
            f"{np.mean([r['mean_queue'] for r in rows]):>13.3f}"
            f"{np.mean([r['mean_temperature'] for r in rows]):>12.3f}"
        )

    print("\nInterpretation:")
    print("- Higher (less negative) mean reward is better.")
    print("- Power cost is a simulated action cost, NOT measured CPU energy.")
    print("- Check queue and temperature alongside reward.")


if __name__ == "__main__":
    main()
