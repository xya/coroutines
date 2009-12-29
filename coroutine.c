#include <malloc.h>
#include <stdio.h>
#include "coroutine.h"

struct coroutine
{
    int state;              // unstarted, running, paused, finished
    coroutine_func entry;
    void *stack_ptr;        // the coroutine's stack (i.e. used with ESP register)
    void *pc;               // the coroutine's program counter
    coroutine_t caller;     // the coroutine which resumed this one
};

#define STATE_UNSTARTED     0
#define STATE_RUNNING       1
#define STATE_PAUSED        2
#define STATE_FINISHED      3

#define COROUTINE_STACK_SIZE    4096

static coroutine_t current = NULL;

static void *coroutine_switch(coroutine_t co, void *arg);

coroutine_t coroutine_current()
{
    return current;
}

int coroutine_alive(coroutine_t co)
{
    return co && (co->state < STATE_FINISHED);
}

void coroutine_main(coroutine_func f, void *arg)
{
    // create a coroutine for the 'main' function
    coroutine_t co = (coroutine_t)malloc(sizeof(struct coroutine));
    co->state = STATE_RUNNING;
    co->entry = f;
    co->pc = NULL;
    co->caller = NULL;
    current = co;
    co->stack_ptr = NULL; // TODO get stack pointer
    
    // execute it
    co->entry(arg);
    
    // the coroutine finished
    printf("Main coroutine exited.\n");
    co->state = STATE_FINISHED;
    current = NULL;
}

coroutine_t coroutine_spawn(coroutine_func f)
{
    // create a coroutine for the function
    coroutine_t co;
    void *stack;
    
    co = (coroutine_t)malloc(sizeof(struct coroutine));
    if(!co)
        return NULL;
    stack = malloc(COROUTINE_STACK_SIZE);
    if(!stack)
    {
        free(co);
        return NULL;
    }
    co->state = STATE_UNSTARTED;
    co->entry = f;
    co->pc = NULL;
    co->caller = NULL;
    co->stack_ptr = stack;
    return co;
}

void *coroutine_yield(void *arg)
{
    coroutine_t co = coroutine_current();
    if(!co)
    {
        fprintf(stderr, "coroutine_yield(): no coroutine is executing.\n");
        return NULL;
    }
    else if(!co->caller)
    {
        fprintf(stderr, "coroutine_yield(): current coroutine doesn't have a caller.\n");
        return NULL;
    }
    else if(!coroutine_alive(co->caller))
    {
        fprintf(stderr, "coroutine_yield(): caller must be still alive.\n");
        return NULL;
    }
    return coroutine_switch(co->caller, arg);
}

void *coroutine_resume(coroutine_t co, void *arg)
{
    int state;
    if(!coroutine_current())
    {
        fprintf(stderr, "coroutine_resume(): no coroutine is executing.\n");
        return NULL;
    }
    else if(!co)
    {
        fprintf(stderr, "coroutine_resume(): callee coroutine must not be null.\n");
        return NULL;
    }
    else if(!coroutine_alive(co))
    {
        fprintf(stderr, "coroutine_resume(): callee coroutine must be still alive.\n");
        return NULL;
    }
    return coroutine_switch(co, arg);
}

void *coroutine_switch(coroutine_t co, void *arg)
{
    (void)co;
    (void)arg;
    return NULL;
    //save the current context (registers and arg)
    //switch to coroutine 'co' (i.e., call co->pc)
    //when rescheduled, restore the context and return the result
}
