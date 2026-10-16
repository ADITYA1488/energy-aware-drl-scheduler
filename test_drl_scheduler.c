
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "drl_scheduler.h"

int infer_drl_action(float queue, float temp, float core);

int main(void)
{
    Task tasks[] = {
        {1, 0, 0, 5.0, 1, 20, 0, 0, TASK_READY},
        {2, 0, 0, 3.0, 1, 20, 0, 0, TASK_READY},
        {3, 2, 0, 2.0, 1, 20, 0, 0, TASK_READY}
    };

    DrlSchedulerMetrics result;

    int status = scheduler_run_drl(
        tasks,
        sizeof(tasks) / sizeof(tasks[0]),
        1.0,   /* One second per simulated tick */
        15.0,  /* Assumed maximum power in watts */
        1.0,   /* No power cap */
        &result
    );

    assert(status == 0);
    assert(result.completed_tasks == 3);
    assert(result.elapsed_ticks > 0);
    assert(result.simulated_energy_joules > 0.0);
    assert(isfinite(result.simulated_energy_joules));
    assert(result.deadline_misses == 0);

    for (size_t i = 0; i < sizeof(tasks) / sizeof(tasks[0]); i++) {
        assert(tasks[i].status == TASK_COMPLETED);
        assert(tasks[i].remaining_work == 0.0);
    }

    puts("PASS: DRL scheduler completes all tasks");
    printf("Completed tasks: %zu\n", result.completed_tasks);
    printf("Elapsed ticks: %llu\n",
           (unsigned long long)result.elapsed_ticks);
    printf("Simulated energy: %.3f J\n",
           result.simulated_energy_joules);
    printf("Average simulated power: %.3f W\n",
           result.simulated_power_watts);
    printf("Deadline misses: %zu\n", result.deadline_misses);

    return 0;
}
