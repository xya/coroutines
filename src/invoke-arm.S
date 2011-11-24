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
