// Copyright (c) 2009, 2011, 2014, Pierre-Andre Saulais <pasaulais@free.fr>
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

#ifndef COROUTINE_H
#define COROUTINE_H

#include <stddef.h>

typedef struct _coroutine *coroutine_t;
typedef struct _coroutine_context *coroutine_context_t;
typedef void *coroutine_arg_t;

typedef void (*coroutine_func_t)(coroutine_arg_t ctx, coroutine_arg_t arg);

// Create a new context to execute coroutines in.
coroutine_context_t coroutine_create_context(size_t stack_size);

// Create a new coroutine, but don't start executing it.
coroutine_t coroutine_create(coroutine_context_t ctx, coroutine_func_t f);

// Pause the execution of the current coroutine, passing 'arg' to the caller
// coroutine (the one which called coroutine_resume()) and returning the value
// passed to coroutine_resume(). If the caller coroutine finished executing
// while a coroutine is paused, that coroutine becomes a zombie.
void *coroutine_yield(coroutine_context_t ctx, coroutine_arg_t arg);

// Resume (or start) the execution of the coroutine.
// Stop the execution of the current coroutine. Return the value passed
// to coroutine_yield() or a NULL pointer if the coroutine finished executing.
// The first argument passed to every coroutine is 'user_ctx', the second 'arg'
// If 'user_ctx' is NULL, then the coroutine_context_t is passed instead.
void *coroutine_resume(coroutine_t co, coroutine_arg_t arg);

// Return the currently executing coroutine.
coroutine_t coroutine_current(coroutine_context_t ctx);

// Manipulate the per-context user data, passed as first argument to coroutines.
coroutine_arg_t coroutine_get_context_data(coroutine_context_t ctx);
void coroutine_set_context_data(coroutine_context_t ctx, coroutine_arg_t user_ctx);

// Manipulate the per-coroutine user data.
coroutine_arg_t coroutine_get_data(coroutine_t co);
void coroutine_set_data(coroutine_t co, coroutine_arg_t data);

// Return the coroutine's context
coroutine_context_t coroutine_get_context(coroutine_t co);

// Return a non-zero value if the coroutine has not finished executing yet.
int coroutine_alive(coroutine_t co);

// Free the memory allocated for the coroutine.
void coroutine_free(coroutine_t co);

// Free the memory allocated for the context.
void coroutine_free_context(coroutine_context_t ctx);

#endif
