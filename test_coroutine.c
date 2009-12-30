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
    while(coroutine_alive(co_ping))
    {
        n = (uintptr_t)coroutine_resume(co_ping, (void *)n); 
        printf("pong! %zi\n", n);
    }
}

void ping(void *arg)
{
    uintptr_t n = (uintptr_t)arg;
    if(n <= 10)
    {
        printf("ping! %zi\n", n);
        n++;
        n = (uintptr_t)coroutine_yield((void *)n);
    }
}