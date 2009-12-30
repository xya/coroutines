#include <stdio.h>
#include <inttypes.h>
#include "coroutine.h"

void foo(void *arg);
void bar(void *arg);
void baz(void *arg);

int main(int argc, char **argv)
{
    coroutine_main(foo, NULL);
}

void foo(void *arg)
{
    printf("in foo\n");
    coroutine_t co_bar = coroutine_spawn(bar);
    coroutine_t co_baz = coroutine_spawn(baz);
    printf("foo: starting bar\n");
    coroutine_resume(co_bar, NULL);
    printf("foo: starting baz\n");
    coroutine_resume(co_baz, co_bar);
    printf("leaving foo\n");
    coroutine_free(co_bar);
    coroutine_free(co_baz);
}

void bar(void *arg)
{
    printf("in bar\n");
    printf("bar: yield()\n");
    coroutine_yield(NULL);
    printf("bar: yield returned\n");
    printf("leaving bar\n");
}

void baz(void *arg)
{
    coroutine_t co_bar = (coroutine_t)arg;
    printf("in baz\n");
    printf("baz: resume(bar)\n");
    coroutine_resume(co_bar, NULL);
    printf("baz: resume returned\n");
    printf("leaving baz\n");
}
