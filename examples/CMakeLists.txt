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
