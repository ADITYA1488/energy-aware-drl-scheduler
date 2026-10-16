
#include "drl_scheduler.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

/* Implemented in main.c */
extern int infer_drl_action(float queue, float temp, float core);

#define ACTION_COUNT 4

static const double POWER_RATIOS[ACTION_COUNT] = {
    0.08, 0.20, 0.48, 1.00
};

/*
 * Relative work rates for the four simulated operating points.
 * These are initial model assumptions, not measured CPU frequencies.
 */
static const double WORK_RATES[ACTION_COUNT] = {
    0.25, 0.50, 0.75, 1.00
};

static const double TEMP_RISE[ACTION_COUNT] = {
    0.01, 0.02, 0.05, 0.10
};

static double clamp01(double value)
{
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
}

int scheduler_run_drl(
    Task *tasks,
    size_t task_count,
    double tick_duration_seconds,
    double max_power_watts,
    double power_cap_ratio,
    DrlSchedulerMetrics *result)
{
    if (result == NULL ||
        (task_count > 0 && tasks == NULL) ||
        !isfinite(tick_duration_seconds) ||
        !isfinite(max_power_watts) ||
        !isfinite(power_cap_ratio) ||
        tick_duration_seconds <= 0.0 ||
        max_power_watts <= 0.0 ||
        power_cap_ratio <= 0.0 ||
        power_cap_ratio > 1.0) {
        return -1;
    }

    memset(result, 0, sizeof(*result));

    for (size_t i = 0; i < task_count; ++i) {
        if (!isfinite(tasks[i].remaining_work) ||
            tasks[i].remaining_work <= 0.0) {
            return -1;
        }

        for (size_t j = i + 1; j < task_count; ++j) {
            if (tasks[i].id == tasks[j].id) {
                return -1;
            }
        }

        tasks[i].status = TASK_READY;
        tasks[i].total_run_ticks = 0;
        tasks[i].first_start_tick = 0;
        tasks[i].completion_tick = 0;
    }

    if (task_count == 0) {
        return 0;
    }

    double temperature = 0.30;
    double active_core = 0.0;
    uint64_t tick = 0;
    size_t completed = 0;

    while (completed < task_count) {
        size_t selected = task_count;
        size_t ready_count = 0;

        /* Select the earliest-arriving ready task; break ties by ID. */
        for (size_t i = 0; i < task_count; ++i) {
            if (tasks[i].status == TASK_READY &&
                tasks[i].arrival_tick <= tick) {
                ready_count++;

                if (selected == task_count ||
                    tasks[i].arrival_tick < tasks[selected].arrival_tick ||
                    (tasks[i].arrival_tick == tasks[selected].arrival_tick &&
                     tasks[i].id < tasks[selected].id)) {
                    selected = i;
                }
            }
        }

        /* Advance over idle time if no task has arrived yet. */
        if (selected == task_count) {
            uint64_t next_arrival = UINT64_MAX;

            for (size_t i = 0; i < task_count; ++i) {
                if (tasks[i].status == TASK_READY &&
                    tasks[i].arrival_tick < next_arrival) {
                    next_arrival = tasks[i].arrival_tick;
                }
            }

            if (next_arrival == UINT64_MAX || next_arrival < tick) {
                return -1;
            }

            tick = next_arrival;
            continue;
        }

        Task *task = &tasks[selected];

        float queue_load = clamp01(
            (double)ready_count / (double)task_count);

        int action = infer_drl_action(
            queue_load,
            (float)temperature,
            (float)active_core);

        if (action < 0 || action >= ACTION_COUNT) {
            return -1;
        }

        double power = POWER_RATIOS[action] * max_power_watts;
        double power_limit = power_cap_ratio * max_power_watts;

        if (power > power_limit) {
            power = power_limit;
        }

        if (task->first_start_tick == 0 && tick != 0) {
            task->first_start_tick = tick;
        } else if (tick == 0 && task->total_run_ticks == 0) {
            task->first_start_tick = 0;
        }

        task->status = TASK_RUNNING;

        /*
         * Execute one simulated tick. A task can finish partway
         * through the tick; energy is charged for the full tick here.
         */
        double work_done = WORK_RATES[action];
        if (work_done > task->remaining_work) {
            work_done = task->remaining_work;
        }

        task->remaining_work -= work_done;
        task->total_run_ticks++;

        tick++;

        result->simulated_energy_joules += power * tick_duration_seconds;
        result->simulated_power_watts += power;
        temperature = clamp01(temperature + TEMP_RISE[action] - 0.02);
        active_core = (action >= 2) ? 1.0 : 0.0;

        if (task->remaining_work <= 0.000001) {
            task->remaining_work = 0.0;
            task->status = TASK_COMPLETED;
            task->completion_tick = tick;
            completed++;

            result->completed_tasks++;
            result->total_turnaround_time +=
                (double)(task->completion_tick - task->arrival_tick);

            result->total_waiting_time +=
                (double)(task->completion_tick - task->arrival_tick) -
                (double)task->total_run_ticks;

            if (task->deadline_tick != 0 &&
                task->completion_tick > task->deadline_tick) {
                result->deadline_misses++;
            }
        } else {
            task->status = TASK_READY;
        }
    }

    result->elapsed_ticks = tick;

    if (tick > 0) {
        result->simulated_power_watts =
            result->simulated_energy_joules /
            ((double)tick * tick_duration_seconds);
    }

    return 0;
}
