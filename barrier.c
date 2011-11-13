#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "coroutine.h"
#include "barrier.h"

int main(int argc, char **argv)
{
    struct _task_list list;
    memset(&list, 0, sizeof(struct _task_list));
    list.task_count = 8;
    list.ctx = coroutine_create_context(0);
    coroutine_set_context_data(list.ctx, &list);
    coroutine_main(list.ctx, (coroutine_func_t)tasks_main, NULL);
    coroutine_free_context(list.ctx);
}

void tasks_main(task_list_t list, coroutine_arg_t arg)
{
    uint32_t i, tasks_scheduled;
    task_t *t = NULL;
    alloc_tasks(list);
    do
    {
        tasks_scheduled = 0;
        t = list->tasks;
        for(i = 0; i < list->task_count; i++, t++)
        {
            if(try_schedule_task(t))
                tasks_scheduled++;
        }
    }
    while(tasks_scheduled > 0);
    dispose_tasks(list);
}

void alloc_tasks(task_list_t list)
{
    task_t *task = NULL;
    uint16_t i;
    // initialize the tasks
    list->tasks = (task_t *)calloc(list->task_count, sizeof(task_t));
    for(i = 0; i < list->task_count; i++)
    {
        task = list->tasks + i;
        task->co = coroutine_create(list->ctx, (coroutine_func_t)do_task);
        task->id = i;
        task->barrier = 0;
        coroutine_set_data(task->co, list);
    }
    for(i = 0; i < MAX_BARRIERS; i++)
        list->barriers[i].task_left = list->task_count;
}

void dispose_tasks(task_list_t list)
{
    uint32_t i;
    for(i = 0; i < list->task_count; i++)
        coroutine_free(list->tasks[i].co);
}

int try_schedule_task(task_t *task)
{
    uint32_t i;
    barrier_t *barrier = NULL;
    task_list_t list = NULL;
    void *result = NULL;
    if(!coroutine_alive(task->co))
        return 0;
    list = (task_list_t)coroutine_get_data(task->co);
    if(!list)
        return 0;
    // a task waiting for a barrier is schedulable iif all tasks are waiting at this barrier
    barrier = task_current_barrier(task);
    if(barrier && barrier->task_left)
        return 0;
    // resume the task.
    result = coroutine_resume(task->co, task);
    if(result != YIELD_BARRIER)
        return 1;
    // if the task is waiting for a barrier, decrement the count
    barrier = task_current_barrier(task);
    if(barrier && barrier->task_left && (0 == --(barrier->task_left)))
    {
        // all tasks have waited at this barrier, clear it
        for(i = 0; i < list->task_count; i++)
            task_unset_barrier(&list->tasks[i], barrier);
    }
    return 1;
}

void wait_at_barrier(task_t *task, uint32_t barrier_id)
{
    task->barrier = barrier_id;
    coroutine_yield(coroutine_get_context(task->co), YIELD_BARRIER);
}

barrier_t* task_current_barrier(task_t *task)
{
    uint32_t id = 0;
    task_list_t list = NULL;
    if(!coroutine_alive(task->co))
        return NULL;
    id = task->barrier;
    if(!id || (id > MAX_BARRIERS))
        return NULL;
    list = (task_list_t)coroutine_get_data(task->co);
    if(!list)
        return NULL;
    return &list->barriers[id - 1];
}

void task_unset_barrier(task_t *task, barrier_t *barrier)
{
    barrier_t *current = task_current_barrier(task);
    if(current && (current == barrier))
        task->barrier = 0;
}

void do_task(task_list_t list, task_t *task)
{
    printf("%02d: part A\n", task->id);
    if(task->id % 2)
    {
        coroutine_yield(list->ctx, YIELD_PAUSE);
        printf("%02d: part B\n", task->id);
        wait_at_barrier(task, 1);
    }
    else
    {
        wait_at_barrier(task, 1);
        printf("%02d: part B\n", task->id);
    }
    printf("%02d: part C\n", task->id);
}
