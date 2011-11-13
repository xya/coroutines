#include <stdio.h>
#include <inttypes.h>
#include "coroutine.h"

void ping(coroutine_context_t ctx, void *arg);
void pong(coroutine_context_t ctx, void *arg);

int main(int argc, char **argv)
{
    coroutine_context_t ctx = coroutine_create_context(0, NULL);
    coroutine_main(ctx, (coroutine_func_t)ping, NULL);
    coroutine_free_context(ctx);
}

void ping(coroutine_context_t ctx, void *arg)
{
    uintptr_t n = 1;
    coroutine_t co_pong = coroutine_create(ctx, (coroutine_func_t)pong);
    do
    {
        printf("ping! %zi\n", n);
        n = (uintptr_t)coroutine_resume(ctx, co_pong, (void *)(n + 1));
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
