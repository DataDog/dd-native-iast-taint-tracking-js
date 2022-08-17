cmake_minimum_required (VERSION 3.7)
project (cpptest)

#add_definitions(-include CppUTest/MemoryLeakDetectorNewMacros.h)
#add_definitions(-include CppUTest/MemoryLeakDetectorMallocMacros.h)

add_executable(native_test main.cc pool.cc)
target_include_directories(native_test PUBLIC ../../src)
target_link_libraries(native_test LINK_PUBLIC CppUTest)
