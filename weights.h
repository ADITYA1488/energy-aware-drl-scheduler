import torch
import numpy as np
from stable_baselines3 import PPO

# Load trained PPO model
model = PPO.load("ppo_edge_scheduler.zip")

# Extract policy weights and biases
policy_net = model.policy.mlp_extractor.policy_net
action_net = model.policy.action_net

w1 = policy_net[0].weight.detach().cpu().numpy()
b1 = policy_net[0].bias.detach().cpu().numpy()
w2 = policy_net[2].weight.detach().cpu().numpy()
b2 = policy_net[2].bias.detach().cpu().numpy()
w_out = action_net.weight.detach().cpu().numpy()
b_out = action_net.bias.detach().cpu().numpy()

def array_to_c(name, arr):
    if arr.ndim == 1:
        elems = ", ".join(f"{x:.6f}f" for x in arr)
        return f"const float {name}[{arr.shape[0]}] = {{{elems}}};\n"
    elif arr.ndim == 2:
        rows = [ "{" + ", ".join(f"{x:.6f}f" for x in row) + "}" for row in arr ]
        return f"const float {name}[{arr.shape[0]}][{arr.shape[1]}] = {{\n  " + ",\n  ".join(rows) + "\n};\n"

# Generate the valid C header file
with open("weights.h", "w") as f:
    f.write("#ifndef WEIGHTS_H\n#define WEIGHTS_H\n\n")
    f.write(array_to_c("B1", b1))
    f.write(array_to_c("W1", w1))
    f.write(array_to_c("B2", b2))
    f.write(array_to_c("W2", w2))
    f.write(array_to_c("B_OUT", b_out))
    f.write(array_to_c("W_OUT", w_out))
    f.write("\n#endif\n")

print("✅ Generated weights.h file! Download it from Colab files.")
