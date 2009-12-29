#include <malloc.h>
#include "coroutine.h"

struct
{
    int state;              // unstarted, running, paused, finished
    coroutine_func entry;
    void *stack_ptr;        // the coroutine's stack (i.e. used with ESP register)
    void *pc;               // the coroutine's program counter
    coroutine_t parent;     // the coroutine which resumed this one
} coroutine;

static coroutine_t current = NULL;

void coroutine_main(coroutine_func f, void *arg)
{
    (void)f;
    (void)arg;
}

coroutine_t coroutine_spawn(coroutine_func f)
{
    (void)f;
    return NULL;
}

void *coroutine_yield(void *arg)
{
    (void)arg;
    return NULL;
}

void *coroutine_resume(coroutine_t co, void *arg)
{
    (void)co;
    (void)arg;
    return NULL;
}

coroutine_t coroutine_current()
{
    return current;
}

int coroutine_alive(coroutine_t co)
{
    return 0;
}
