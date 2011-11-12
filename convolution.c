#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "coroutine.h"
#include "convolution.h"

int main(int argc, char **argv)
{
    struct _convolution_data data;
    memset(&data, 0, sizeof(struct _convolution_data));
    data.src_width = SRC_WIDTH;
    data.src_height = SRC_HEIGHT;
    data.dst_width = DST_WIDTH;
    data.dst_height = DST_HEIGHT;
    data.task_count = data.src_height;
    coroutine_main((coroutine_func)convolution_main, &data);
}

void convolution_main(convolution_data data)
{
    uint32_t i, tasks_scheduled;
    convolution_prepare_data(data);
    do
    {
        tasks_scheduled = 0;
        for(i = 0; i < data->task_count; i++)
        {
            if(try_schedule_task(data, &data->tasks[i]))
                tasks_scheduled++;
        }
    }
    while(tasks_scheduled > 0);
    convolution_dispose_data(data);
}

void convolution_prepare_data(convolution_data data)
{
    convolution_task *task = NULL;
    uint32_t i;
    // allocate memory for the data to resample
    data->src_data = (float *)calloc(data->src_width * data->src_height, sizeof(float));
    data->tmp_data = (float *)calloc(data->dst_width * data->src_height, sizeof(float));
    data->dst_data = (float *)calloc(data->dst_width * data->dst_height, sizeof(float));
    // initialize the tasks
    data->tasks = (convolution_task *)calloc(data->task_count, sizeof(convolution_task));
    for(i = 0; i < data->task_count; i++)
    {
        task = data->tasks + i;
        task->data = data;
        task->co = coroutine_create((coroutine_func)do_convolution_task, 0);
        task->id = i;
    }
    for(i = 0; i < MAX_BARRIERS; i++)
        data->barriers[i].task_left = data->task_count;
}

void convolution_dispose_data(convolution_data data)
{
    uint32_t i;
    for(i = 0; i < data->task_count; i++)
        coroutine_free(data->tasks[i].co);
    free(data->src_data);
    free(data->tmp_data);
    free(data->dst_data);
}

int try_schedule_task(convolution_data data, convolution_task *task)
{
    uint32_t i;
    barrier_t *barrier = NULL;
    void *result = NULL;
    if(!coroutine_alive(task->co))
        return 0;
    // a task waiting for a barrier is schedulable iif all tasks are waiting at this barrier
    barrier = task_current_barrier(data, task);
    if(barrier && barrier->task_left)
        return 0;
    // resume the task.
    result = coroutine_resume(task->co, task);
    if(result != YIELD_BARRIER)
        return 1;
    // if the task is waiting for a barrier, decrement the count
    barrier = task_current_barrier(data, task);
    if(barrier && barrier->task_left && (0 == --(barrier->task_left)))
    {
        // all tasks have waited at this barrier, clear it
        for(i = 0; i < data->task_count; i++)
            task_unset_barrier(data, &data->tasks[i], barrier);
    }
    return 1;
}

void wait_at_barrier(uint32_t barrier_id)
{
    coroutine_set_user_state(coroutine_current(), barrier_id);
    coroutine_yield(YIELD_BARRIER);
}

barrier_t* task_current_barrier(convolution_data data, convolution_task *task)
{
    uint32_t id = 0;
    if(!coroutine_alive(task->co))
        return NULL;
    id = coroutine_get_user_state(task->co);
    if(!id || (id > MAX_BARRIERS))
        return NULL;
    return &data->barriers[id - 1];
}

void task_unset_barrier(convolution_data data, convolution_task *task, barrier_t *barrier)
{
    barrier_t *current = task_current_barrier(data, task);
    if(current && (current == barrier))
        coroutine_set_user_state(task->co, 0);
}

void do_convolution_task(convolution_task *task)
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
