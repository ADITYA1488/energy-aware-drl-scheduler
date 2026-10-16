
#include <assert.h>
#include <stdio.h>
#include "scheduler.h"

#define TASK_COUNT 4

static void print_metrics(
    const char *policy,
    const SchedulerMetrics *m)
{
    printf("\nPolicy: %s\n", policy);
    printf("Completed tasks: %u\n",
           (unsigned int)m->completed_tasks);
    printf("Final tick: %llu\n",
           (unsigned long long)m->current_tick);
    printf("Total waiting time: %.2f\n",
           m->total_waiting_time);
    printf("Total turnaround time: %.2f\n",
           m->total_turnaround_time);
    printf("Deadline misses: %u\n",
           (unsigned int)m->deadline_misses);
}

static void create_workload(Task tasks[TASK_COUNT])
{
    tasks[0] = (Task){1, 0, 0, 5.0, 1, 30, 0, 0, TASK_READY};
    tasks[1] = (Task){2, 0, 0, 3.0, 1, 30, 0, 0, TASK_READY};
    tasks[2] = (Task){3, 0, 0, 1.0, 1, 30, 0, 0, TASK_READY};
    tasks[3] = (Task){4, 0, 0, 4.0, 1, 30, 0, 0, TASK_READY};
}

int main(void)
{
    Task fcfs_tasks[TASK_COUNT];
    Task rr_tasks[TASK_COUNT];

    /* Both policies receive identical workloads. */
    create_workload(fcfs_tasks);
    create_workload(rr_tasks);

    scheduler_init();

    assert(scheduler_run_fcfs(fcfs_tasks, TASK_COUNT) == 0);
    SchedulerMetrics fcfs = *scheduler_get_metrics();

    scheduler_init();

    assert(scheduler_run_round_robin(
        rr_tasks, TASK_COUNT, 2) == 0);
    SchedulerMetrics rr = *scheduler_get_metrics();

    assert(fcfs.completed_tasks == TASK_COUNT);
    assert(rr.completed_tasks == TASK_COUNT);

    print_metrics("FCFS", &fcfs);
    print_metrics("Round Robin (quantum = 2)", &rr);

    puts("\nPASS: Both policies completed the same workload.");
    return 0;
}
