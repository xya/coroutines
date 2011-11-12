#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include <inttypes.h>

#define SRC_WIDTH   16
#define SRC_HEIGHT  16
#define DST_WIDTH   32
#define DST_HEIGHT  32

typedef struct _convolution_data *convolution_data;

typedef struct
{
    convolution_data data;
    coroutine_t co;
    uint32_t id;
    uint32_t padding[1];
} convolution_task;

struct _convolution_data
{
    float *src_data;
    float *tmp_data;
    float *dst_data;
    uint32_t src_width;
    uint32_t src_height;
    uint32_t dst_width;
    uint32_t dst_height;
    convolution_task *tasks;
    uint32_t task_count;
    uint32_t padding[1];
};

void convolution_main(convolution_data data);
void convolution_prepare_data(convolution_data data);
void convolution_dispose_data(convolution_data data);
void convolution_worker(convolution_task *task);

#endif
