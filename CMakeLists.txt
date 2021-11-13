cmake_minimum_required(VERSION 3.0)
project(NNOS1 C)

set(CMAKE_C_STANDARD 99)

find_package (Threads)

#add_executable(cv1 cv1/atexit-once.c)
#
#add_executable(cv2 cv2/pthread_cleanup_sem.c)
#target_link_libraries (cv2 ${CMAKE_THREAD_LIBS_INIT})
#
#add_executable(cv3-1 cv3/bank_withdraw_SW1.c)
#target_link_libraries (cv3-1 ${CMAKE_THREAD_LIBS_INIT})
#
#add_executable(cv3-2 cv3/bank_withdraw_SW1_sched.c)
#target_link_libraries (cv3-2 ${CMAKE_THREAD_LIBS_INIT})

#add_executable(cv3-3 cv3/bank_withdraw_xchg.c) - not working
#target_link_libraries (cv3-3 ${CMAKE_THREAD_LIBS_INIT})

#add_executable(cv3-4 cv3/bank_withdraw_xchg_sched.c) - not working
#target_link_libraries (cv3-4 ${CMAKE_THREAD_LIBS_INIT})

add_executable(cv4 cv4/test_pt_sem.c)
target_link_libraries (cv4 ${CMAKE_THREAD_LIBS_INIT})

#add_executable(sysV examples/semaphore_system_v.c )

