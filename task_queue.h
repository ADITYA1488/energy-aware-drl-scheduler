#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include "task.h"

#define TASK_QUEUE_CAPACITY 1024

typedef struct {
    Task *items[TASK_QUEUE_CAPACITY];
    size_t head;
    size_t tail;
    size_t count;
} TaskQueue;

void task_queue_init(TaskQueue *queue);

bool task_queue_enqueue(TaskQueue *queue, Task *task);
Task *task_queue_dequeue(TaskQueue *queue);

bool task_queue_is_empty(const TaskQueue *queue);
bool task_queue_is_full(const TaskQueue *queue);
size_t task_queue_size(const TaskQueue *queue);

#endif