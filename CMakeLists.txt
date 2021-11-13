cmake_minimum_required(VERSION 3.0)
project(NNOS1 C)

set(CMAKE_C_STANDARD 99)

find_package (Threads)

add_executable(cv1 cv1/atexit-once.c)
add_executable(cv2 cv2/pthread_cleanup_sem.c)

#add_executable(sysV examples/semaphore_system_v.c )


target_link_libraries (cv2 ${CMAKE_THREAD_LIBS_INIT})
