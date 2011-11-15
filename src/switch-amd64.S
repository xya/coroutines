.section .text

// void *coroutine_switch(coroutine_t co, void *arg, coroutine_context_t ctx);
// rdi: co, rsi: arg, rdx: ctx, rax: current
coroutine_switch:   .globl coroutine_switch
// save the current context (stack, return address and registers)
    movq (%rdx), %rax       # ctx->current->ret_addr = caller
    popq 8(%rax)
    movq %rdi, (%rdx)       # ctx->current = co
    movq %rax, 24(%rdi)     # co->caller = current
    pushq %rbp              # current->stack = current stack
    movq %rsp, 16(%rax)
    testq $0x1, (%rdi)      # if(!(co->ctx_state & STARTED))
    jz coroutine_entry      #   call coroutine_entry

// void *coroutine_return(coroutine_t co, void *arg, coroutine_context_t ctx)
// rdi: co, rsi: arg, rdx: ctx
coroutine_return:
    movq 16(%rdi), %rsp     # restore the coroutine`s stack
    popq %rbp               # restore rbp
    movq %rsi, %rax         # <return value> = arg
    push 8(%rdi)            # return to co's caller
    ret

// void coroutine_entry(coroutine_t co, void *arg, coroutine_context_t ctx) no_return
// run the dest coroutine (for the first time)
coroutine_entry:
// rdi: co, rsi: arg, rcx: entry, rdx: ctx
    orq $0x1, (%rdi)        # co->state |= STARTED
    movq 16(%rdi), %rsp     # switch to coroutine`s stack
    pushq %rdi              # save co
    movq %rdi, %rcx         # co->entry(ctx->user, arg)
    movq 24(%rdx), %rdi
    call *32(%rcx)

// the coroutine exited, switch back to its last caller
// rdi: caller, rsi: co, rdx: ctx
    popq %rsi               # restore co
    movq (%rsi), %rdx       # ctx = co->ctx_state & ~STATE_MASK
    andq $-8, %rdx          # ...
    orq $0x2, (%rsi)        # co->state |= FINISHED
    movq 24(%rsi), %rdi     # current = co->caller
    movq %rdi, (%rdx)
    movq $0, 24(%rsi)       # co->caller = NULL
    movq $0, %rsi           # coroutine_return(current, 0, ctx)
    jmp coroutine_return
