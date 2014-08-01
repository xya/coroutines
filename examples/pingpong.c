// Copyright (c) 2009, 2011, Pierre-Andre Saulais <pasaulais@free.fr>
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

#include <stdio.h>
#include <inttypes.h>
#include "coroutine.h"

void ping(coroutine_context_t ctx, uintptr_t n);
void pong(coroutine_context_t ctx, void *arg);

int main(int argc, char **argv)
{
    coroutine_context_t ctx = coroutine_create_context(0);
    ping(ctx, 1);
    coroutine_free_context(ctx);
}

void ping(coroutine_context_t ctx, uintptr_t n)
{
    coroutine_t co_pong = coroutine_create(ctx, (coroutine_func_t)pong);
    do
    {
        printf("ping! %zi\n", n);
        n = (uintptr_t)coroutine_resume(co_pong, (void *)(n + 1));
    }
    while(coroutine_alive(co_pong));
    coroutine_free(co_pong);
}

void pong(coroutine_context_t ctx, void *arg)
{
    uintptr_t n = (uintptr_t)arg;
    while(n < 10)
    {
        printf("pong! %zi\n", n);
        n = (uintptr_t)coroutine_yield(ctx, (void *)(n + 1));
    }
}
