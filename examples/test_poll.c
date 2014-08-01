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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "coroutine.h"

#define BUFFER_SIZE (4096 * 1)

typedef struct
{
    char *src_file;
    char *dst_file;
    fd_set read_fds;
    fd_set write_fds;
    int max_fd;
    int pipe[2];
    size_t sent;
    size_t received;
    coroutine_context_t ctx;
} SendReceiveOptions;

typedef struct
{
    int fd;
    int is_read;
    coroutine_t co;
} io_op;

typedef struct
{
    char msg_size[4];
    char msg_type;
    uint8_t blob_type;
    uint16_t transfer_id;
    uint32_t block_id;
    uint16_t block_size;
} BlockHeader;

void send_receive(SendReceiveOptions *opts);
void fill_fd_sets(SendReceiveOptions *opts, io_op **ops, int opcount);
int op_ready(SendReceiveOptions *opts, io_op *op);
int blocking_mode(int fd, int blocking);
ssize_t coroutine_read(coroutine_context_t ctx, int fd, void *buffer, size_t size);
ssize_t coroutine_write(coroutine_context_t ctx, int fd, void *buffer, size_t size);
void send_file(SendReceiveOptions *opts, coroutine_arg_t arg);
void receive_file(SendReceiveOptions *opts, coroutine_arg_t arg);
void set_msg_size(BlockHeader *bh);

