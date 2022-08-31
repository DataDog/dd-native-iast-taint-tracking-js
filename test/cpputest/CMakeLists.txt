cmake_minimum_required (VERSION 3.7)
project (cpptest)

add_executable(native_test main.cc container/pool.cc WeakObjIface.cc WeakMap.cc)
target_include_directories(native_test PUBLIC ../../src)
target_link_libraries(native_test LINK_PUBLIC CppUTest)
