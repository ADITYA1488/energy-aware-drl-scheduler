#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "weights.h"

#define STEPS 100
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

// C Forward Pass: Neural Network Policy Inference
int infer_drl_action(float queue, float temp, float core) {
    float input[INPUT_DIM] = {queue, temp, core};
    float h1[HIDDEN_DIM], h2[HIDDEN_DIM], output[ACTION_DIM];

    // Layer 1
    for (int i = 0; i < HIDDEN_DIM; i++) {
        float sum = B1[i];
        for (int j = 0; j < INPUT_DIM; j++) sum += W1[i][j] * input[j];
        h1[i] = relu(sum);
    }

    // Layer 2
    for (int i = 0; i < HIDDEN_DIM; i++) {
        float sum = B2[i];
        for (int j = 0; j < HIDDEN_DIM; j++) sum += W2[i][j] * h1[j];
        h2[i] = relu(sum);
    }

    // Output Layer
    int best_action = 0;
    float max_logit = -1e9f;
    for (int i = 0; i < ACTION_DIM; i++) {
        float sum = B_OUT[i];
        for (int j = 0; j < HIDDEN_DIM; j++) sum += W_OUT[i][j] * h2[j];
        output[i] = sum;
        if (output[i] > max_logit) {
            max_logit = output[i];
            best_action = i;
        }
    }
    return best_action;
}

// Hardware Emulation Step
void step_hardware(SystemState *sys, int action, float *pwr) {
    float powers[4] = {0.2f, 0.5f, 1.2f, 2.5f};
    float caps[4]   = {0.2f, 0.4f, 0.7f, 1.0f};
    float temps[4]  = {0.01f, 0.02f, 0.05f, 0.10f};

    *pwr = powers[action];
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

int main() {
    srand(42);
    printf("=== ENERGY-AWARE OS SCHEDULER (C RUNTIME) ===\n\n");

    // DRL Scheduler
    SystemState drl_sys = {0.5f, 0.3f, 0.0f, 0.0f};
    for (int t = 0; t < STEPS; t++) {
        int action = infer_drl_action(drl_sys.queue_load, drl_sys.temp, drl_sys.core);
        float pwr;
        step_hardware(&drl_sys, action, &pwr);
    }

    // Baseline Performance Governor
    SystemState perf_sys = {0.5f, 0.3f, 0.0f, 0.0f};
    srand(42);
    for (int t = 0; t < STEPS; t++) {
        float pwr;
        step_hardware(&perf_sys, 3, &pwr);
    }

    printf("Results over %d steps:\n", STEPS);
    printf("Performance Governor Power: %.2f W\n", perf_sys.total_power);
    printf("DRL C Scheduler Power:       %.2f W\n", drl_sys.total_power);
    printf("⚡ Energy Improvement:        %.1f%%\n", 
           ((perf_sys.total_power - drl_sys.total_power) / perf_sys.total_power) * 100.0f);

    return 0;
}
