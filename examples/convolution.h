#ifndef COROUTINE_CONVOLUTION_H
#define COROUTINE_CONVOLUTION_H

#include <inttypes.h>
#include "coroutine.h"

#define SRC_WIDTH           16
#define SRC_HEIGHT          16
#define DST_WIDTH           32
#define DST_HEIGHT          32

#define MAX_BARRIERS        1
#define YIELD_PAUSE         (coroutine_arg_t)0
#define YIELD_BARRIER       (coroutine_arg_t)1

typedef struct _convolution_data *convolution_data;

typedef struct
{
    convolution_data data;
    coroutine_t co;
    uint16_t id;
    uint16_t barrier;
    uint32_t padding[1];
} convolution_task;

typedef struct
{
    uint32_t task_left;
} barrier_t;

struct _convolution_data
{
    float *src_data;
    float *tmp_data;
    float *dst_data;
    uint32_t src_width;
    uint32_t src_height;
    uint32_t dst_width;
    uint32_t dst_height;
    coroutine_context_t ctx;
    convolution_task *tasks;
    uint32_t task_count;
    barrier_t barriers[MAX_BARRIERS];
};

void convolution_main(convolution_data data);
void convolution_prepare_data(convolution_data data);
void convolution_dispose_data(convolution_data data);
void do_convolution_task(convolution_data data, convolution_task *task);

int try_schedule_task(convolution_task *task);
barrier_t* task_current_barrier(convolution_task *task);
void task_unset_barrier(convolution_task *task, barrier_t *barrier);
void wait_at_barrier(convolution_task *task, uint32_t barrier_id);

#endif
