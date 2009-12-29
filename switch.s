.section .text

.globl coroutine_switch

coroutine_switch:
    pushq %rbp
    movq %rsp, %rbp
    movq $1, %rax
    leave
    ret

# save the current context (registers and arg)
# switch to coroutine 'co' (i.e., call co->pc)
# when rescheduled, restore the context and return the result
