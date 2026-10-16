
#include <assert.h>
#include <stdio.h>
#include "scheduler.h"

int main(void)
{
    Task tasks[] = {
        {1, 0, 0, 5.0, 1, 20, 0, 0, TASK_READY},
        {2, 0, 0, 3.0, 1, 20, 0, 0, TASK_READY},
        {3, 0, 0, 1.0, 1, 20, 0, 0, TASK_READY}
    };

    scheduler_init();

    int result = scheduler_run_round_robin(tasks, 3, 2);
    assert(result == 0);

    const SchedulerMetrics *m = scheduler_get_metrics();

    printf("Completed tasks: %u\n",
           (unsigned int)m->completed_tasks);
    printf("Current tick: %llu\n",
           (unsigned long long)m->current_tick);
    printf("Total waiting time: %.2f\n",
           m->total_waiting_time);
    printf("Total turnaround time: %.2f\n",
           m->total_turnaround_time);
    printf("Deadline misses: %u\n",
           (unsigned int)m->deadline_misses);

    assert(m->completed_tasks == 3);
    assert(m->current_tick == 9);
    assert(m->total_waiting_time == 13.0);
    assert(m->total_turnaround_time == 22.0);
    assert(m->deadline_misses == 0);

    for (size_t i = 0; i < 3; i++) {
        assert(tasks[i].status == TASK_COMPLETED);
    }

    puts("PASS: Round Robin scheduling tests");
    return 0;
}
