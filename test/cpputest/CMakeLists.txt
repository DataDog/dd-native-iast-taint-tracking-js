cmake_minimum_required (VERSION 3.7)
project (cpptest)


add_executable(native_test
                main.cc
                iast_manager.cc
                container/pool.cc
                container/queued_pool.cc
                weakiface.cc
                weakmap.cc)
set_property(TARGET native_test PROPERTY CXX_STANDARD 17)
target_include_directories(native_test PUBLIC ../../src)
target_link_libraries(native_test LINK_PUBLIC CppUTest)
