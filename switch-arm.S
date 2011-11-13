.section .text
.align 4
.arm

@ void *coroutine_switch(coroutine_t co, void *arg, coroutine_context_t ctx);
@ r0: co, r1: arg, r2: ctx, r3: current, r14: state
.globl coroutine_switch
coroutine_switch:
@ save the current context (stack, return address and registers)
    ldr r3, [r2]                @ current = ctx->current
    str r0, [r2]                @ ctx->current = co
    push {fp}                   @ save stack frame
    str lr, [r3]                @ current->ret_addr = caller
    ldr r14, [r0]               @ state = co->state
    str r3, [r0, #12]           @ co->caller = current
    str sp, [r3, #8]            @ current->stack = stack pointer
    ands r14, r14, #0x10000000  @ call coroutine_entry if co hasn't started yet
    beq coroutine_entry

@ void *coroutine_return(coroutine_t co, void *arg, coroutine_context_t ctx)
@ r0: co, r1: arg, r2: ctx
coroutine_return:
    ldr sp, [r0, #8]            @ restore the coroutine's stack
    pop {fp}
    mov r0, r1                  @ return arg to co's caller
    ldr pc, [r0, #4]

@ void coroutine_entry(coroutine_t co, void *arg, coroutine_context_t ctx) no_return
@ run the dest coroutine (for the first time)
coroutine_entry:
@ r0: co, r1: arg, r2: entry, r3: ctx, r14: state
    ldr r14, [r0]               @ co->state |= STARTED
    ldr sp, [r0, #8]            @ switch to coroutine's stack
    orr r14, r14, #0x10000000   @ co->state |= STARTED
    str r14, [r0]               @ co->state |= STARTED
    mov r2, r0                  @ co->entry(ctx->user, arg)
    ldr r0, [r3, #8]            @ ...
    ldr r2, [r2, #16]           @ ...
    stmfd sp!, {r0, r3}         @ save co and ctx
    bl r2                       @ co->entry(ctx->user, arg)

@ the coroutine exited, switch back to its last caller
@ r0: caller, r1/r3: co, r2: ctx, r14: state
    ldmfd sp!, {r1, r2}         @ restore co and ctx
    ldr r14, [r1]               @ co->state |= FINISHED
    orr r14, r14, #0x80000000   @ ...
    str r14, [r1]               @ ...
    mov r3, r1
    ldr r0, [r3, #12]           @ current = co->caller
    str r0, [r2]                @ ...
    mov r1, #0                  @ co->caller = NULL
    str r1, [r3, #12]           @ ...
    b coroutine_return          @ coroutine_return(current, 0, ctx)
