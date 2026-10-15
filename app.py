import os
import subprocess
import matplotlib.pyplot as plt
import streamlit as st

st.set_page_config(
    page_title="Energy-Aware DRL CPU Scheduler",
    page_icon="⚡",
    layout="wide",
)

st.title("⚡ Research-Enhanced DRL OS CPU Scheduler")
st.markdown(
    """
Benchmark a C-native DRL scheduler incorporating **power capping**, **multi-deadline slack tracking**, 
and **joint cluster/DVFS control** against baseline Linux governor policies.
"""
)

# --- Sidebar Controls ---
st.sidebar.header("⚙️ System & Research Configuration")

preset = st.sidebar.selectbox(
    "Hardware Preset",
    [
        "Custom",
        "Raspberry Pi 4 (5W Edge)",
        "Mobile ARM big.LITTLE (15W)",
        "Desktop CPU (65W)",
    ],
)

if preset == "Raspberry Pi 4 (5W Edge)":
    default_pwr = 5.0
elif preset == "Desktop CPU (65W)":
    default_pwr = 65.0
else:
    default_pwr = 15.0

max_power = st.sidebar.slider("CPU Peak TDP (Watts)", 1.0, 100.0, default_pwr)
power_cap = st.sidebar.slider(
    "Dynamic Power Cap Ratio (%) [Wang et al.]", 30, 100, 80
)
initial_queue = st.sidebar.slider("Initial Task Queue Load", 0.0, 1.0, 0.5)
sim_steps = st.sidebar.slider("Simulation Timesteps", 50, 500, 100)

c_binary = "./scheduler"
c_source = "main.c"


def compile_c_engine():
    if not os.path.exists(c_source):
        return False, f"Source file '{c_source}' not found."

    cmd = ["gcc", "-O3", "-std=c99", c_source, "-o", "scheduler", "-lm"]
    res = subprocess.run(cmd, capture_output=True, text=True)

    if res.returncode == 0:
        return True, "Compilation successful!"
    else:
        return False, res.stderr


with st.sidebar:
    st.markdown("---")
    st.subheader("C Toolchain Status")
    compiled, msg = compile_c_engine()
    if compiled:
        st.success("GCC Binary Ready (`./scheduler`)")
    else:
        st.error(f"Compilation Error:\n{msg}")

    run_btn = st.button("🚀 Run Research Benchmark", type="primary")

# --- Simulation Execution ---
col1, col2 = st.columns([1, 1])

if run_btn:
    if not compiled:
        st.error("Cannot run simulation because C compilation failed.")
    else:
        # Pass parameters including power cap to C binary
        cmd = [
            c_binary,
            str(max_power),
            str(initial_queue),
            str(sim_steps),
            str(power_cap),
        ]
        result = subprocess.run(cmd, capture_output=True, text=True)
        raw_output = result.stdout

        with col1:
            st.subheader("🖥️ C Engine Runtime Console")
            st.code(raw_output, language="bash")

        perf_power = None
        drl_power = None
        savings = None

        for line in raw_output.split("\n"):
            if "Performance Governor Cumulative Energy:" in line:
                perf_power = float(
                    line.split(":")[-1].replace("Joules", "").strip()
                )
            elif "DRL C Scheduler Cumulative Energy:" in line:
                drl_power = float(
                    line.split(":")[-1].replace("Joules", "").strip()
                )
            elif "Energy Improvement:" in line:
                savings = float(line.split(":")[-1].replace("%", "").strip())

        with col2:
            st.subheader("📊 Comparative Energy Analysis")

            if perf_power is not None and drl_power is not None:
                m1, m2 = st.columns(2)
                m1.metric("Linux Performance Governor", f"{perf_power:.2f} J")
                m2.metric(
                    "DRL C-Engine Scheduler",
                    f"{drl_power:.2f} J",
                    delta=f"-{savings:.1f}% Energy",
                )

                fig, ax = plt.subplots(figsize=(6, 4))
                categories = [
                    "Performance Governor",
                    f"DRL C Scheduler ({power_cap}% Cap)",
                ]
                energies = [perf_power, drl_power]
                colors = ["#d62728", "#1f77b4"]

                bars = ax.bar(categories, energies, color=colors, width=0.45)
                ax.set_ylabel("Cumulative Energy Consumption (Joules)")
                ax.set_title(f"Energy Benchmark ({max_power}W Profile)")
                ax.grid(axis="y", linestyle="--", alpha=0.5)

                for bar in bars:
                    yval = bar.get_height()
                    ax.text(
                        bar.get_x() + bar.get_width() / 2.0,
                        yval + (max(energies) * 0.02),
                        f"{yval:.2f} J",
                        ha="center",
                        va="bottom",
                        fontweight="bold",
                    )

                st.pyplot(fig)
else:
    with col1:
        st.info(
            "👈 Adjust target hardware parameters and power caps in the sidebar, then click **Run Research Benchmark**."
        )
    with col2:
        st.markdown(
            """
        ### Active Research Integration Features
        * **Workload-Phase Power Capping:** Dynamically constrains CPU power draw to user-specified ceilings [Wang et al. 2022].
        * **Multi-Deadline Slack Tracking:** Evaluates queue urgency to prevent latency violations [Li et al. 2025].
        * **Joint Cluster & Operating Point Control:** Maps discrete core choice and uncore frequency in a single feed-forward step [Wang 2025].
        """
        )
