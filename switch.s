.section .text

.globl coroutine_switch

coroutine_switch:
            pushq %rbp
            movq %rsp, %rbp
    // save the current context (registers and arg)
            //pusha
            //pushf
        
    // pause the source coroutine
            movl $2, (%rsi)         # src->state = PAUSED
            //movq _end, 4(%rsi)      # src->pc = _end
            movq %rsi, 20(%rdi)     # dst->caller = src
            cmpq $0, (%rdi)
            jne _resume
            
    // run the dest coroutine (for the first time)
            pushq %rdi
            movq %rdi, %rax
            movq %rdx, %rdi
            call *32(%rax)          # dst->entry(arg)
            popq %rdi
            jmp _end
    
    // resume the dest coroutine
_resume:    movl $1, (%rdi)         # dst->state = RUNNING
            jmp *4(%rdi)            # dst->pc()

 _end:      movq $0, %rax
            leave
            ret

// save the current context (registers and arg)
// switch to coroutine 'co' (i.e., call co->pc)
// when rescheduled, restore the context and return the result
