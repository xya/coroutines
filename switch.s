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
    
    cmpl $0, (%rdi)
    je coroutine_entry
    
// when rescheduled, restore the context and return the result 
coroutine_do_resume:
    movl $1, (%rdi)         # dst->state = RUNNING
    movq 16(%rdi), %rsp     # restore the dest coroutines stack
    popq %rbp               # restore rbp
    movq %rdx, %rax         # <return value> = arg
    jmp *8(%rdi)            # dst->pc()
            
// run the dest coroutine (for the first time)
coroutine_entry:
    movl $1, (%rdi)         # dst->state = RUNNING
    movq 16(%rdi), %rsp     # switch to the dest coroutines stack
    pushq %rdi
    pushq %rsi
    movq %rdi, %rax
    movq %rdx, %rdi
    call *32(%rax)          # dst->entry(arg)
    popq %rsi
    popq %rdi
    movq 16(%rsi), %rsp     # restore the callers stack
    popq %rbp
    movl $3, (%rdi)         # dst->state = FINISHED
    movq $0, %rax
    ret
