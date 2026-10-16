import random
import numpy as np
import torch

from stable_baselines3 import PPO
from stable_baselines3.common.env_checker import check_env
from stable_baselines3.common.monitor import Monitor

from edge_env import EdgeSchedulerEnv


SEED = 42
TOTAL_TIMESTEPS = 50_000
MODEL_PATH = "ppo_edge_scheduler_trained"


def main():
    random.seed(SEED)
    np.random.seed(SEED)
    torch.manual_seed(SEED)

    env = EdgeSchedulerEnv()
    check_env(env, warn=True)
    env = Monitor(env)

    model = PPO(
        policy="MlpPolicy",
        env=env,
        learning_rate=3e-4,
        n_steps=1024,
        batch_size=64,
        n_epochs=10,
        gamma=0.99,
        seed=SEED,
        policy_kwargs={
            "net_arch": dict(pi=[64, 64], vf=[64, 64]),
            "activation_fn": torch.nn.Tanh,
        },
        verbose=1,
        device="cpu",
    )

    print(f"Training PPO for {TOTAL_TIMESTEPS:,} timesteps...")
    model.learn(total_timesteps=TOTAL_TIMESTEPS)

    model.save(MODEL_PATH)
    print(f"\nSaved trained model: {MODEL_PATH}.zip")

    # Basic deterministic evaluation.
    rewards = []

    for episode in range(20):
        obs, _ = env.reset(seed=SEED + 1000 + episode)
        episode_reward = 0.0

        for _ in range(200):
            action, _ = model.predict(obs, deterministic=True)
            obs, reward, terminated, truncated, _ = env.step(action)
            episode_reward += float(reward)

            if terminated or truncated:
                break

        rewards.append(episode_reward)

    print("\nEvaluation over 20 episodes")
    print(f"Mean reward: {np.mean(rewards):.4f}")
    print(f"Std deviation: {np.std(rewards):.4f}")

    env.close()


if __name__ == "__main__":
    main()
