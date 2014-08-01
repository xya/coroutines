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

.globl invoke_function_aapcs

invoke_function_aapcs:
@@ uint32_t invoke_function_aapcs(func_ptr fn, uint32_t *args, size_t n)
@@ r4: fn, r5: args, r6: n r7: n_copy
    push {r4, r5, r6, r7, lr} @@ preserve non-scratch registers
    mov r4, r0                @@ copy invocation parameters
    mov r5, r1                @@ ...
    mov r6, r2                @@ ...
    mov r7, r2
copy_reg_args:
    subs r6, r6, #1           @@ Copy function argument 1
    ldrpl r0, [r5], #4        @@ ...
    subpls r6, r6, #1         @@ Copy function argument 2
    ldrpl r1, [r5], #4        @@ ...
    subpls r6, r6, #1         @@ Copy function argument 3
    ldrpl r2, [r5], #4        @@ ...
    subpls r6, r6, #1         @@ Copy function argument 4
    ldrpl r3, [r5], #4        @@ ...
    subpls r6, r6, #1         @@ Copy remaining function arguments, if any
    bmi do_invoke             @@ ...
copy_args_loop:
    ldr r14, [r5], #4         @@ push one argument to the stack
    push {r14}                @@ ...
    subs r6, r6, #1           @@ ...
    bpl copy_args_loop        @@ ...
do_invoke:
    blx r4                    @@ call the function
    subs r7, r7, #5           @@ clean the stack from function arguments
    bmi return
clean_stack:
    pop {r1}                  @@ pop one argument from the stack
    subs r7, r7, #1           @@ ...
    bpl clean_stack           @@ ...
return:
    pop {r4, r5, r6, r7, pc}  @@ restore non-scratch registers and return to caller
