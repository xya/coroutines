.section .text

// references in coroutine.c
.globl current

// void *coroutine_switch(coroutine_t co, void *arg);
// ecx: co eax: current/arg edx: ctx
coroutine_switch:   .globl coroutine_switch
// save the current context (stack, return address and registers)
    movl 12(%esp), %edx       # load ctx
    movl (%edx), %eax         # current = ctx->current
    pop 4(%eax)               # current->ret_addr = caller
    movl (%esp), %ecx         # load co
    movl %ecx, (%edx)         # ctx->current = co
    movl %eax, 12(%ecx)       # co->caller = current
    push %ebp                 # save stack frame
    movl %esp, 8(%eax)        # current->stack = current stack
    movl 8(%esp), %eax        # load arg
    testl $0x1, (%ecx)        # if(!(co->ctx_state & STARTED))
    jz coroutine_entry        #   call coroutine_entry

// void *coroutine_return(coroutine_t co, void *arg)
// ecx: co, eax: arg
coroutine_return:
    movl 8(%ecx), %esp      # restore the coroutine's stack
    popl %ebp               # restore the frame pointer
    push 4(%ecx)            # return to co's caller
    ret

// void coroutine_entry(coroutine_t co, void *arg) no_return
// run the dest coroutine (for the first time)
coroutine_entry:
// ecx: co, eax: arg
    orl $0x1, (%ecx)        # co->state |= STARTED
    movl 8(%ecx), %esp      # switch to coroutine's stack
    push %ecx               # save co
    movl (%ecx), %edx       # ctx = co->ctx_state & ~STATE_MASK
    andl $-8, %edx          # ...
    push %eax               # co->entry(ctx->user, arg)
    push 12(%edx)           # ...
    call *16(%ecx)          # ...

// the coroutine exited, switch back to its last caller
// ecx: caller, eax: co, edx: ctx
    add $8, %esp            # clean the stack after the call
    pop %eax                # restore co
    movl (%eax), %edx       # ctx = co->ctx_state & ~STATE_MASK
    andl $-8, %edx          # ...
    orl $0x2, (%eax)        # co->state |= FINISHED
    movl 12(%eax), %ecx     # current = co->caller
    movl %ecx, (%edx)       # ...
    movl $0, 12(%eax)       # co->caller = NULL
    movl $0, %eax           # coroutine_return(current, 0, ctx)
    jmp coroutine_return
