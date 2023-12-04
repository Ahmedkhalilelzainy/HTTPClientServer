cmake_minimum_required(VERSION 3.26)
project(untitled3)

set(CMAKE_CXX_STANDARD 17)

add_executable(untitled3 main.cpp
        ClientDir/Client.cpp
        ServerDir/Server.cpp
        helper.cpp)
