# Copyright (c) 2011, Pierre-Andre Saulais <pasaulais@free.fr>
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

cmake_minimum_required(VERSION 2.8)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  option(BUILD_32_BITS "Build 32 bits executables and libraries." OFF)
else()
  option(BUILD_32_BITS "Build 32 bits executables and libraries." ON)
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

option(BUILD_EXAMPLES "Build examples." ON)

if(ANDROID)
  set(COROUTINE_ARCH ARM)
elseif(BUILD_32_BITS)
  set(COROUTINE_ARCH x86)
else()
  set(COROUTINE_ARCH x86_64)
endif(ANDROID)

message(STATUS "Selected architecture: ${COROUTINE_ARCH}")

set(COROUTINE_SRC src/coroutine.c include/coroutine.h)
include_directories(include)

if(${COROUTINE_ARCH} STREQUAL x86_64)
  set(COROUTINE_SRC ${COROUTINE_SRC} src/switch-amd64.S)
  if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
  endif(CMAKE_COMPILER_IS_GNUCC)
elseif(${COROUTINE_ARCH} STREQUAL x86)
  set(COROUTINE_SRC ${COROUTINE_SRC} src/switch-x86.S)
  if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
  endif(CMAKE_COMPILER_IS_GNUCC)
elseif(${COROUTINE_ARCH} STREQUAL ARM)
  set(COROUTINE_SRC ${COROUTINE_SRC} src/switch-arm.S src/invoke-arm.S)
else()
  message(FATAL_ERROR Unknown architecture)
endif(${COROUTINE_ARCH} STREQUAL x86_64)

if(CMAKE_COMPILER_IS_GNUCC)
  set_source_files_properties(${COROUTINE_SRC} PROPERTIES LANGUAGE C) 
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=c99")
endif(CMAKE_COMPILER_IS_GNUCC)

add_library(coroutine ${COROUTINE_SRC})

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif(BUILD_EXAMPLES)
