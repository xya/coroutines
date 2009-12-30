#include <stdio.h>
#include <inttypes.h>
#include "coroutine.h"

void ping(void *arg);
void pong(void *arg);

int main(int argc, char **argv)
{
    coroutine_main(pong, NULL);
}

void pong(void *arg)
{
    uintptr_t n = 1;
    coroutine_t co_ping = coroutine_spawn(ping);
    do
    {
        printf("ping! %zi\n", n);
        n = (uintptr_t)coroutine_resume(co_ping, (void *)(n + 1)); 
    }
    while(coroutine_alive(co_ping));
}

void ping(void *arg)
{
    uintptr_t n = (uintptr_t)arg;
    while(n < 10)
    {
        printf("pong! %zi\n", n);
        n = (uintptr_t)coroutine_yield((void *)(n + 1));
    }
}