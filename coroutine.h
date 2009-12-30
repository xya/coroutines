#ifndef COROUTINE_H
#define COROUTINE_H

typedef struct coroutine *coroutine_t;

typedef void (*coroutine_func)(void *arg);

extern coroutine_t current;

// Execute a function as the main coroutine
void coroutine_main(coroutine_func f, void *arg);

// Create a new coroutine, but don't start executing it
coroutine_t coroutine_spawn(coroutine_func f);

// Pause the execution of the current coroutine, passing 'arg' to the caller
// coroutine (the one which called coroutine_resume()) and returning the value
// passed to coroutine_resume(). If the caller coroutine finished executing
// while a coroutine is paused, that coroutine becomes a zombie.
void *coroutine_yield(void *arg);

// Resume (or start) the execution of the coroutine, passing 'arg' to it.
// Stop the execution of the current coroutine. Return the value passed
// to coroutine_yield() or a NULL pointer if the coroutine finished executing.
void *coroutine_resume(coroutine_t co, void *arg);

// Return the currently executing coroutine
coroutine_t coroutine_current();

// Return a non-zero value if the coroutine has not finished executing yet.
int coroutine_alive(coroutine_t co);

#endif
