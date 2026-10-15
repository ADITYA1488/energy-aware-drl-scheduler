import os
import subprocess
import matplotlib.pyplot as plt
import numpy as np
import streamlit as st

st.set_page_config(
    page_title="Energy-Aware DRL CPU Scheduler",
    page_icon="⚡",
    layout="wide",
)

st.title("⚡ Energy-Aware OS CPU Scheduler (C Engine)")
st.markdown(
    """
This dashboard benchmarks a **C-Based Real-Time Neural Network CPU Scheduler** against standard 
Linux OS governor policies on ARM big.LITTLE architectures. Policy inference and physical hardware simulation 
run directly inside compiled C bytecode (`main.c` & `weights.h`).
"""
)

st.sidebar.header("🕹️ Simulation Controls")

# Compile C code on app launch/interaction
c_binary = "./scheduler"
c_source = "main.c"


def compile_c_engine():
    if not os.path.exists(c_source):
        return False, f"Source file '{c_source}' not found in repository."

    cmd = ["gcc", "-O3", "-std=c99", c_source, "-o", "scheduler", "-lm"]
    res = subprocess.run(cmd, capture_output=True, text=True)

    if res.returncode == 0:
        return True, "Compilation successful!"
    else:
        return False, res.stderr


# Sidebar Build Status
with st.sidebar:
    st.subheader("C Toolchain Status")
    compiled, msg = compile_c_engine()
    if compiled:
        st.success("GCC Binary: Ready (`./scheduler`)")
    else:
        st.error(f"GCC Compilation Error:\n{msg}")

    run_btn = st.button("🚀 Execute C Scheduler Engine", type="primary")

# Main Interface Layout
col1, col2 = st.columns([1, 1])

if run_btn:
    if not compiled:
        st.error("Cannot run simulation because C compilation failed.")
    else:
        # Run C binary
        result = subprocess.run([c_binary], capture_output=True, text=True)
        raw_output = result.stdout

        with col1:
            st.subheader("🖥️ C Runtime Console Output")
            st.code(raw_output, language="bash")

        # Parse C output for visualization
        # Standard expected metrics output parsing
        perf_power = None
        drl_power = None
        savings = None

        for line in raw_output.split("\n"):
            if "Performance Governor Power:" in line or "Performance Governor Cumulative Energy:" in line:
                perf_power = float(line.split(":")[-1].replace("Joules", "").replace("W", "").strip())
            elif "DRL C Scheduler Power:" in line or "DRL C Scheduler Cumulative Energy:" in line:
                drl_power = float(line.split(":")[-1].replace("Joules", "").replace("W", "").strip())
            elif "Energy Improvement:" in line or "Energy Efficiency Improvement:" in line:
                savings = float(line.split(":")[-1].replace("%", "").strip())

        with col2:
            st.subheader("📊 Performance & Energy Analysis")

            if perf_power is not None and drl_power is not None:
                # Key Metrics display
                m1, m2 = st.columns(2)
                m1.metric(
                    label="Linux Performance Governor",
                    value=f"{perf_power:.2f} J",
                )
                m2.metric(
                    label="DRL C-Engine Scheduler",
                    value=f"{drl_power:.2f} J",
                    delta=f"-{savings:.1f}% Energy",
                )

                # Comparative Bar Chart Plot
                fig, ax = plt.subplots(figsize=(6, 4))
                categories = ["Performance Governor", "DRL C Scheduler"]
                energies = [perf_power, drl_power]
                colors = ["#d62728", "#1f77b4"]

                bars = ax.bar(categories, energies, color=colors, width=0.5)
                ax.set_ylabel("Cumulative Power/Energy Consumption (Joules)")
                ax.set_title("Energy Draw Comparison")
                ax.grid(axis="y", linestyle="--", alpha=0.5)

                for bar in bars:
                    yval = bar.get_height()
                    ax.text(
                        bar.get_x() + bar.get_width() / 2.0,
                        yval + 0.5,
                        f"{yval:.2f} J",
                        ha="center",
                        va="bottom",
                        fontweight="bold",
                    )

                st.pyplot(fig)
            else:
                st.info(
                    "Simulation executed. Add more parse logic or review stdout if metric parsing missed."
                )

else:
    with col1:
        st.info("👈 Click **Execute C Scheduler Engine** in the sidebar to run the C binary.")
    with col2:
        st.markdown(
            """
        ### Architecture Highlights
        * **Pure C Inference Engine:** Performs neural network forward-pass matrix multiplication ($W_1 \cdot X + B_1$) without PyTorch overhead.
        * **Hardware Emulation:** Simulates task queue backlog updates and chip heat thermal dissipation models in C.
        * **Sub-Microsecond Latency:** Ready for low-latency embedded Linux/ARM deployments.
        """
        )
