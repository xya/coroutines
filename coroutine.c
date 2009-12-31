#include <malloc.h>
#include <stdio.h>
#include "coroutine.h"

struct call_entry
{
    coroutine_t callee;
    call_entry_t next;
};

struct coroutine
{
    int state;              // unstarted, running, paused, finished
    void *pc;               // the coroutine's program counter
    void *stack;            // the coroutine's stack (i.e. used with ESP register)
    coroutine_t caller;     // the coroutine which resumed this one
    coroutine_func entry;
    void *orig_stack;
    struct call_entry refs; // linked list of coroutines whose caller is this coroutine
};

#define STATE_UNSTARTED     0
#define STATE_STARTED       1
#define STATE_FINISHED      2

#define COROUTINE_STACK_SIZE    4096

coroutine_t current = NULL;

extern void *coroutine_switch(coroutine_t co, void *arg);
void coroutine_add_callee(coroutine_t co, coroutine_t callee, call_entry_t tail);
void coroutine_remove_callee(coroutine_t co, coroutine_t callee);
int coroutine_find_callee(coroutine_t co, coroutine_t callee, call_entry_t *tail);
void coroutine_set_caller(coroutine_t co, coroutine_t caller);

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
    if(coroutine_current())
    {
        fprintf(stderr, "coroutine_main(): a coroutine is already executing.\n");
        return;
    }
    
    // create a coroutine for the 'main' function
    coroutine_t co = (coroutine_t)malloc(sizeof(struct coroutine));
    co->state = STATE_STARTED;
    co->entry = f;
    co->pc = NULL;
    co->caller = NULL;
    co->stack = NULL;
    co->orig_stack = NULL;
    co->refs.callee = NULL;
    co->refs.next = NULL;
    current = co;
    
    // execute it
    co->entry(arg);
    
    // the coroutine finished
    co->state = STATE_FINISHED;
    co->caller = NULL;
    coroutine_free(co);
    current = NULL;
}

coroutine_t coroutine_create(coroutine_func f, size_t stacksize)
{
    // create a coroutine for the function
    coroutine_t co;
    void *stack;
    
    if(!coroutine_current())
    {
        fprintf(stderr, "coroutine_create(): no coroutine is executing.\n");
        return NULL;
    }
    
    co = (coroutine_t)malloc(sizeof(struct coroutine));
    if(!co)
        return NULL;
    if(stacksize < COROUTINE_STACK_SIZE)
        stacksize = COROUTINE_STACK_SIZE;
    stack = malloc(stacksize);
    if(!stack)
    {
        free(co);
        return NULL;
    }
    co->state = STATE_UNSTARTED;
    co->entry = f;
    co->pc = NULL;
    co->caller = NULL;
    co->stack = stack + stacksize; // stack grows downwards
    co->orig_stack = stack;
    co->refs.callee = NULL;
    co->refs.next = NULL;
    return co;
}

void *coroutine_yield(void *arg)
{
    coroutine_t cur = coroutine_current();
    if(!cur)
    {
        fprintf(stderr, "coroutine_yield(): no coroutine is executing.\n");
        return NULL;
    }
    coroutine_t caller = cur->caller;
    if(!caller)
    {
        fprintf(stderr, "coroutine_yield(): current coroutine doesn't have a caller.\n");
        return NULL;
    }
    else if(!coroutine_alive(caller))
    {
        fprintf(stderr, "coroutine_yield(): caller must be still alive.\n");
        return NULL;
    }
    return coroutine_switch(caller, arg);
}

void *coroutine_resume(coroutine_t co, void *arg)
{
    if(!coroutine_current())
    {
        fprintf(stderr, "coroutine_resume(): no coroutine is executing.\n");
        return NULL;
    }
    else if(!co)
    {
        fprintf(stderr, "coroutine_resume(): coroutine 'co' must not be null.\n");
        return NULL;
    }
    else if(!coroutine_alive(co))
    {
        fprintf(stderr, "coroutine_resume(): coroutine 'co' must be still alive.\n");
        return NULL;
    }
    return coroutine_switch(co, arg);
}

void coroutine_add_callee(coroutine_t co, coroutine_t callee, call_entry_t tail)
{
    // is the callee list empty?
    if(co->refs.callee == NULL)
    {
        co->refs.callee = callee;
        return;
    }
    
    // find the tail of the callee list
    if(!tail)
    {
        tail = &co->refs;
        while(tail->next)
            tail = tail->next;
    }
    
    // add the callee at the end of the list
    tail->next = (call_entry_t)malloc(sizeof(struct call_entry));
    tail->next->callee = callee;
    tail->next->next = NULL;
}

void coroutine_remove_callee(coroutine_t co, coroutine_t callee)
{
    call_entry_t ref = NULL, next = NULL;
    if(coroutine_find_callee(co, callee, &ref))
    {
        next = ref->next;
        if(next)
        {
            ref->callee = next->callee;
            ref->next = next->next;
            free(next);
        }
        else
        {
            ref->callee = NULL;
        }
    }
}

// find a callee in a coroutine's call list
int coroutine_find_callee(coroutine_t co, coroutine_t callee, call_entry_t *tail)
{
    call_entry_t last = &co->refs;
    if(last->callee == NULL)
    {
        if(tail)
            *tail = NULL;
        return 0;
    }
    else if(last->callee = callee)
    {
        if(tail)
            *tail = last;
        return 1;
    }
    else
    {
        while(last->next)
        {
            if(last->callee == callee)
            {
                if(tail)
                    *tail = last;
                return 1;
            }
            last = last->next;
        }
        if(tail)
            *tail = last;
        return 0;
    }
}

void coroutine_set_caller(coroutine_t co, coroutine_t caller)
{
    call_entry_t tail = NULL;
    
    printf("coroutine %p holds a reference to coroutine %p\n", co, caller);
    
    co->caller = caller;
    // keep track of which coroutines have another coroutine as caller
    if(!coroutine_find_callee(caller, co, &tail))
        coroutine_add_callee(caller, co, tail);
}

void coroutine_free(coroutine_t co)
{
    if(!co)
    {
        fprintf(stderr, "coroutine_free(): coroutine 'co' must not be null.\n");
        return;
    }
    else if(co->state == STATE_STARTED)
    {
        fprintf(stderr, "coroutine_free(): coroutine 'co' must be terminated or unstarted.\n");
        return;
    }
    printf("freeing coroutine %p\n", co);
    
    // we need to remove references to this coroutine for every callee
    call_entry_t tail = &co->refs;
    if(tail->callee != NULL)
    {
        while(tail)
        {
            printf("coroutine %p held a reference to coroutine %p\n", tail->callee, co);
            if(tail->callee->caller == co)
                tail->callee->caller = NULL;
            tail = tail->next;
        }
    }
    
    // free the callee list
    call_entry_t current = co->refs.next;
    while(current)
    {
        tail = current->next;
        free(current);
        current = tail;
    }
    
    // maybe this coroutine has a reference to a callee, too
    if(co->caller)
    {
        printf("Removing coroutine %p from coroutine %p's list \n", co, co->caller);
        coroutine_remove_callee(co->caller, co);
        co->caller = NULL;
    }

    if(co->orig_stack)
        free(co->orig_stack);
    free(co);
}
