import streamlit as st
import numpy as np
import matplotlib.pyplot as plt
import gymnasium as gym
from stable_baselines3 import PPO
from edge_env import EdgeSchedulerEnv

st.title("⚡ Energy-Aware Adaptive CPU Scheduler (DRL)")
st.markdown("Dynamic CPU core cluster & frequency selection using Proximal Policy Optimization (PPO).")

# Sidebar Controls
st.sidebar.header("Simulation Settings")
steps = st.sidebar.slider("Simulation Steps", min_value=10, max_value=200, value=50)
run_sim = st.sidebar.button("Run Simulation")

if run_sim:
    env = EdgeSchedulerEnv()
    model = PPO.load("ppo_edge_scheduler.zip")
    
    drl_pwr, base_pwr = [], []
    obs, _ = env.reset(seed=42)
    
    for _ in range(steps):
        action, _ = model.predict(obs, deterministic=True)
        obs, _, _, _, info = env.step(action)
        drl_pwr.append(info["power"])
        
    obs, _ = env.reset(seed=42)
    for _ in range(steps):
        obs, _, _, _, info = env.step(3) # Max Perf Baseline
        base_pwr.append(info["power"])
        
    fig, ax = plt.subplots(figsize=(8, 3))
    ax.plot(drl_pwr, label="DRL Scheduler", color="#1f77b4")
    ax.plot(base_pwr, label="Baseline Governor", color="#d62728", linestyle="--")
    ax.set_ylabel("Power (Watts)")
    ax.set_xlabel("Time Step")
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    st.pyplot(fig)
    st.metric(label="Average Power Reduction", value=f"{((np.mean(base_pwr) - np.mean(drl_pwr)) / np.mean(base_pwr)) * 100:.1f}%")
