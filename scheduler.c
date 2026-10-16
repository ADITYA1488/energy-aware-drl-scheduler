#include "scheduler.h"
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

static SchedulerMetrics metrics;

void scheduler_init(void)
{
    scheduler_reset_metrics();
}

void scheduler_reset_metrics(void)
{
    metrics.current_tick = 0;
    metrics.completed_tasks = 0;
    metrics.deadline_misses = 0;
    metrics.total_waiting_time = 0.0;
    metrics.total_turnaround_time = 0.0;
}

const SchedulerMetrics *scheduler_get_metrics(void)
{
    return &metrics;
}

const char *scheduler_policy_name(SchedulingPolicy policy)
{
    switch (policy) {
        case POLICY_FCFS:
            return "FCFS";
        case POLICY_ROUND_ROBIN:
            return "Round Robin";
        default:
            return "Unknown";
    }
}


int scheduler_run_fcfs(Task *tasks, size_t task_count)
{
    if (task_count > 0 && tasks == NULL) {
        return -1;
    }

    scheduler_reset_metrics();

    for (size_t i = 0; i < task_count; i++) {
        if (tasks[i].remaining_work <= 0.0) {
            return -1;
        }

        for (size_t j = i + 1; j < task_count; j++) {
            if (tasks[i].id == tasks[j].id) {
                return -1;
            }
        }

        tasks[i].status = TASK_READY;
        tasks[i].first_start_tick = 0;
        tasks[i].completion_tick = 0;
        tasks[i].total_run_ticks = 0;
    }

    size_t completed = 0;
    uint64_t tick = 0;

    while (completed < task_count) {
        size_t selected = task_count;
        uint64_t earliest_arrival = UINT64_MAX;

        for (size_t i = 0; i < task_count; i++) {
            if (tasks[i].status == TASK_READY &&
                tasks[i].arrival_tick <= tick &&
                (selected == task_count ||
                 tasks[i].arrival_tick < earliest_arrival ||
                 (tasks[i].arrival_tick == earliest_arrival &&
                  tasks[i].id < tasks[selected].id))) {
                selected = i;
                earliest_arrival = tasks[i].arrival_tick;
            }
        }

        if (selected == task_count) {
            uint64_t next_arrival = UINT64_MAX;

            for (size_t i = 0; i < task_count; i++) {
                if (tasks[i].status == TASK_READY &&
                    tasks[i].arrival_tick < next_arrival) {
                    next_arrival = tasks[i].arrival_tick;
                }
            }

            if (next_arrival == UINT64_MAX) {
                return -1;
            }

            tick = next_arrival;
            continue;
        }

        Task *task = &tasks[selected];

        task->status = TASK_RUNNING;
        task->first_start_tick = tick;

        metrics.total_waiting_time +=
            (double)(tick - task->arrival_tick);

        double work = task->remaining_work;

        if (work > (double)UINT64_MAX) {
            return -1;
        }

        uint64_t run_ticks = (uint64_t)work;
        if ((double)run_ticks < work) {
            run_ticks++;
        }

        if (UINT64_MAX - tick < run_ticks) {
            return -1;
        }

        tick += run_ticks;

        task->total_run_ticks = run_ticks;
        task->remaining_work = 0.0;
        task->completion_tick = tick;
        task->status = TASK_COMPLETED;

        metrics.total_turnaround_time +=
            (double)(task->completion_tick - task->arrival_tick);

        if (task->deadline_tick != 0 &&
            task->completion_tick > task->deadline_tick) {
            metrics.deadline_misses++;
        }

        metrics.completed_tasks++;
        completed++;
    }

    metrics.current_tick = tick;
    return 0;
}


