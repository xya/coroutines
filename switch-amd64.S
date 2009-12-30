.section .text

.globl current
.globl coroutine_switch

coroutine_switch:
// save the current context (registers and arg)
    cmpl $2, (%rdi)         # is the coroutine finished?
    je coroutine_error
    popq 8(%rsi)            # src->pc = return address of the caller
    pushq %rbp
    movq %rsp, 16(%rsi)     # src->stack = current stack
    movq %rsi, 24(%rdi)     # dst->caller = src
    movq %rdi, current      # current = dst
    cmpl $0, (%rdi)         # has the coroutine not been started before?
    je coroutine_enter

// when rescheduled, restore the context and return the result 
coroutine_do_resume:
    movq 16(%rdi), %rsp     # restore the dest coroutine`s stack
    popq %rbp               # restore rbp
    movq %rdx, %rax         # <return value> = arg
    jmp *8(%rdi)            # dst->pc()

// run the dest coroutine (for the first time)
coroutine_enter:
    movl $1, (%rdi)         # dst->state = STARTED
    movq 16(%rdi), %rsp     # switch to the dest coroutine`s stack
    pushq %rdi
    movq %rdi, %rax
    movq %rdx, %rdi
    call *32(%rax)          # dst->entry(arg)

// the coroutine exited, switch back to its last caller
coroutine_exit:
    popq %rdi
    movq 24(%rdi), %rsi     # get the last caller
    movq 16(%rsi), %rsp     # restore its stack
    popq %rbp
    movl $2, (%rdi)         # dst->state = FINISHED
    movq $0, 24(%rdi)       # dst->caller = NULL
    movq %rsi, current      # current = caller
    movq $0, %rax
    jmp *8(%rsi)            # src->pc()

coroutine_error:
    movq $0, %rax
    ret
