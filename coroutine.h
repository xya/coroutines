#ifndef COROUTINE_H
#define COROUTINE_H

typedef struct _coroutine *coroutine_t;
typedef struct _coroutine_context *coroutine_context_t;
typedef void *coroutine_arg_t;

typedef void (*coroutine_func_t)(coroutine_arg_t ctx, coroutine_arg_t arg);

// Create a new context to execute coroutines in.
// The first argument passed to every coroutine is 'user_ctx'.
// If 'user_ctx' is NULL, then the coroutine_context_t is passed instead.
// The second argument is coroutine_main/coroutine_resume's 'arg' argument.
coroutine_context_t coroutine_create_context(size_t stack_size, coroutine_arg_t user_ctx);

// Execute a function as the main coroutine: f(user_ctx, arg).
void coroutine_main(coroutine_context_t ctx, coroutine_func_t f, coroutine_arg_t arg);

// Create a new coroutine, but don't start executing it.
coroutine_t coroutine_create(coroutine_context_t ctx, coroutine_func_t f);

// Pause the execution of the current coroutine, passing 'arg' to the caller
// coroutine (the one which called coroutine_resume()) and returning the value
// passed to coroutine_resume(). If the caller coroutine finished executing
// while a coroutine is paused, that coroutine becomes a zombie.
void *coroutine_yield(coroutine_context_t ctx, coroutine_arg_t arg);

// Resume (or start) the execution of the coroutine, passing 'arg' to it.
// Stop the execution of the current coroutine. Return the value passed
// to coroutine_yield() or a NULL pointer if the coroutine finished executing.
void *coroutine_resume(coroutine_context_t ctx, coroutine_t co, coroutine_arg_t arg);

// Return the currently executing coroutine.
coroutine_t coroutine_current(coroutine_context_t ctx);

// Manipulate the per-context user data, passed as first argument to coroutines.
coroutine_arg_t coroutine_get_context_data(coroutine_context_t ctx);
void coroutine_set_context_data(coroutine_context_t ctx, coroutine_arg_t user_ctx);

// Manipulate the per-coroutine user data.
coroutine_arg_t coroutine_get_data(coroutine_t co);
void coroutine_set_data(coroutine_t co, coroutine_arg_t data);

// Return a non-zero value if the coroutine has not finished executing yet.
int coroutine_alive(coroutine_t co);

// Free the memory allocated for the coroutine.
void coroutine_free(coroutine_t co);

// Free the memory allocated for the context.
void coroutine_free_context(coroutine_context_t ctx);

#endif