int scheduler_run_round_robin(
    Task *tasks,
    size_t task_count,
    uint32_t quantum)
{
    if ((task_count > 0 && tasks == NULL) ||
        quantum == 0) {
        return -1;
    }

    scheduler_reset_metrics();

    if (task_count == 0) {
        return 0;
    }

    for (size_t i = 0; i < task_count; i++) {
        if (tasks[i].remaining_work <= 0.0) {
            return -1;
        }

        for (size_t j = i + 1; j < task_count; j++) {
            if (tasks[i].id == tasks[j].id) {
                return -1;
            }
        }

        tasks[i].status = TASK_READY;
        tasks[i].first_start_tick = 0;
        tasks[i].completion_tick = 0;
        tasks[i].total_run_ticks = 0;
    }

    size_t *queue = malloc(task_count * sizeof(size_t));
    if (queue == NULL) {
        return -1;
    }

    size_t head = 0;
    size_t tail = 0;
    size_t count = 0;
    size_t completed = 0;
    uint64_t tick = 0;

    /* Add tasks that are ready at tick zero. */
    for (size_t i = 0; i < task_count; i++) {
        if (tasks[i].arrival_tick == 0) {
            queue[tail] = i;
            tail = (tail + 1) % task_count;
            count++;
        }
    }

    while (completed < task_count) {
        if (count == 0) {
            uint64_t next_arrival = UINT64_MAX;

            for (size_t i = 0; i < task_count; i++) {
                if (tasks[i].status != TASK_COMPLETED &&
                    tasks[i].arrival_tick > tick &&
                    tasks[i].arrival_tick < next_arrival) {
                    next_arrival = tasks[i].arrival_tick;
                }
            }

            if (next_arrival == UINT64_MAX) {
                free(queue);
                return -1;
            }

            tick = next_arrival;

            for (size_t i = 0; i < task_count; i++) {
                if (tasks[i].status != TASK_COMPLETED &&
                    tasks[i].arrival_tick <= tick) {
                    int already_queued = 0;

                    for (size_t k = 0; k < count; k++) {
                        if (queue[(head + k) % task_count] == i) {
                            already_queued = 1;
                            break;
                        }
                    }

                    if (!already_queued) {
                        queue[tail] = i;
                        tail = (tail + 1) % task_count;
                        count++;
                    }
                }
            }
        }

        size_t index = queue[head];
        head = (head + 1) % task_count;
        count--;

        Task *task = &tasks[index];

        if (task->status == TASK_COMPLETED) {
            continue;
        }

        task->status = TASK_RUNNING;

        if (task->first_start_tick == 0 && tick != 0) {
            task->first_start_tick = tick;
        }

        double slice = task->remaining_work;
        if (slice > (double)quantum) {
            slice = (double)quantum;
        }

        uint64_t start_tick = tick;
        uint64_t end_tick = tick + (uint64_t)slice;

        /* Add newly arrived tasks while this task runs. */
        for (size_t i = 0; i < task_count; i++) {
            if (i != index &&
                tasks[i].status != TASK_COMPLETED &&
                tasks[i].arrival_tick > start_tick &&
                tasks[i].arrival_tick <= end_tick) {
                int already_queued = 0;

                for (size_t k = 0; k < count; k++) {
                    if (queue[(head + k) % task_count] == i) {
                        already_queued = 1;
                        break;
                    }
                }

                if (!already_queued) {
                    queue[tail] = i;
                    tail = (tail + 1) % task_count;
                    count++;
                }
            }
        }

        tick = end_tick;
        task->total_run_ticks += (uint64_t)slice;
        task->remaining_work -= slice;

        if (task->remaining_work < 0.000001) {
            task->remaining_work = 0.0;
            task->status = TASK_COMPLETED;
            task->completion_tick = tick;
            completed++;

            metrics.completed_tasks++;
            metrics.total_waiting_time +=
    (double)(task->completion_tick - task->arrival_tick) -
    (double)task->total_run_ticks;
            metrics.total_turnaround_time +=
                (double)(task->completion_tick -
                         task->arrival_tick);

            if (task->deadline_tick != 0 &&
                task->completion_tick > task->deadline_tick) {
                metrics.deadline_misses++;
            }
        } else {
            task->status = TASK_READY;
            queue[tail] = index;
            tail = (tail + 1) % task_count;
            count++;
        }
    }

    metrics.current_tick = tick;
    free(queue);
    return 0;
}
