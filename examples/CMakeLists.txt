# Copyright (c) 2011, 2014, Pierre-Andre Saulais <pasaulais@free.fr>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# * Neither the name of the <organization> nor the
#   names of its contributors may be used to endorse or promote products
#   derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

include_directories(../include)

add_executable(pingpong pingpong.c)
target_link_libraries(pingpong coroutine)

add_executable(nested nested.c)
target_link_libraries(nested coroutine)

add_executable(barrier barrier.c barrier.h)
target_link_libraries(barrier coroutine)

if(NOT HAVE_PTHREAD_H)
  find_path(PTHREADS_INCLUDE_DIR pthread.h)
  find_library(PTHREADS_LIBRARY pthread DOC "The Portable Threads Library")
  if(PTHREADS_INCLUDE_DIR AND PTHREADS_LIBRARY)
    set(HAVE_PTHREAD_H ON)
    include_directories(${PTHREADS_INCLUDE_DIR})
  endif(PTHREADS_INCLUDE_DIR AND PTHREADS_LIBRARY)
endif(NOT HAVE_PTHREAD_H)

if(HAVE_PTHREAD_H)
  add_definitions(-D_GNU_SOURCE)
  add_executable(barrier2 barrier2.c barrier2.h)
  target_link_libraries(barrier2 coroutine pthread)
endif(HAVE_PTHREAD_H)

add_executable(test_poll test_poll.c)
target_link_libraries(test_poll coroutine)

if(${COROUTINE_ARCH} STREQUAL ARM)
  add_executable(test_invoke test_invoke.c)
  target_link_libraries(test_invoke coroutine)
endif(${COROUTINE_ARCH} STREQUAL ARM)
