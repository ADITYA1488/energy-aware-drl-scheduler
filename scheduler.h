#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stddef.h>
#include <stdint.h>
#include "task.h"

typedef enum {
    POLICY_FCFS = 0,
    POLICY_ROUND_ROBIN
} SchedulingPolicy;

typedef struct {
    uint64_t current_tick;
    size_t completed_tasks;
    size_t deadline_misses;
    double total_waiting_time;
    double total_turnaround_time;
} SchedulerMetrics;

void scheduler_init(void);
void scheduler_reset_metrics(void);

const SchedulerMetrics *scheduler_get_metrics(void);
const char *scheduler_policy_name(SchedulingPolicy policy);

int scheduler_run_fcfs(Task *tasks, size_t task_count);
int scheduler_run_round_robin(
    Task *tasks,
    size_t task_count,
    uint32_t quantum
    
);

#endif