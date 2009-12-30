.section .text

.globl current
.globl coroutine_switch

coroutine_switch:
    //pushq %rbp
    //movq %rsp, %rbp
// save the current context (registers and arg)
    //pusha
    //pushf
        
// pause the source coroutine
    movl $2, (%rsi)         # src->state = PAUSED
    movq $coroutine_switch_resumed, 8(%rsi) # src->pc = coroutine_switch_resumed                        
    movq %rsi, 24(%rdi)     # dst->caller = src
    movq %rdi, current      # current = dst
    cmpl $0, (%rdi)
    je coroutine_switch_run
    
// resume the dest coroutine
coroutine_switch_resume:
    movl $1, (%rdi)         # dst->state = RUNNING
    movq %rdx, %rax         # <return value> = arg
    jmp *8(%rdi)            # dst->pc()
            
// run the dest coroutine (for the first time)
coroutine_switch_run:
    movl $1, (%rdi)         # dst->state = RUNNING
    pushq %rdi
    movq %rdi, %rax
    movq %rdx, %rdi
    call *32(%rax)          # dst->entry(arg)
    popq %rdi
    movl $3, (%rdi)         # dst->state = FINISHED
    movq $0, %rax
    //leave
    ret

// when rescheduled, restore the context and return the result     
coroutine_switch_resumed:
    //leave
    ret
