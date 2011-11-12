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
    uint32_t i, tasks_activated;
    convolution_task *task = NULL;
    convolution_prepare_data(data);
    do
    {
        tasks_activated = 0;
        for(i = 0; i < data->task_count; i++)
        {
            task = data->tasks + i;
            if(coroutine_alive(task->co))
            {
                coroutine_resume(task->co, task);
                tasks_activated++;
            }
        }
    }
    while(tasks_activated > 0);
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
        task->co = coroutine_create((coroutine_func)convolution_worker, 0);
        task->id = i;
    }
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

void convolution_worker(convolution_task *task)
{
    printf("Started task %02d\n", task->id);
    printf("Finished task %02d\n", task->id);
}
