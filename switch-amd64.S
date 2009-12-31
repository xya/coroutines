.section .text

// references in coroutine.c
.globl current
.globl coroutine_terminated

// void *coroutine_switch(coroutine_t co, void *arg);
coroutine_switch:   .globl coroutine_switch
// save the current context (stack, return address and registers)
    cmpl $2, (%rdi)         # is the coroutine finished?
    je coroutine_error
    movq %rsi, %rdx
    movq current, %rsi
    popq 8(%rsi)            # current->pc = return address of the caller
    pushq %rbp
    movq %rsp, 16(%rsi)     # current->stack = current stack
    movq %rsi, 24(%rdi)     # co->caller = current
    movq %rdi, current      # current = co
    cmpl $0, (%rdi)         # has the coroutine not been started before?
    je coroutine_enter

// when rescheduled, restore the context and return the result 
coroutine_do_resume:
    movq 16(%rdi), %rsp     # restore the coroutine`s stack
    popq %rbp               # restore rbp
    movq %rdx, %rax         # <return value> = arg
    jmp *8(%rdi)            # co->pc()

// run the dest coroutine (for the first time)
coroutine_enter:
    movl $1, (%rdi)         # co->state = STARTED
    movq 16(%rdi), %rsp     # switch to coroutine`s stack
    pushq %rdi
    movq %rdi, %rax
    movq %rdx, %rdi
    call *32(%rax)          # co->entry(arg)

// the coroutine exited, switch back to its last caller
coroutine_exit:
    popq %rdi
    movq 24(%rdi), %rsi     # get the last caller
    movq 16(%rsi), %rsp     # restore its stack
    popq %rbp
    pushq %rsi
    call coroutine_terminated
    popq %rsi
    movq $0, %rax
    jmp *8(%rsi)            # caller->pc()

coroutine_error:
    movq $0, %rax
    ret