int main(int argc, char **argv)
{
    char dst_file[256];
    clock_t start, duration;
    double sec, speed;
    SendReceiveOptions opts;
    coroutine_context_t ctx = NULL;
    
    if(argc < 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    snprintf(dst_file, sizeof(dst_file), "%s.1", argv[1]);
    opts.src_file = argv[1];
    opts.dst_file = dst_file;
    FD_ZERO(&opts.read_fds);
    FD_ZERO(&opts.write_fds);
    opts.max_fd = 0;
    pipe(opts.pipe);
    blocking_mode(opts.pipe[0], 0);
    blocking_mode(opts.pipe[1], 0);
    opts.sent = 0;
    opts.received = 0;
    opts.ctx = coroutine_create_context(4096 * 2);
    coroutine_set_context_data(opts.ctx, &opts);
    
    start = clock();
    send_receive(&opts);
    duration = clock() - start;
    sec = ((double)duration / (double)CLOCKS_PER_SEC);
    speed = ((double)opts.received / (1024.0 * 1024.0)) / sec;
    printf("Sent %zi bytes in %f seconds (%f MiB/s)\n", opts.received, sec, speed);
    coroutine_free_context(ctx);
    return 0;
}

void send_receive(SendReceiveOptions *opts)
{
    coroutine_t sender = coroutine_create(opts->ctx, (coroutine_func_t)send_file);
    coroutine_t receiver = coroutine_create(opts->ctx, (coroutine_func_t)receive_file);
    io_op *op[2] = {NULL, NULL};
    
    op[0] = coroutine_resume(sender, opts);
    op[1] = coroutine_resume(receiver, opts);
    while(op[0] || op[1])
    {
        fill_fd_sets(opts, op, 2);
        if(select(opts->max_fd, &opts->read_fds, &opts->write_fds, NULL, NULL) == -1)
        {
            perror("select()");
            break;
        }
        for(int i = 0; i < 2; i++)
        {
            if(op_ready(opts, op[i]))
                op[i] = coroutine_resume(op[i]->co, NULL);
        }
    }
    coroutine_free(sender);
    coroutine_free(receiver);
}

void fill_fd_sets(SendReceiveOptions *opts, io_op **ops, int opcount)
{
    io_op *op;
    int max_fd = 0;
    
    FD_ZERO(&opts->read_fds);
    FD_ZERO(&opts->write_fds);
    for(int i = 0; i < opcount; i++)
    {
        op = ops[i];
        if(op)
        {
            if(op->is_read)
                FD_SET(op->fd, &opts->read_fds);
            else
                FD_SET(op->fd, &opts->write_fds);
            if(opts->max_fd < (op->fd + 1))
                opts->max_fd = op->fd + 1;
        }
    }
}

int op_ready(SendReceiveOptions *opts, io_op *op)
{
    if(!op)
        return 0;
    else if(op->is_read)
        return FD_ISSET(op->fd, &opts->read_fds);
    else
        return FD_ISSET(op->fd, &opts->write_fds);
}

int blocking_mode(int fd, int blocking)
{
    int flag = O_NONBLOCK;
    int new_mode, old_mode = fcntl(fd, F_GETFL, flag);
    if(blocking)
        new_mode = (old_mode & ~flag);
    else
        new_mode = (old_mode | flag);
    fcntl(fd, F_SETFL, new_mode);
    return (old_mode & flag) == 0;
}

ssize_t coroutine_read(coroutine_context_t ctx, int fd, void *buffer, size_t size)
{
    io_op op;
    ssize_t bytesRead;
    
    //printf("coroutine_read(%i, %p, %zi)\n", fd, buffer, size);
    while(1)
    {
        // try to read it without blocking
        bytesRead = read(fd, buffer, size);
        if(bytesRead >= 0)
            return bytesRead;
        else if(errno != EAGAIN)
            return -1;
        
        // yield and pass the details
        op.fd = fd;
        op.is_read = 1;
        op.co = coroutine_current(ctx);
        coroutine_yield(ctx, &op);
    }
}

ssize_t coroutine_write(coroutine_context_t ctx,int fd, void *buffer, size_t size)
{
    io_op op;
    ssize_t bytesWritten;
    
    //printf("coroutine_write(%i, %p, %zi)\n", fd, buffer, size);
    while(1)
    {
        // try to write without blocking
        bytesWritten = write(fd, buffer, size);
        if(bytesWritten >= 0)
            return bytesWritten;
        else if(errno != EAGAIN)
            return -1;
        
        // yield and pass the details
        op.fd = fd;
        op.is_read = 0;
        op.co = coroutine_current(ctx);
        coroutine_yield(ctx, &op);
    }
}

void send_file(SendReceiveOptions *opts, coroutine_arg_t arg)
{
    BlockHeader bh;
    uint32_t blockID = 0;
    char buffer[BUFFER_SIZE];
    int readBytes;
    int r = open(opts->src_file, O_RDONLY | O_NONBLOCK, 0);
    
    if(r < 0)
        return;
    bh.msg_type = '\0';
    bh.blob_type = 1;
    bh.transfer_id = 0;
    while(1)
    {
        readBytes = coroutine_read(opts->ctx, r, buffer, sizeof(buffer));
        if(readBytes == 0)
            break;
        bh.block_id = blockID++;
        bh.block_size = readBytes;
        set_msg_size(&bh);
        coroutine_write(opts->ctx, opts->pipe[1], &bh, sizeof(BlockHeader));
        coroutine_write(opts->ctx, opts->pipe[1], buffer, readBytes);
        opts->sent += (size_t)readBytes;
    }
    close(r);
    close(opts->pipe[1]);
}

#define min(a, b) ((a) < (b)? (a) : (b))

void receive_file(SendReceiveOptions *opts, coroutine_arg_t arg)
{
    BlockHeader bh;
    char buffer[BUFFER_SIZE];
    int readBytes;
    int w = open(opts->dst_file, O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0666);
    
    if(w < 0)
        return;
    
    while(1)
    {
        readBytes = coroutine_read(opts->ctx, opts->pipe[0], &bh, sizeof(BlockHeader));
        if(readBytes == 0)
            break;
        readBytes = coroutine_read(opts->ctx, opts->pipe[0], buffer, min(bh.block_size, sizeof(buffer)));
        if(readBytes < bh.block_size)
        {
            fprintf(stderr, "Pipe was closed while receiving a block\n");
            break;
        }
        coroutine_write(opts->ctx, w, buffer, bh.block_size);
        opts->received += bh.block_size;
    }
    close(w);
    close(opts->pipe[0]);
}

void set_msg_size(BlockHeader *bh)
{
    size_t textSize = sizeof(bh->msg_size);
    int size = sizeof(*bh) - textSize + bh->block_size;
    if((size < 0x0000) && (size > 0xffff))
        size = 0;
    sprintf(bh->msg_size, "%04x", size);
}
