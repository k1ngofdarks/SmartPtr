cmake_minimum_required(VERSION 3.29)
project(SmartPtr)

set(CMAKE_CXX_STANDARD 20)

add_executable(SmartPtr test_shared.cpp)
add_executable(WeakPtr test_weak.cpp)
