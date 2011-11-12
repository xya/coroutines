.section .text

// references in coroutine.c
.globl current
.globl coroutine_terminated

// void *coroutine_switch(coroutine_t co, void *arg);
// rdi: co, rsi: arg, rax: current, rdx: state
coroutine_switch:   .globl coroutine_switch
// save the current context (stack, return address and registers)
    movq current, %rax
    popq 8(%rax)            # current->ret_addr = caller
    pushq %rbp
    movq %rsp, 16(%rax)     # current->stack = current stack
    movq %rax, 24(%rdi)     # co->caller = current
    movq %rdi, current      # current = co
    movl (%rdi), %edx       # has the coroutine not been started before?
    and $0x10000000, %edx
    cmpl $0, %edx
    je coroutine_entry

// when rescheduled, restore the context and return the result 
coroutine_do_resume:
    movq 16(%rdi), %rsp     # restore the coroutine`s stack
    popq %rbp               # restore rbp
    movq %rsi, %rax         # <return value> = arg
    push 8(%rdi)            # return to co's caller
    ret

// run the dest coroutine (for the first time)
coroutine_entry:
    orl $0x10000000, (%rdi) # co->state |= STARTED
    movq 16(%rdi), %rsp     # switch to coroutine`s stack
    pushq %rdi
    movq %rdi, %rdx         # co->entry(arg)
    movq %rsi, %rdi
    call *32(%rdx)

// the coroutine exited, switch back to its last caller
    popq %rdi
    movq 24(%rdi), %rsi     # get the last caller
    movq 16(%rsi), %rsp     # restore its stack
    popq %rbp
    pushq %rsi
    call coroutine_terminated
    popq %rsi
    movq $0, %rax           # <return value> = 0
    push 8(%rsi)            # return to the last caller
    ret
