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
    coroutine_main((coroutine_func)tasks_main, &list);
}

void tasks_main(task_list_t list)
{
    uint32_t i, tasks_scheduled;
    alloc_tasks(list);
    do
    {
        tasks_scheduled = 0;
        for(i = 0; i < list->task_count; i++)
        {
            if(try_schedule_task(list, &list->tasks[i]))
                tasks_scheduled++;
        }
    }
    while(tasks_scheduled > 0);
    dispose_tasks(list);
}

void alloc_tasks(task_list_t list)
{
    task_t *task = NULL;
    uint32_t i;
    // initialize the tasks
    list->tasks = (task_t *)calloc(list->task_count, sizeof(task_t));
    for(i = 0; i < list->task_count; i++)
    {
        task = list->tasks + i;
        task->co = coroutine_create((coroutine_func)do_task, 0);
        task->id = i;
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

int try_schedule_task(task_list_t list, task_t *task)
{
    uint32_t i;
    barrier_t *barrier = NULL;
    void *result = NULL;
    if(!coroutine_alive(task->co))
        return 0;
    // a task waiting for a barrier is schedulable iif all tasks are waiting at this barrier
    barrier = task_current_barrier(list, task);
    if(barrier && barrier->task_left)
        return 0;
    // resume the task.
    result = coroutine_resume(task->co, task);
    if(result != YIELD_BARRIER)
        return 1;
    // if the task is waiting for a barrier, decrement the count
    barrier = task_current_barrier(list, task);
    if(barrier && barrier->task_left && (0 == --(barrier->task_left)))
    {
        // all tasks have waited at this barrier, clear it
        for(i = 0; i < list->task_count; i++)
            task_unset_barrier(list, &list->tasks[i], barrier);
    }
    return 1;
}

void wait_at_barrier(uint32_t barrier_id)
{
    coroutine_set_user_state(coroutine_current(), barrier_id);
    coroutine_yield(YIELD_BARRIER);
}

barrier_t* task_current_barrier(task_list_t list, task_t *task)
{
    uint32_t id = 0;
    if(!coroutine_alive(task->co))
        return NULL;
    id = coroutine_get_user_state(task->co);
    if(!id || (id > MAX_BARRIERS))
        return NULL;
    return &list->barriers[id - 1];
}

void task_unset_barrier(task_list_t list, task_t *task, barrier_t *barrier)
{
    barrier_t *current = task_current_barrier(list, task);
    if(current && (current == barrier))
        coroutine_set_user_state(task->co, 0);
}

void do_task(task_t *task)
{
    printf("%02d: part A\n", task->id);
    if(task->id % 2)
    {
        coroutine_yield(YIELD_PAUSE);
        printf("%02d: part B\n", task->id);
        wait_at_barrier(1);
    }
    else
    {
        wait_at_barrier(1);
        printf("%02d: part B\n", task->id);
    }
    printf("%02d: part C\n", task->id);
}
