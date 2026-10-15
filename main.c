#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "weights.h"

#define INPUT_DIM 3
#define HIDDEN_DIM 64
#define ACTION_DIM 4

typedef struct {
    float queue_load;
    float temp;
    float core;
    float total_power;
} SystemState;

float relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

// Forward Pass Inference
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
        h1[i] = relu(sum);
    }

    for (int i = 0; i < HIDDEN_DIM; i++) {
        float sum = B2[i];
        for (int j = 0; j < HIDDEN_DIM; j++) {
            sum += W2[i][j] * h1[j];
        }
        h2[i] = relu(sum);
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

// Hardware Step with User-Defined Max Power Scale
void step_hardware(SystemState *sys, int action, float *pwr, float max_power_scale) {
    // Action ratios scaled relative to user's specified max power
    float power_ratios[4] = {0.08f, 0.20f, 0.48f, 1.00f};
    float caps[4]         = {0.20f, 0.40f, 0.70f, 1.00f};
    float temps[4]        = {0.01f, 0.02f, 0.05f, 0.10f};

    *pwr = power_ratios[action] * max_power_scale;
    sys->total_power += *pwr;

    float arrival = ((float)rand() / RAND_MAX) * 0.3f + 0.1f;
    sys->queue_load += arrival - caps[action];
    if (sys->queue_load < 0.0f) sys->queue_load = 0.0f;
    if (sys->queue_load > 1.0f) sys->queue_load = 1.0f;

    sys->temp += temps[action] - 0.02f;
    if (sys->temp < 0.0f) sys->temp = 0.0f;
    if (sys->temp > 1.0f) sys->temp = 1.0f;

    sys->core = (action >= 2) ? 1.0f : 0.0f;
}

int main(int argc, char *argv[]) {
    // Default system parameters if CLI arguments aren't supplied
    float max_power_watt = (argc > 1) ? atof(argv[1]) : 15.0f;
    float init_queue     = (argc > 2) ? atof(argv[2]) : 0.5f;
    int steps            = (argc > 3) ? atoi(argv[3]) : 100;

    printf("=== CUSTOM ENERGY-AWARE CPU SCHEDULER ===\n");
    printf("Target CPU Peak Power: %.2f W\n", max_power_watt);
    printf("Initial Workload Queue: %.2f\n", init_queue);
    printf("Simulation Timesteps:   %d\n\n", steps);

    // 1. DRL Scheduler Execution
    SystemState drl_sys = {init_queue, 0.3f, 0.0f, 0.0f};
    srand(42);
    for (int t = 0; t < steps; t++) {
        int action = infer_drl_action(drl_sys.queue_load, drl_sys.temp, drl_sys.core);
        float pwr;
        step_hardware(&drl_sys, action, &pwr, max_power_watt);
    }

    // 2. Linux Performance Governor Execution
    SystemState perf_sys = {init_queue, 0.3f, 0.0f, 0.0f};
    srand(42);
    for (int t = 0; t < steps; t++) {
        float pwr;
        step_hardware(&perf_sys, 3, &pwr, max_power_watt);
    }

    // Print Results
    printf("Performance Governor Cumulative Energy: %.2f Joules\n", perf_sys.total_power);
    printf("DRL C Scheduler Cumulative Energy:       %.2f Joules\n", drl_sys.total_power);

    float savings = ((perf_sys.total_power - drl_sys.total_power) / perf_sys.total_power) * 100.0f;
    printf("⚡ Energy Improvement:                    %.1f%%\n", savings);

    return 0;
}
