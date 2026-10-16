#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef enum {
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_COMPLETED,
    TASK_REJECTED
} TaskStatus;

typedef struct {
    uint32_t id;
    uint64_t arrival_tick;
    uint64_t total_run_ticks;
    double remaining_work;
    int priority;
    uint64_t deadline_tick;

    uint64_t first_start_tick;
    uint64_t completion_tick;

    TaskStatus status;
} Task;

#endif