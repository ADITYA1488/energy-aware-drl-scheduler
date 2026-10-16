#include "task_queue.h"

void task_queue_init(TaskQueue *queue)
{
    if (queue == NULL) {
        return;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

bool task_queue_is_empty(const TaskQueue *queue)
{
    return queue == NULL || queue->count == 0;
}

bool task_queue_is_full(const TaskQueue *queue)
{
    return queue != NULL &&
           queue->count == TASK_QUEUE_CAPACITY;
}

size_t task_queue_size(const TaskQueue *queue)
{
    return queue == NULL ? 0 : queue->count;
}

bool task_queue_enqueue(TaskQueue *queue, Task *task)
{
    if (queue == NULL || task == NULL ||
        task_queue_is_full(queue)) {
        return false;
        }

    queue->items[queue->tail] = task;
    queue->tail = (queue->tail + 1) % TASK_QUEUE_CAPACITY;
    queue->count++;

    return true;
}

Task *task_queue_dequeue(TaskQueue *queue)
{
    Task *task;

    if (task_queue_is_empty(queue)) {
        return NULL;
    }

    task = queue->items[queue->head];
    queue->head = (queue->head + 1) % TASK_QUEUE_CAPACITY;
    queue->count--;

    return task;
}