.section .text

// references in coroutine.c
.globl current

// void *coroutine_switch(coroutine_t co, void *arg);
// ecx: co, eax: arg, edx: current/state
coroutine_switch:   .globl coroutine_switch
// save the current context (stack, return address and registers)
    movl current, %edx      # current->ret_addr = caller
    pop 4(%edx)
    movl (%esp), %ecx       # current = co
    movl %ecx, current
    movl %edx, 12(%ecx)     # co->caller = current
    movl 4(%esp), %eax      # load arg
    push %ebp               # current->stack = current stack
    movl %esp, 8(%edx)
    movl (%ecx), %edx       # call coroutine_entry if co hasn't started yet
    and $0x10000000, %edx
    cmpl $0, %edx
    je coroutine_entry

// void *coroutine_return(coroutine_t co, void *arg)
// ecx: co, eax: arg
coroutine_return:
    movl 8(%ecx), %esp      # restore the coroutine`s stack
    popl %ebp               # restore rbp
    push 4(%ecx)            # return to co's caller
    ret

// void coroutine_entry(coroutine_t co, void *arg) no_return
// run the dest coroutine (for the first time)
coroutine_entry:
// ecx: co, eax: arg
    orl $0x10000000, (%ecx) # co->state |= STARTED
    movl 8(%ecx), %esp      # switch to coroutine`s stack
    push %ecx               # save co
    push %eax
    call *16(%ecx)          # co->entry(arg)

// the coroutine exited, switch back to its last caller
// ecx: caller, eax: co
    add $4, %esp
    pop %eax                # restore co
    orl $0x80000000, (%eax) # co->state |= FINISHED
    movl 12(%eax), %ecx     # current = co->caller
    movl %ecx, current
    movl $0, 12(%eax)       # co->caller = NULL
    movl $0, %eax           # coroutine_return(current, 0)
    jmp coroutine_return
