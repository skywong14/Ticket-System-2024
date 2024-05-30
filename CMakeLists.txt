cmake_minimum_required(VERSION 3.22)
project(Ticket_System_2024)

set(CMAKE_CXX_STANDARD 20)

include_directories(codes/My_Stl)
include_directories(codes/tools)

add_executable(codes
        codes/My_Stl/exceptions.hpp
        codes/My_Stl/vector.hpp
        codes/src/main.cpp
        codes/tools/BPTree.hpp
        codes/tools/MyTools.hpp
        codes/src/user.cpp
        codes/src/user.hpp
        codes/src/train.hpp
        codes/My_Stl/map.hpp
        codes/tools/MyTools.cpp
        codes/src/order.hpp
        codes/src/order.cpp
        codes/tools/LinerMemory.h
        codes/src/train.cpp)
