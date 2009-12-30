#include <stdio.h>
#include <inttypes.h>
#include "coroutine.h"

void ping(void *arg);
void pong(void *arg);

int main(int argc, char **argv)
{
    coroutine_main(ping, NULL);
}

void ping(void *arg)
{
    uintptr_t n = 1;
    coroutine_t co_pong = coroutine_spawn(pong);
    do
    {
        printf("ping! %zi\n", n);
        n = (uintptr_t)coroutine_resume(co_pong, (void *)(n + 1)); 
    }
    while(coroutine_alive(co_pong));
    coroutine_free(co_pong);
}

void pong(void *arg)
{
    uintptr_t n = (uintptr_t)arg;
    while(n < 10)
    {
        printf("pong! %zi\n", n);
        n = (uintptr_t)coroutine_yield((void *)(n + 1));
    }
}