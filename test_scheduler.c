#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scheduler.h"

int main(void)
{
    const SchedulerMetrics *metrics;

    scheduler_init();

    metrics = scheduler_get_metrics();

    assert(metrics != NULL);
    assert(metrics->current_tick == 0);
    assert(metrics->completed_tasks == 0);
    assert(metrics->deadline_misses == 0);
    assert(metrics->total_waiting_time == 0.0);
    assert(metrics->total_turnaround_time == 0.0);

    assert(strcmp(scheduler_policy_name(POLICY_FCFS), "FCFS") == 0);
    assert(strcmp(scheduler_policy_name(POLICY_ROUND_ROBIN),
                  "Round Robin") == 0);

    scheduler_reset_metrics();

    puts("PASS: scheduler metadata tests");
    return 0;
}