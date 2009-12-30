.section .text

.globl current
.globl coroutine_switch

coroutine_switch:
// save the current context (registers and arg)
    popq 8(%rsi)            # src->pc = return address of the caller
    pushq %rbp
    movq %rsp, 16(%rsi)     # src->stack = current stack
    movq %rsi, 24(%rdi)     # dst->caller = src
    movl $2, (%rsi)         # src->state = PAUSED
    movq %rdi, current      # current = dst
    cmpl $0, (%rdi)         # has the coroutine been started before?
    je coroutine_enter
    
// when rescheduled, restore the context and return the result 
coroutine_do_resume:
    movl $1, (%rdi)         # dst->state = RUNNING
    movq 16(%rdi), %rsp     # restore the dest coroutine`s stack
    popq %rbp               # restore rbp
    movq %rdx, %rax         # <return value> = arg
    jmp *8(%rdi)            # dst->pc()
            
// run the dest coroutine (for the first time)
coroutine_enter:
    movl $1, (%rdi)         # dst->state = RUNNING
    movq 16(%rdi), %rsp     # switch to the dest coroutine`s stack
    pushq %rdi
    pushq %rsi
    movq %rdi, %rax
    movq %rdx, %rdi
    call *32(%rax)          # dst->entry(arg)

// the coroutine exited, switch back to its original caller
// TODO: switch back to the last caller, in case it is different
coroutine_exit:
    popq %rsi
    popq %rdi
    movq 16(%rsi), %rsp     # restore the callers stack
    popq %rbp
// TODO: call coroutine_exited() to free the stack and reap zombies
    movl $3, (%rdi)         # dst->state = FINISHED
    movq $0, %rax
    jmp *8(%rsi)            # src->pc()
    