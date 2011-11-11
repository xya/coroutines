.section .text

// references in coroutine.c
.globl current
.globl coroutine_terminated

// void *coroutine_switch(coroutine_t co, void *arg);
coroutine_switch:   .globl coroutine_switch
// save the current context (stack, return address and registers)
    movl 4(%esp), %ecx      # ecx = co
    cmpl $2, (%ecx)         # is the coroutine finished?
    je coroutine_error
    movl 8(%esp), %eax      # <return value> = arg
    movl current, %edx      # edx = current
    pop 4(%edx)             # current->pc = return address of the caller
    push %ebp
    movl %esp, 8(%edx)      # current->stack = current stack
    movl %edx, 12(%ecx)     # co->caller = current
    movl %ecx, current      # current = co
    cmpl $0, (%ecx)         # has the coroutine not been started before?
    je coroutine_enter

// when rescheduled, restore the context and return the result 
coroutine_do_resume:
    movl 8(%ecx), %esp      # restore the coroutine`s stack
    pop %ebp                # restore rbp
    jmp *4(%ecx)            # co->pc()

// run the dest coroutine (for the first time)
coroutine_enter:
    movl $1, (%ecx)         # co->state = STARTED
    movl 8(%ecx), %esp      # switch to coroutine`s stack
    push %ecx
    push %eax
    call *16(%ecx)          # co->entry(arg)

// the coroutine exited, switch back to its last caller
coroutine_exit:
    pop %eax                
    pop %ecx
    movl 12(%ecx), %edx     # get the last caller
    movl 8(%edx), %esp      # restore its stack
    pop %ebp
    push %ecx
    call coroutine_terminated
    pop %ecx
    movl $0, %eax
    jmp *4(%edx)            # caller->pc()

coroutine_error:
    movl $0, %eax
    ret
