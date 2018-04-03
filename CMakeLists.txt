cmake_minimum_required(VERSION 3.10)
project(proj)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(proj
        db.cpp
        db.h)
