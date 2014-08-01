@@ Copyright (c) 2011, Pierre-Andre Saulais <pasaulais@free.fr>
@@ All rights reserved.
@@ 
@@ Redistribution and use in source and binary forms, with or without
@@ modification, are permitted provided that the following conditions are met:
@@ * Redistributions of source code must retain the above copyright
@@   notice, this list of conditions and the following disclaimer.
@@ * Redistributions in binary form must reproduce the above copyright
@@   notice, this list of conditions and the following disclaimer in the
@@   documentation and/or other materials provided with the distribution.
@@ * Neither the name of the <organization> nor the
@@   names of its contributors may be used to endorse or promote products
@@   derived from this software without specific prior written permission.
@@ 
@@ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
@@ ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
@@ WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
@@ DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
@@ DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
@@ (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
@@ LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
@@ ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
@@ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
@@ SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

.section .text
.align 4
.arm

@ void *coroutine_switch(coroutine_t co, void *arg, coroutine_context_t ctx);
@ r0: co, r1: arg, r2: ctx, r3: current, r14: ctx_state
.globl coroutine_switch
coroutine_switch:
@ save the current context (stack, return address and registers)
    ldr r3, [r2]                @ current = ctx->current
    str r0, [r2]                @ ctx->current = co
    push {fp}                   @ save stack frame
    str lr, [r3, #4]            @ current->ret_addr = caller
    ldr r14, [r0]               @ ctx_state = co->ctx_state
    str r3, [r0, #12]           @ co->caller = current
    str sp, [r3, #8]            @ current->stack = stack pointer
    ands r14, r14, #0x1         @ if(!(co->ctx_state & STARTED))
    beq coroutine_entry         @   call coroutine_entry

@ void *coroutine_return(coroutine_t co, void *arg, coroutine_context_t ctx)
@ r0: co, r1: arg, r2: ctx, r3: ret_addr
coroutine_return:
    ldr sp, [r0, #8]            @ restore the coroutine's stack
    pop {fp}
    mov r3, r0                  @ return arg to co's caller
    mov r0, r1
    ldr pc, [r3, #4]

@ void coroutine_entry(coroutine_t co, void *arg, coroutine_context_t ctx) no_return
@ run the dest coroutine (for the first time)
coroutine_entry:
@ r0: co, r1: arg, r2: ctx, r3: entry,  r14: ctx_state
    ldr r14, [r0]               @ co->ctx_state |= STARTED
    ldr sp, [r0, #8]            @ switch to coroutine's stack
    orr r14, r14, #0x1          @ co->ctx_state |= STARTED
    str r14, [r0]               @ co->ctx_state |= STARTED
    stmfd sp!, {r0, r2}         @ save co and ctx
    mov r3, r0                  @ co->entry(ctx->user, arg)
    ldr r0, [r2, #12]           @ ...
    ldr r3, [r3, #16]           @ ...
    blx r3                      @ ...

@ the coroutine exited, switch back to its last caller
@ r0: caller, r1/r3: co, r2: ctx, r14: ctx_state
    ldmfd sp!, {r1, r2}         @ restore co and ctx
    ldr r14, [r1]               @ co->ctx_state |= FINISHED
    orr r14, r14, #0x2          @ ...
    str r14, [r1]               @ ...
    mov r3, r1
    ldr r0, [r3, #12]           @ current = co->caller
    str r0, [r2]                @ ...
    mov r1, #0                  @ co->caller = NULL
    str r1, [r3, #12]           @ ...
    b coroutine_return          @ coroutine_return(current, 0, ctx)
