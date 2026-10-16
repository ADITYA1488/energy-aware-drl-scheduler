# Energy-Aware Adaptive CPU Scheduling Using Deep Reinforcement Learning

An experimental software-only project exploring how Proximal Policy Optimization (PPO) can make adaptive scheduling decisions under simulated workload, power-cost, and thermal constraints.

> **Project status:** Research prototype in development. Power, workload, and temperature values are simulated. This project does not currently demonstrate measured hardware energy savings or control the real Windows/Linux operating-system scheduler.

## Overview

CPU scheduling involves balancing competing objectives: completing work, keeping queues manageable, and avoiding excessive power consumption or thermal stress.

This project explores whether a reinforcement learning agent can make adaptive scheduling decisions in a simulated heterogeneous-core environment and how its behavior compares with simpler scheduling policies.

The workflow combines:

- A custom environment built with Gymnasium.
- PPO policy training using Stable-Baselines3.
- Evaluation and diagnostic scripts for understanding learned behavior.
- Policy comparisons against fixed and heuristic baselines.
- A native C scheduling implementation for software-level experimentation.

## Objectives

1. Model workload and resource-management trade-offs in a reproducible simulation.
2. Train a PPO agent to select among discrete scheduling actions.
3. Compare the learned policy against fixed-action and adaptive-heuristic baselines.
4. Examine throughput, simulated power cost, queue load, dropped work, and temperature.
5. Explore deployment of policy inference in C, with correctness checked against the Python implementation.

## Architecture

```text
Simulated workload
       |
       v
Gymnasium environment
       |
       v
State / observation
       |
       v
PPO policy (Stable-Baselines3)
       |
       v
Discrete scheduling action
       |
       v
Simulated workload and thermal model
       |
       v
Reward + performance metrics
       |
       v
Policy evaluation and baseline comparison
```

The repository also contains native C scheduling code for exploring conventional scheduling and C-based policy execution.

## Technology Stack

| Technology | Purpose |
|---|---|
| Python | Environment, training, and evaluation |
| Gymnasium | Reinforcement learning environment interface |
| Stable-Baselines3 | PPO implementation |
| NumPy | Numerical computation |
| C | Native scheduling implementation |
| GCC / MinGW | C compilation on supported development setups |

## Repository Guide

| File | Purpose |
|---|---|
| `edge_env.py` | Simulated scheduling environment |
| `train_model.py` | Train the PPO agent |
| `ppo_edge_scheduler_trained.zip` | Saved trained PPO model |
| `evaluate_model.py` | Evaluate the trained policy |
| `compare_adaptive_policies.py` | Compare PPO with fixed and heuristic policies |
| `diagnose_policy.py` | Inspect action-selection behavior |
| `inspect_policy_grid.py` | Explore policy decisions across selected observations |
| `trace_policy.py` | Trace actions across simulated episodes |
| `compare_all_policies.c` | Native C policy/scheduling comparison |
| `drl_scheduler.c` | Native C scheduling implementation |

Additional C source and header files support the native scheduling components.

## Getting Started

### 1. Clone the repository

```powershell
git clone https://github.com/ADITYA1488/energy-aware-drl-scheduler.git
cd energy-aware-drl-scheduler
```

### 2. Create a virtual environment

On Windows PowerShell:

```powershell
py -3.12 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
```

Install the dependencies declared in the repository:

```powershell
pip install -r requirements.txt
```

If the dependency file does not include every package used by the training scripts, install the missing packages listed in the project's Python imports.

### 3. Train the PPO model

```powershell
python train_model.py
```

Training creates or updates the saved model according to the configuration in `train_model.py`. Training duration and results depend on the environment, random seed, and hyperparameters.

### 4. Evaluate the model

```powershell
python evaluate_model.py
python compare_adaptive_policies.py
python diagnose_policy.py
python inspect_policy_grid.py
python trace_policy.py
```

Run these scripts individually. They serve different purposes: evaluation, baseline comparison, action-distribution analysis, and policy diagnostics.

### 5. Native C experiments

The repository includes C source files for scheduling and policy comparisons. A GCC-compatible compiler is required. Use the source files and headers in the repository when compiling; the native C implementation should be treated as a software simulation unless a separate real-system integration is implemented and validated.

## Evaluation Metrics

The project examines:

- **Episode reward:** The cumulative objective defined by the simulation.
- **Completed work:** Amount of work processed by the simulated scheduler.
- **Simulated power cost:** A model-defined cost, not a wattmeter measurement.
- **Dropped work:** Work that exceeds the modeled queue capacity.
- **Mean queue load:** An indicator of accumulated pending work.
- **Temperature:** A simulated thermal state.
- **Action distribution:** How frequently the agent selects each available action.

Comparisons should use the same workload assumptions, episode lengths, and random seeds wherever possible. Results should be reported across multiple runs, including relevant variability and limitations.

## Current Findings

In one high-load stress-test experiment using 50 evaluation seeds, the PPO policy produced the following results:

| Metric | PPO | Always action 1 | Adaptive heuristic |
|---|---:|---:|---:|
| Mean episode reward | -50.912 | -256.609 | -215.965 |
| Simulated power cost per step | 0.941 | 0.500 | 0.589 |
| Completed work | 79.320 | 63.993 | 67.927 |
| Dropped work | 0.000 | 14.354 | 10.421 |
| Mean queue load | 0.039 | 0.976 | 0.804 |
| Mean simulated temperature | 0.901 | 0.281 | 0.709 |

These figures are from a particular simulated workload configuration and are not universal performance guarantees.

The experiment suggests that PPO can improve simulated throughput and queue management under the tested workload. However, its higher simulated power cost and elevated mean temperature show that the current reward function does not yet establish a satisfactory energy–thermal balance.

**No claim of real-world energy savings, operating-system scheduler replacement, or physical thermal safety is made.**

## Limitations and Next Steps

- Strengthen and validate thermal constraints in the reward and environment dynamics.
- Track maximum episode temperature in addition to mean temperature.
- Compare policies under identical workloads and multiple reproducible seeds.
- Validate Python-to-C policy inference parity for the current model.
- Separate simulated performance metrics from real hardware measurements.
- Document the native C build and test workflow.
- Evaluate generalization to workloads not used during training.

## Research Direction

The project investigates the use of reinforcement learning for adaptive resource scheduling. It is an experimental prototype rather than a validated production scheduler. Further work is needed to establish robust baselines, reproducibility, constraint handling, and generalization before drawing stronger conclusions.

## License

See the repository's license file for the terms governing use and distribution.
