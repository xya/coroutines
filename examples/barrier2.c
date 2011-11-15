#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "coroutine.h"
#include "barrier2.h"

#define N_THREADS   4
#define N_TASKS     4

static pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
    uint16_t i, j;
    uint16_t threads = N_THREADS, tasks = N_TASKS;
    pthread_barrier_t barriers[MAX_BARRIERS];
    task_list_t lists = NULL;
    task_list_t list = NULL;

    // create a list per thread
    pthread_mutex_lock(&list_mutex);
    lists = (task_list_t)calloc(threads, sizeof(struct _task_list));

    // create inter-thread barriers
    for(i = 0; i < MAX_BARRIERS; i++)
        pthread_barrier_init(&barriers[i], NULL, threads);

    // launch threads
    for(i = 0; i < threads; i++)
    {
        list = &lists[i];
        list->id = i;
        list->task_count = tasks;
        for(j = 0; j < MAX_BARRIERS; j++)
        {
            list->barriers[j].id = j + 1;
            list->barriers[j].barrier = &barriers[j];
            list->barriers[j].task_left = list->task_count;
        }
        pthread_create(&list->thread, NULL, tasks_main, list);
    }
    pthread_mutex_unlock(&list_mutex);

    // wait for the threads to finish
    for(i = 0; i < threads; i++)
        pthread_join(lists[i].thread, NULL);

    // free the barriers
    for(i = 0; i < MAX_BARRIERS; i++)
        pthread_barrier_destroy(&barriers[i]);
}

void tasks_main(task_list_t list)
{
    uint32_t i, tasks_scheduled;
    task_t *t = NULL;
    pthread_mutex_lock(&list_mutex);
    alloc_tasks(list);
    pthread_mutex_unlock(&list_mutex);
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
    list->ctx = coroutine_create_context(0);
    coroutine_set_context_data(list->ctx, list);
    list->tasks = (task_t *)calloc(list->task_count, sizeof(task_t));
    for(i = 0; i < list->task_count; i++)
    {
        task = list->tasks + i;
        task->co = coroutine_create(list->ctx, (coroutine_func_t)do_task);
        task->id = i;
        task->barrier = 0;
        coroutine_set_data(task->co, list);
    }
}

void dispose_tasks(task_list_t list)
{
    uint32_t i;
    for(i = 0; i < list->task_count; i++)
        coroutine_free(list->tasks[i].co);
    coroutine_free_context(list->ctx);
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
        // all tasks waited at this coroutine barrier

        // wait for the thread barrier
        printf("<%d> Waiting for barrier %d\n", list->id, barrier->id);
        pthread_barrier_wait(barrier->barrier);
        printf("<%d> Passed barrier %d\n", list->id, barrier->id);

        // clear the coroutine barrier
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
    printf("<%d> Computing A[%d]\n", list->id, task->id);
    wait_at_barrier(task, 1);
    printf("<%d> Computing B[%d]\n", list->id, task->id);
}
