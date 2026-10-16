#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "weights.h"

#define INPUT_DIM 3
#define HIDDEN_DIM 64
#define ACTION_DIM 4

// 1. Research-Backed Extended System State (Wang et al. 2022, Li et al. 2025)
typedef struct {
    float queue_load;       // Backlog load ratio [0.0 - 1.0]
    float temp;             // Core temperature ratio [0.0 - 1.0]
    float active_core;      // 0.0 = LITTLE cluster, 1.0 = big cluster
    float deadline_slack;   // Multi-deadline tracking [0.0 = urgent, 1.0 = ample]
    float total_power;      // Cumulative energy tracker (Joules)
} SystemState;

float tanh_activation(float x)
{
    return tanhf(x);
}

// Zero-Overhead Forward Pass Inference (Zhou & Lin 2023)
int infer_drl_action(float queue, float temp, float core) {
    float input[INPUT_DIM] = {queue, temp, core};
    float h1[HIDDEN_DIM];
    float h2[HIDDEN_DIM];
    float output[ACTION_DIM];

    for (int i = 0; i < HIDDEN_DIM; i++) {
        float sum = B1[i];
        for (int j = 0; j < INPUT_DIM; j++) {
            sum += W1[i][j] * input[j];
        }
        h1[i] = tanh_activation(sum);
    }

    for (int i = 0; i < HIDDEN_DIM; i++) {
        float sum = B2[i];
        for (int j = 0; j < HIDDEN_DIM; j++) {
            sum += W2[i][j] * h1[j];
        }
        h2[i] = tanh_activation(sum);
    }

    int best_action = 0;
    float max_logit = -1e9f;
    for (int i = 0; i < ACTION_DIM; i++) {
        float sum = B_OUT[i];
        for (int j = 0; j < HIDDEN_DIM; j++) {
            sum += W_OUT[i][j] * h2[j];
        }
        output[i] = sum;
        if (output[i] > max_logit) {
            max_logit = output[i];
            best_action = i;
        }
    }
    return best_action;
}

// 2. Hardware Engine with Research Features (Wang 2025, Pivezhandi et al. 2026)
void step_hardware(SystemState *sys, int action, float *pwr, float max_power_watt, float power_cap_ratio) {
    // Joint Action Mapping: Core Cluster & Operating Point (Wang 2025)
    // Action 0: LITTLE core (Min Freq) + Low Uncore Interconnect
    // Action 1: LITTLE core (Max Freq) + Med Uncore Interconnect
    // Action 2: big core    (Med Freq) + High Uncore Interconnect
    // Action 3: big core    (Max Freq) + Peak Uncore Interconnect
    float power_ratios[4] = {0.08f, 0.20f, 0.48f, 1.00f};
    float caps[4]         = {0.20f, 0.40f, 0.70f, 1.00f};
    float temps[4]        = {0.01f, 0.02f, 0.05f, 0.10f};

    // 3. Deadline-Aware Priority Override (Li et al. 2025)
    // If slack drops below critical threshold (20%), force high-performance state
    if (sys->deadline_slack < 0.20f && sys->queue_load > 0.4f) {
        action = 3; 
    }

    // 4. Safe Runtime Fallback Capping (Wang et al. 2022)
    // Enforce dynamic power capping ceiling on calculated power draw
    float calculated_pwr = power_ratios[action] * max_power_watt;
    float max_allowed_pwr = max_power_watt * power_cap_ratio;

    if (calculated_pwr > max_allowed_pwr) {
        calculated_pwr = max_allowed_pwr; // Thermal / Power Cap Clamping
    }

    *pwr = calculated_pwr;
    sys->total_power += *pwr;

    // Workload Dynamics & Deadline Slack Decay (Peng et al. 2024)
    float arrival = ((float)rand() / RAND_MAX) * 0.3f + 0.1f;
    sys->queue_load += arrival - caps[action];
    if (sys->queue_load < 0.0f) sys->queue_load = 0.0f;
    if (sys->queue_load > 1.0f) sys->queue_load = 1.0f;

    // Slack updates: high processing speeds restore deadline slack
    sys->deadline_slack += (caps[action] - arrival);
    if (sys->deadline_slack < 0.0f) sys->deadline_slack = 0.0f;
    if (sys->deadline_slack > 1.0f) sys->deadline_slack = 1.0f;

    // Thermal dissipation updates
    sys->temp += temps[action] - 0.02f;
    if (sys->temp < 0.0f) sys->temp = 0.0f;
    if (sys->temp > 1.0f) sys->temp = 1.0f;

    sys->active_core = (action >= 2) ? 1.0f : 0.0f;
}

int main(int argc, char *argv[]) {
    // Command-line arguments passed from Streamlit
    float max_power_watt = (argc > 1) ? atof(argv[1]) : 15.0f;
    float init_queue     = (argc > 2) ? atof(argv[2]) : 0.5f;
    int steps            = (argc > 3) ? atoi(argv[3]) : 100;
    float power_cap_pct  = (argc > 4) ? (atof(argv[4]) / 100.0f) : 1.0f; // Default: 100% (No Cap)

    printf("=== RESEARCH-ENHANCED DRL CPU SCHEDULER ===\n");
    printf("Target Peak TDP:       %.2f W\n", max_power_watt);
    printf("Dynamic Power Cap:     %.0f%%\n", power_cap_pct * 100.0f);
    printf("Initial Task Backlog:  %.2f\n", init_queue);
    printf("Simulation Timesteps:  %d\n\n", steps);

    // 1. DRL Scheduler Execution
    SystemState drl_sys = {init_queue, 0.3f, 0.0f, 0.8f, 0.0f};
    srand(42);
    for (int t = 0; t < steps; t++) {
        int action = infer_drl_action(drl_sys.queue_load, drl_sys.temp, drl_sys.active_core);
        float pwr;
        step_hardware(&drl_sys, action, &pwr, max_power_watt, power_cap_pct);
    }

    // 2. Linux Performance Governor Execution (Baseline)
    SystemState perf_sys = {init_queue, 0.3f, 0.0f, 0.8f, 0.0f};
    srand(42);
    for (int t = 0; t < steps; t++) {
        float pwr;
        step_hardware(&perf_sys, 3, &pwr, max_power_watt, 1.0f); // Uncapped max performance
    }

    // Print Benchmark Metrics
    printf("Performance Governor Cumulative Energy: %.2f Joules\n", perf_sys.total_power);
    printf("DRL C Scheduler Cumulative Energy:       %.2f Joules\n", drl_sys.total_power);

    float savings = ((perf_sys.total_power - drl_sys.total_power) / perf_sys.total_power) * 100.0f;
    printf("⚡ Energy Improvement:                    %.1f%%\n", savings);

    return 0;
}
