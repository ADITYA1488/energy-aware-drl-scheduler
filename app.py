import os
import subprocess
import matplotlib.pyplot as plt
import streamlit as st

st.set_page_config(
    page_title="Custom Energy-Aware CPU Scheduler",
    page_icon="⚡",
    layout="wide",
)

st.title("⚡ Customizable OS CPU Scheduler Benchmark")
st.markdown(
    """
Configure any CPU profile below to benchmark the DRL C-Engine scheduler against 
the Linux `performance` governor for custom hardware configurations.
"""
)

# --- Sidebar Controls ---
st.sidebar.header("⚙️ Target Hardware Config")

preset = st.sidebar.selectbox(
    "Load Hardware Preset",
    ["Custom", "Raspberry Pi 4 (5W)", "Mobile / ARM big.LITTLE (15W)", "Desktop CPU (65W)"],
)

if preset == "Raspberry Pi 4 (5W)":
    default_pwr = 5.0
elif preset == "Desktop CPU (65W)":
    default_pwr = 65.0
else:
    default_pwr = 15.0

max_power = st.sidebar.slider("CPU Peak TDP (Watts)", 1.0, 100.0, default_pwr)
initial_queue = st.sidebar.slider("Initial Task Queue Load", 0.0, 1.0, 0.5)
sim_steps = st.sidebar.slider("Simulation Duration (Steps)", 50, 500, 100)

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

    run_btn = st.button("🚀 Run Custom Hardware Simulation", type="primary")

# --- Simulation Execution & Output ---
col1, col2 = st.columns([1, 1])

if run_btn:
    if not compiled:
        st.error("Cannot run simulation because C compilation failed.")
    else:
        # Pass custom sidebar values directly as CLI arguments to the C binary
        cmd = [c_binary, str(max_power), str(initial_queue), str(sim_steps)]
        result = subprocess.run(cmd, capture_output=True, text=True)
        raw_output = result.stdout

        with col1:
            st.subheader("🖥️ C Execution Output")
            st.code(raw_output, language="bash")

        perf_power = None
        drl_power = None
        savings = None

        for line in raw_output.split("\n"):
            if "Performance Governor Cumulative Energy:" in line:
                perf_power = float(line.split(":")[-1].replace("Joules", "").strip())
            elif "DRL C Scheduler Cumulative Energy:" in line:
                drl_power = float(line.split(":")[-1].replace("Joules", "").strip())
            elif "Energy Improvement:" in line:
                savings = float(line.split(":")[-1].replace("%", "").strip())

        with col2:
            st.subheader("📊 Performance Analysis")

            if perf_power is not None and drl_power is not None:
                m1, m2 = st.columns(2)
                m1.metric("Linux Performance Governor", f"{perf_power:.2f} J")
                m2.metric("DRL C-Engine Scheduler", f"{drl_power:.2f} J", delta=f"-{savings:.1f}% Energy")

                fig, ax = plt.subplots(figsize=(6, 4))
                categories = ["Performance Governor", "DRL C Scheduler"]
                energies = [perf_power, drl_power]
                colors = ["#d62728", "#1f77b4"]

                bars = ax.bar(categories, energies, color=colors, width=0.5)
                ax.set_ylabel("Cumulative Energy Consumption (Joules)")
                ax.set_title(f"Energy Draw Comparison ({max_power}W Profile)")
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
        st.info("👈 Adjust CPU profile parameters in the sidebar and click **Run Custom Hardware Simulation**.")
    with col2:
        st.markdown(
            """
        ### Hardware Customization Capabilities
        * **Adjustable TDP Limits:** Test low-power IoT chips (5W) up to high-performance desktop silicon (65W+).
        * **Variable Workload Backlogs:** Model initial task queue pressure across dynamic burst loads.
        * **Real-time C Argument Injection:** Streams runtime configuration directly into native compiled binary memory.
        """
        )
