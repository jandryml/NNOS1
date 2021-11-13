cmake_minimum_required(VERSION 3.0)
project(NNOS1 C)

set(CMAKE_C_STANDARD 99)

add_executable(cv1 cv1/atexit-once.c)

add_executable(sysV examples/semaphore_system_v.c )
