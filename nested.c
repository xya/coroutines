#include <stdio.h>
#include <inttypes.h>
#include "coroutine.h"

void foo(coroutine_context_t ctx, void *arg);
void bar(coroutine_context_t ctx, void *arg);
void baz(coroutine_context_t ctx, void *arg);

int main(int argc, char **argv)
{
    coroutine_context_t ctx = coroutine_create_context(0, NULL);
    coroutine_main(ctx, (coroutine_func_t)foo, NULL);
    coroutine_free_context(ctx);
}

void foo(coroutine_context_t ctx, void *arg)
{
    printf("entering foo\n");
    coroutine_t co_bar = coroutine_create(ctx, (coroutine_func_t)bar);
    coroutine_t co_baz = coroutine_create(ctx, (coroutine_func_t)baz);
    coroutine_resume(ctx, co_bar, NULL);
    coroutine_resume(ctx, co_baz, co_bar);
    printf("leaving foo\n");
    coroutine_free(co_bar);
    coroutine_free(co_baz);
}

void bar(coroutine_context_t ctx, void *arg)
{
    printf("entering bar\n");
    coroutine_yield(ctx, NULL);
    printf("leaving bar\n");
}

void baz(coroutine_context_t ctx, void *arg)
{
    coroutine_t co_bar = (coroutine_t)arg;
    printf("entering baz\n");
    coroutine_resume(ctx, co_bar, NULL);
    printf("leaving baz\n");
}
