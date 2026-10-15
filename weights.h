import torch
import numpy as np
from stable_baselines3 import PPO

# 1. Load your trained model
model = PPO.load("ppo_edge_scheduler.zip")

# 2. Extract policy network weights & biases
policy_net = model.policy.mlp_extractor.policy_net
action_net = model.policy.action_net

w1 = policy_net[0].weight.detach().cpu().numpy()  # [64, 3]
b1 = policy_net[0].bias.detach().cpu().numpy()    # [64]
w2 = policy_net[2].weight.detach().cpu().numpy()  # [64, 64]
b2 = policy_net[2].bias.detach().cpu().numpy()    # [64]
w_out = action_net.weight.detach().cpu().numpy()  # [4, 64]
b_out = action_net.bias.detach().cpu().numpy()    # [4]

def array_to_c(name, arr):
    if arr.ndim == 1:
        elems = ", ".join(f"{x:.6f}f" for x in arr)
        return f"const float {name}[{arr.shape[0]}] = {{{elems}}};\n"
    elif arr.ndim == 2:
        rows = []
        for row in arr:
            elems = ", ".join(f"{x:.6f}f" for x in row)
            rows.append(f"{{{elems}}}")
        return f"const float {name}[{arr.shape[0]}][{arr.shape[1]}] = {{\n  " + ",\n  ".join(rows) + "\n};\n"

# 3. Write C header file
with open("weights.h", "w") as f:
    f.write("#ifndef WEIGHTS_H\n#define WEIGHTS_H\n\n")
    f.write(array_to_c("W1", w1))
    f.write(array_to_c("B1", b1))
    f.write(array_to_c("W2", w2))
    f.write(array_to_c("B2", b2))
    f.write(array_to_c("W_OUT", w_out))
    f.write(array_to_c("B_OUT", b_out))
    f.write("\n#endif\n")

print("✅ Successfully generated weights.h!")
