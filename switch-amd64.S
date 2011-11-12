.section .text

// references in coroutine.c
.globl current

// void *coroutine_switch(coroutine_t co, void *arg);
// rdi: co, rsi: arg, rax: current, rdx: state
coroutine_switch:   .globl coroutine_switch
// save the current context (stack, return address and registers)
    movq current, %rax      # current->ret_addr = caller
    popq 8(%rax)
    movq %rdi, current      # current = co
    movq %rax, 24(%rdi)     # co->caller = current
    pushq %rbp              # current->stack = current stack
    movq %rsp, 16(%rax)
    movl (%rdi), %edx       # call coroutine_entry if co hasn't started yet
    and $0x10000000, %edx
    cmpl $0, %edx
    je coroutine_entry

// void *coroutine_return(coroutine_t co, void *arg)
// rdi: co, rsi: arg
coroutine_return:
    movq 16(%rdi), %rsp     # restore the coroutine`s stack
    popq %rbp               # restore rbp
    movq %rsi, %rax         # <return value> = arg
    push 8(%rdi)            # return to co's caller
    ret

// void coroutine_entry(coroutine_t co, void *arg) no_return
// run the dest coroutine (for the first time)
coroutine_entry:
// rdi: co, rsi: arg
    orl $0x10000000, (%rdi) # co->state |= STARTED
    movq 16(%rdi), %rsp     # switch to coroutine`s stack
    pushq %rdi              # save co
    movq %rdi, %rdx         # co->entry(arg)
    movq %rsi, %rdi
    call *32(%rdx)

// the coroutine exited, switch back to its last caller
// rdi: caller, rsi: co
    popq %rsi               # restore co
    orl $0x80000000, (%rsi) # co->state |= FINISHED
    movq 24(%rsi), %rdi     # current = co->caller
    movq %rdi, current
    movq $0, 24(%rsi)       # co->caller = NULL
    movq $0, %rsi           # coroutine_return(current, 0)
    jmp coroutine_return
