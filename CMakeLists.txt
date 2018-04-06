cmake_minimum_required(VERSION 3.10)
project(proj)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_FLAGS -m32)

add_executable(proj
        db.cpp
        db.h)
