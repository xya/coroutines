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

#ifndef COROUTINE_BARRIER2_H
#define COROUTINE_BARRIER2_H

#include <inttypes.h>
#include <pthread.h>
#include "coroutine.h"

#define MAX_BARRIERS        1
#define YIELD_PAUSE         (coroutine_arg_t)0
#define YIELD_BARRIER       (coroutine_arg_t)1

typedef struct _task_list *task_list_t;

typedef struct
{
    coroutine_t co;
    uint16_t id;
    uint16_t barrier;
} task_t;

typedef struct
{
    uint16_t id;
    uint16_t task_left;
    pthread_barrier_t *barrier;
} barrier_t;

struct _task_list
{
    uint32_t id;
    coroutine_context_t ctx;
    task_t *tasks;
    uint32_t task_count;
    pthread_t thread;
    barrier_t barriers[MAX_BARRIERS];
};

void tasks_main(task_list_t list);
void alloc_tasks(task_list_t list);
void dispose_tasks(task_list_t list);
void do_task(task_list_t list, task_t *task);

int try_schedule_task(task_t *task);
barrier_t* task_current_barrier(task_t *task);
void task_unset_barrier(task_t *task, barrier_t *barrier);
void wait_at_barrier(task_t *task, uint32_t barrier_id);

#endif
