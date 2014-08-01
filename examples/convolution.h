// Copyright (c) 2011, Pierre-Andre Saulais <pasaulais@free.fr>
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// * Neither the name of the <organization> nor the
//   names of its contributors may be used to endorse or promote products
//   derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
