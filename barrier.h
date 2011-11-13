#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include <inttypes.h>
#include "coroutine.h"

#define MAX_BARRIERS        1
#define YIELD_PAUSE         (coroutine_arg_t)0
#define YIELD_BARRIER       (coroutine_arg_t)1

typedef struct _task_list *task_list_t;

typedef struct
{
    coroutine_t co;
    uint32_t id;
} task_t;

typedef struct
{
    uint32_t task_left;
} barrier_t;

struct _task_list
{
    coroutine_context_t ctx;
    task_t *tasks;
    uint32_t task_count;
    barrier_t barriers[MAX_BARRIERS];
};

void tasks_main(task_list_t list, coroutine_arg_t arg);
void alloc_tasks(task_list_t list);
void dispose_tasks(task_list_t list);
void do_task(task_list_t list, task_t *task);

int try_schedule_task(task_list_t list, task_t *task);
barrier_t* task_current_barrier(task_list_t list, task_t *task);
void task_unset_barrier(task_list_t list, task_t *task, barrier_t *barrier);
void wait_at_barrier(task_list_t list, uint32_t barrier_id);

#endif
