// Copyright (c) 2009, 2011, Pierre-Andre Saulais <pasaulais@free.fr>
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// * Neither the name of the <organization> nor the
//   names of its contributors may be used to endorse or promote products
//   derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

.intel_syntax noprefix
.section .text

// void *coroutine_switch(coroutine_t co, void *arg, coroutine_context_t ctx);
// rdi: co, rsi: arg, rdx: ctx, rax: current
coroutine_switch:   .globl coroutine_switch
// save the current context (stack, return address and registers)
    mov rax, qword ptr [rdx]        // ctx->current->ret_addr = caller
    pop qword ptr [rax+8]
    mov qword ptr [rdx], rdi        // ctx->current = co
    mov qword ptr [rdi+24], rax     // co->caller = current
    push rbp                        // current->stack = current stack
    mov qword ptr [rax+16], rsp
    test qword ptr [rdi], 1         // if(!(co->ctx_state & STARTED))
    jz coroutine_entry              //   call coroutine_entry

// void *coroutine_return(coroutine_t co, void *arg, coroutine_context_t ctx)
// rdi: co, rsi: arg, rdx: ctx
coroutine_return:
    mov rsp, qword ptr [rdi+16]     // restore the coroutine`s stack
    pop rbp                         // restore rbp
    mov rax, rsi                    // <return value> = arg
    push qword ptr [rdi+8]          // return to co's caller
    ret

// void coroutine_entry(coroutine_t co, void *arg, coroutine_context_t ctx) no_return
// run the dest coroutine (for the first time)
coroutine_entry:
// rdi: co, rsi: arg, rcx: entry, rdx: ctx
    or qword ptr [rdi], 1           // co->state |= STARTED
    mov rsp, qword ptr [rdi+16]     // switch to coroutine`s stack
    push rdi                        // save co
    mov rcx, rdi                    // co->entry(ctx->user, arg)
    mov rdi, qword ptr [rdx+24]
    call qword ptr [rcx+32]

// the coroutine exited, switch back to its last caller
// rdi: caller, rsi: co, rdx: ctx
    pop rsi                         // restore co
    mov rdx, qword ptr [rsi]        // ctx = co->ctx_state & ~STATE_MASK
    and rdx, -8                     // ...
    or qword ptr [rsi], 2           // co->state |= FINISHED
    mov rdi, qword ptr [rsi+24]     // current = co->caller
    mov qword ptr [rdx], rdi
    mov qword ptr [rsi+24], 0       // co->caller = NULL
    mov rsi, 0                      // coroutine_return(current, 0, ctx)
    jmp coroutine_return
