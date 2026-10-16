
#include <stdio.h>
#include <string.h>

#include "scheduler.h"
#include "drl_scheduler.h"

#define TASK_COUNT 4

static void create_workload(Task tasks[TASK_COUNT])
{
    tasks[0] = (Task){1, 0, 0, 5.0, 1, 30, 0, 0, TASK_READY};
    tasks[1] = (Task){2, 0, 0, 3.0, 1, 30, 0, 0, TASK_READY};
    tasks[2] = (Task){3, 2, 0, 1.0, 1, 30, 0, 0, TASK_READY};
    tasks[3] = (Task){4, 3, 0, 4.0, 1, 30, 0, 0, TASK_READY};
}

static void print_classic_metrics(
    const char *name,
    const SchedulerMetrics *m)
{
    printf("\n=== %s ===\n", name);
    printf("Completed tasks:       %zu\n", m->completed_tasks);
    printf("Elapsed ticks:         %llu\n",
           (unsigned long long)m->current_tick);
    printf("Total waiting time:    %.2f ticks\n",
           m->total_waiting_time);
    printf("Total turnaround time: %.2f ticks\n",
           m->total_turnaround_time);
    printf("Deadline misses:       %zu\n", m->deadline_misses);
}

static void print_drl_metrics(const DrlSchedulerMetrics *m)
{
    printf("\n=== DRL ===\n");
    printf("Completed tasks:       %zu\n", m->completed_tasks);
    printf("Elapsed ticks:         %llu\n",
           (unsigned long long)m->elapsed_ticks);
    printf("Total waiting time:    %.2f ticks\n",
           m->total_waiting_time);
    printf("Total turnaround time: %.2f ticks\n",
           m->total_turnaround_time);
    printf("Deadline misses:       %zu\n", m->deadline_misses);
    printf("Simulated energy:      %.4f J\n",
           m->simulated_energy_joules);
    printf("Average simulated power: %.4f W\n",
           m->simulated_power_watts);
}

int main(void)
{
    Task fcfs_tasks[TASK_COUNT];
    Task rr_tasks[TASK_COUNT];
    Task drl_tasks[TASK_COUNT];

    /* Every policy receives a fresh copy of the same workload. */
    create_workload(fcfs_tasks);
    create_workload(rr_tasks);
    create_workload(drl_tasks);

    scheduler_init();
    if (scheduler_run_fcfs(fcfs_tasks, TASK_COUNT) != 0) {
        fprintf(stderr, "FCFS failed.\n");
        return 1;
    }
    SchedulerMetrics fcfs = *scheduler_get_metrics();

    scheduler_init();
    if (scheduler_run_round_robin(rr_tasks, TASK_COUNT, 2) != 0) {
        fprintf(stderr, "Round Robin failed.\n");
        return 1;
    }
    SchedulerMetrics rr = *scheduler_get_metrics();

    DrlSchedulerMetrics drl;
    if (scheduler_run_drl(
            drl_tasks,
            TASK_COUNT,
            1.0,   /* seconds per simulated tick */
            15.0,  /* assumed maximum power */
            1.0,   /* no power cap */
            &drl) != 0) {
        fprintf(stderr, "DRL scheduler failed.\n");
        return 1;
    }

    print_classic_metrics("FCFS", &fcfs);
    print_classic_metrics("Round Robin (quantum = 2)", &rr);
    print_drl_metrics(&drl);

    if (fcfs.completed_tasks != TASK_COUNT ||
        rr.completed_tasks != TASK_COUNT ||
        drl.completed_tasks != TASK_COUNT) {
        fprintf(stderr, "\nFAIL: at least one policy did not finish all tasks.\n");
        return 1;
    }

    puts("\nPASS: all policies completed the same input workload.");
    puts("Note: execution-rate and energy models differ;");
    puts("do not interpret this as a validated performance ranking.");

    return 0;
}
