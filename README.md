# ⚡ Energy-Aware Adaptive CPU Scheduling for Heterogeneous Multicore Edge Devices Using DRL

[![Streamlit App](https://static.streamlit.io/badges/streamlit_badge_black_white.svg)](https://energy-aware-drl-scheduler-hapt7r2bbmvnjtkpkjyceb.streamlit.app/)

## Abstract
This project presents a high-performance simulation prototype for dynamic CPU core scheduling on ARM big.LITTLE heterogeneous architectures using Deep Reinforcement Learning (Proximal Policy Optimization - PPO). The scheduler optimizes task queue latency, power draw, and thermal throttling in real-time.

To bridge the gap between DRL training and real-world system execution, trained policy parameters are exported directly into static C floating-point arrays (`weights.h`). Policy inference and physical hardware emulation execute natively inside compiled C bytecode (`main.c`), eliminating Python runtime overhead during execution.

---

## 🚀 Live Interactive Demo

Click the button below to launch the live benchmark dashboard hosted on Streamlit Cloud:

[![Open in Streamlit](https://static.streamlit.io/badges/streamlit_badge_black_white.svg)](https://energy-aware-drl-scheduler-hapt7r2bbmvnjtkpkjyceb.streamlit.app/)

---

## 🏗️ Project Architecture

* **Environment:** Custom Gymnasium environment (`EdgeSchedulerEnv`) modeling ARM big.LITTLE core power/frequency states.
* **Offline Training:** Python with Stable-Baselines3 (PPO) extracts optimal scheduling policies.
* **C Native Engine:** `weights.h` stores policy weights for direct matrix forward-pass calculations ($W \cdot X + B$) in pure C.
* **Evaluation:** Benchmarked against standard Linux `performance` governor policies with dynamic TDP scaling (5W to 65W+).

---

## 💻 Local Setup & Execution

To compile and run the native C scheduler binary locally on a Linux machine:

```bash
# 1. Clone the repository
git clone [https://github.com/aditya1488/energy-aware-drl-scheduler.git](https://github.com/aditya1488/energy-aware-drl-scheduler.git)
cd energy-aware-drl-scheduler

# 2. Compile the C binary using Makefile
make

# 3. Execute scheduler with custom args: ./scheduler [Peak TDP (W)] [Init Queue Load] [Timesteps]
./scheduler 15.0 0.5 100

# 4. (Optional) Run the local Streamlit dashboard
pip install -r requirements.txt
streamlit run app.py
