// Copyright (c) 2011, Pierre-Andre Saulais <pasaulais@free.fr>
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

#include <stdio.h>
#include <inttypes.h>

extern uint32_t invoke_function_aapcs(void *fn, uint32_t *args, size_t n);

int f0()
{
    return 42;
}

int f1(int x)
{
    printf("f1 received: %d\n", x);
    return x + 1;
}

int f2(int x, int y)
{
    printf("f2 received: %d %d\n", x, y);
    return x + y;
}

int f3(int x, int y, int z)
{
    printf("f3 received: %d %d %d\n", x, y, z);
    return (x * y) + z;
}

int f4(int x, int y, int z, int w)
{
    printf("f4 received: %d %d %d %d\n", x, y, z, w);
    return (x * y) + (z * w);
}

int f5(int x, int y, int z, int w, int u)
{
    printf("f5 received: %d %d %d %d %d\n", x, y, z, w, u);
    return (x * y) + (z * w) + u;
}

int f6(int x, int y, int z, int w, int u, int v)
{
    printf("f6 received: %d %d %d %d %d %d\n", x, y, z, w, u, v);
    return (x * y) + (z * w) + (u * v);
}

int main(int argc, char **argv)
{
    int args[] = {2, 3, 5, 7, 11, 13};
    printf("f0() -> %d\n", invoke_function_aapcs((void *)f0, 0, 0));
    printf("f1(%d) -> %d\n", args[0], invoke_function_aapcs((void *)f1, args, 1));
    printf("f2(%d, %d) -> %d\n", args[0], args[1], invoke_function_aapcs((void *)f2, args, 2));
    printf("f3(%d, %d, %d) -> %d\n", args[0], args[1], args[2], invoke_function_aapcs((void *)f3, args, 3));
    printf("f4(%d, %d, %d, %d) -> %d\n", args[0], args[1], args[2], args[3], invoke_function_aapcs((void *)f4, args, 4));
    printf("f5(%d, %d, %d, %d, %d) -> %d\n", args[0], args[1], args[2], args[3], args[4], invoke_function_aapcs((void *)f5, args, 5));
    printf("f6(%d, %d, %d, %d, %d, %d) -> %d\n", args[0], args[1], args[2], args[3], args[4], args[5], invoke_function_aapcs((void *)f6, args, 6));
    return 0;
}
