#include <malloc.h>
#include <stdio.h>
#include <inttypes.h>
#include "coroutine.h"

struct _coroutine_context
{
    coroutine_t current;
    size_t stack_size;
    coroutine_arg_t user_ctx;
};

struct _coroutine
{
    int state;              // XXX uint32
    void *ret_addr;         // return address of coroutine_switch's caller
    void *stack;            // the coroutine's stack (i.e. used with ESP register)
    coroutine_t caller;     // the coroutine which resumed this one
    coroutine_func_t entry;
    void *orig_stack;
};

#define COROUTINE_STARTED       0x10000000
#define COROUTINE_FINISHED      0x80000000
#define COROUTINE_USER          0x0fffffff

#define COROUTINE_STACK_SIZE    4096

extern void *coroutine_switch(coroutine_t co, coroutine_arg_t arg, coroutine_context_t ctx);

coroutine_context_t coroutine_create_context(size_t stack_size, coroutine_arg_t user_ctx)
{
    coroutine_context_t ctx = (coroutine_context_t)malloc(sizeof(struct _coroutine_context));
    if(!ctx)
        return NULL;
    if(stack_size < COROUTINE_STACK_SIZE)
        stack_size = COROUTINE_STACK_SIZE;
    ctx->current = NULL;
    ctx->stack_size = stack_size;
    ctx->user_ctx = user_ctx ? user_ctx : ctx;
    return ctx;
}

coroutine_t coroutine_current(coroutine_context_t ctx)
{
    return ctx ? ctx->current : NULL;
}

int coroutine_alive(coroutine_t co)
{
    return co && ((co->state & COROUTINE_FINISHED) == 0);
}

int coroutine_get_user_state(coroutine_t co)
{
    return co ? co->state & COROUTINE_USER : 0;
}

void coroutine_set_user_state(coroutine_t co, int state)
{
    if(co)
        co->state |= (state & COROUTINE_USER);
}

void coroutine_main(coroutine_context_t ctx, coroutine_func_t f, coroutine_arg_t arg)
{
    if(!ctx)
    {
        fprintf(stderr, "coroutine_main(): the context is null.\n");
        return;
    }
    else if(coroutine_current(ctx))
    {
        fprintf(stderr, "coroutine_main(): a coroutine is already executing.\n");
        return;
    }
    
    // create a coroutine for the 'main' function
    coroutine_t co = (coroutine_t)malloc(sizeof(struct _coroutine));
    co->state = COROUTINE_STARTED;
    co->entry = f;
    co->ret_addr = NULL;
    co->caller = NULL;
    co->stack = NULL;
    co->orig_stack = NULL;
    ctx->current = co;
    
    // execute it
    co->entry(ctx->user_ctx, arg);
    co->state |= COROUTINE_FINISHED;
    ctx->current = co->caller = NULL;
    coroutine_free(co);
}

coroutine_t coroutine_create(coroutine_context_t ctx, coroutine_func_t f)
{
    // create a coroutine for the function
    coroutine_t co;
    void *stack;
    
    if(!ctx)
    {
        fprintf(stderr, "coroutine_create(): the context is null.\n");
        return NULL;
    }
    else if(!coroutine_current(ctx))
    {
        fprintf(stderr, "coroutine_create(): no coroutine is executing.\n");
        return NULL;
    }
    
    co = (coroutine_t)malloc(sizeof(struct _coroutine));
    if(!co)
        return NULL;
    stack = malloc(ctx->stack_size);
    if(!stack)
    {
        free(co);
        return NULL;
    }
    co->state = 0;
    co->entry = f;
    co->ret_addr = NULL;
    co->caller = NULL;
    co->stack = stack + ctx->stack_size; // stack grows downwards
    co->orig_stack = stack;
    return co;
}

void *coroutine_yield(coroutine_context_t ctx, coroutine_arg_t arg)
{
    coroutine_t cur = NULL, caller = NULL;
    if(!ctx)
    {
        fprintf(stderr, "coroutine_yield(): the context is null.\n");
        return NULL;
    }
    cur = coroutine_current(ctx);
    if(!cur)
    {
        fprintf(stderr, "coroutine_yield(): no coroutine is executing.\n");
        return NULL;
    }
    caller = cur->caller;
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
    return coroutine_switch(caller, arg, ctx);
}

void *coroutine_resume(coroutine_context_t ctx, coroutine_t co, coroutine_arg_t arg)
{
    if(!ctx)
    {
        fprintf(stderr, "coroutine_resume(): the context is null.\n");
        return NULL;
    }
    else if(!coroutine_current(ctx))
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
    return coroutine_switch(co, arg, ctx);
}

void coroutine_free(coroutine_t co)
{
    if(!co)
        return;
    if((co->state & COROUTINE_STARTED) && !(co->state & COROUTINE_FINISHED))
    {
        fprintf(stderr, "coroutine_free(): coroutine 'co' must be terminated or unstarted.\n");
        return;
    }

    if(co->orig_stack)
        free(co->orig_stack);
    free(co);
}

void coroutine_free_context(coroutine_context_t ctx)
{
    if(!ctx)
        return;
    free(ctx);
}
