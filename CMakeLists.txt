cmake_minimum_required(VERSION 3.22)
project(Ticket_System_2024)

set(CMAKE_CXX_STANDARD 20)

include_directories(code/My_Stl)
include_directories(code/tools)

add_executable(code
        code/My_Stl/exceptions.hpp
        code/My_Stl/vector.hpp
        code/src/main.cpp
        code/tools/BPTree.hpp
        code/tools/MyTools.hpp
        code/src/user.cpp
        code/src/user.hpp
        code/src/train.hpp
        code/My_Stl/map.hpp
        code/tools/MyTools.cpp
        code/src/order.hpp
        code/src/order.cpp
        code/tools/LinerMemory.h
        code/src/train.cpp)
