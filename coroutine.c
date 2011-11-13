#include <malloc.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include "coroutine.h"

struct _coroutine_context
{
    coroutine_t current;
    size_t stack_size;
    coroutine_arg_t user_ctx;
};

struct _coroutine
{
    intptr_t ctx_state;         // co's context + state (3 lower bits)
    void *ret_addr;             // return address of coroutine_switch's caller
    void *stack;                // the coroutine's stack
    coroutine_t caller;         // the coroutine which resumed this one
    coroutine_func_t entry;     // XXX union with previous
    void *orig_stack;
    coroutine_arg_t user_data;
};

#define COROUTINE_STARTED       (intptr_t)0x1
#define COROUTINE_FINISHED      (intptr_t)0x2
#define COROUTINE_RESERVED      (intptr_t)0x4

#define COROUTINE_STATE_MASK    (intptr_t)(COROUTINE_STARTED  | \
                                           COROUTINE_FINISHED | \
                                           COROUTINE_RESERVED)

#define COROUTINE_STACK_SIZE    4096

extern void *coroutine_switch(coroutine_t co, coroutine_arg_t arg, coroutine_context_t ctx);

coroutine_context_t coroutine_create_context(size_t stack_size)
{
    coroutine_context_t ctx = (coroutine_context_t)memalign(8, sizeof(struct _coroutine_context));
    if(!ctx)
        return NULL;
    if((intptr_t)ctx & COROUTINE_STATE_MASK)
    {
        assert(0 && "Allocated memory was not properly 8-byte aligned.");
        free(ctx);
        return NULL;
    }
    if(stack_size < COROUTINE_STACK_SIZE)
        stack_size = COROUTINE_STACK_SIZE;
    ctx->current = NULL;
    ctx->stack_size = stack_size;
    coroutine_set_context_data(ctx, NULL);
    return ctx;
}

coroutine_t coroutine_current(coroutine_context_t ctx)
{
    return ctx ? ctx->current : NULL;
}

coroutine_context_t coroutine_get_context(coroutine_t co)
{
    return co ? (coroutine_context_t)(co->ctx_state & ~COROUTINE_STATE_MASK) : NULL;
}

int coroutine_alive(coroutine_t co)
{
    return co && !(co->ctx_state & COROUTINE_FINISHED);
}

coroutine_arg_t coroutine_get_context_data(coroutine_context_t ctx)
{
    return ctx ? ctx->user_ctx : NULL;
}

void coroutine_set_context_data(coroutine_context_t ctx, coroutine_arg_t user_ctx)
{
    if(ctx)
        ctx->user_ctx = user_ctx ? user_ctx : ctx;
}

coroutine_arg_t coroutine_get_data(coroutine_t co)
{
    return co ? co->user_data : NULL;
}

void coroutine_set_data(coroutine_t co, coroutine_arg_t data)
{
    if(co)
        co->user_data = data;
}

void coroutine_main(coroutine_context_t ctx, coroutine_func_t f, coroutine_arg_t arg)
{
    coroutine_t co = NULL;
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
    co = (coroutine_t)memalign(8, sizeof(struct _coroutine));
    co->ctx_state = (intptr_t)ctx | COROUTINE_STARTED;
    co->entry = f;
    co->ret_addr = NULL;
    co->caller = NULL;
    co->stack = NULL;
    co->orig_stack = NULL;
    ctx->current = co;
    
    // execute it
    co->entry(ctx->user_ctx, arg);
    co->ctx_state |= COROUTINE_FINISHED;
    ctx->current = co->caller = NULL;
    coroutine_free(co);
}

coroutine_t coroutine_create(coroutine_context_t ctx, coroutine_func_t f)
{
    // create a coroutine for the function
    coroutine_t co = NULL;
    void *stack = NULL;
    
    if(!coroutine_current(ctx))
    {
        fprintf(stderr, "coroutine_create(): no coroutine is executing in the context.\n");
        return NULL;
    }
    
    co = (coroutine_t)memalign(8, sizeof(struct _coroutine));
    if(!co)
        return NULL;
    stack = malloc(ctx->stack_size);
    if(!stack)
    {
        free(co);
        return NULL;
    }
    co->ctx_state = (intptr_t)ctx;
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
    cur = coroutine_current(ctx);
    if(!cur)
    {
        fprintf(stderr, "coroutine_yield(): no coroutine is executing in the context.\n");
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

void *coroutine_resume(coroutine_t co, coroutine_arg_t arg)
{
    coroutine_context_t ctx  = coroutine_get_context(co);
    if(!coroutine_current(ctx))
    {
        fprintf(stderr, "coroutine_resume(): no coroutine is executing in the context.\n");
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
    if((co->ctx_state & COROUTINE_STARTED) && !(co->ctx_state & COROUTINE_FINISHED))
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
    if(coroutine_current(ctx))
    {
        fprintf(stderr, "coroutine_free_current(): a coroutine is still executing in the context.\n");
        return;
    }
    free(ctx);
}
