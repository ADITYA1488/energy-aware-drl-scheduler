
from stable_baselines3 import PPO
from pathlib import Path
import numpy as np

MODEL_PATH = "ppo_edge_scheduler.zip"
OUTPUT_PATH = "weights.h"

model = PPO.load(MODEL_PATH, device="cpu")
state = model.policy.state_dict()

# Extract the trained policy network parameters.
arrays = {
    "W1": state["mlp_extractor.policy_net.0.weight"].cpu().numpy(),
    "B1": state["mlp_extractor.policy_net.0.bias"].cpu().numpy(),
    "W2": state["mlp_extractor.policy_net.2.weight"].cpu().numpy(),
    "B2": state["mlp_extractor.policy_net.2.bias"].cpu().numpy(),
    "W_OUT": state["action_net.weight"].cpu().numpy(),
    "B_OUT": state["action_net.bias"].cpu().numpy(),
}

expected_shapes = {
    "W1": (64, 3),
    "B1": (64,),
    "W2": (64, 64),
    "B2": (64,),
    "W_OUT": (4, 64),
    "B_OUT": (4,),
}

for name, expected in expected_shapes.items():
    if arrays[name].shape != expected:
        raise ValueError(
            f"{name}: expected {expected}, got {arrays[name].shape}"
        )
    if not np.isfinite(arrays[name]).all():
        raise ValueError(f"{name} contains non-finite values")

def format_array(name, array):
    flat = array.flatten()
    values = ", ".join(f"{float(v):.9g}f" for v in flat)

    if array.ndim == 1:
        return f"static const float {name}[{array.shape[0]}] = {{{values}}};"

    rows, cols = array.shape
    row_strings = []
    for r in range(rows):
        row = ", ".join(f"{float(v):.9g}f" for v in array[r])
        row_strings.append("    {" + row + "}")

    return (
        f"static const float {name}[{rows}][{cols}] = {{\n"
        + ",\n".join(row_strings)
        + "\n};"
    )

header = [
    "#ifndef WEIGHTS_H",
    "#define WEIGHTS_H",
    "",
    "/* Exported from Stable-Baselines3 PPO policy. */",
    "",
]

for name in ("B1", "W1", "B2", "W2", "B_OUT", "W_OUT"):
    header.append(format_array(name, arrays[name]))
    header.append("")

header.append("#endif")
header.append("")

Path(OUTPUT_PATH).write_text("\n".join(header), encoding="utf-8")

print(f"Exported policy weights to {OUTPUT_PATH}")
for name in ("W1", "B1", "W2", "B2", "W_OUT", "B_OUT"):
    print(f"{name}: shape={arrays[name].shape}")
