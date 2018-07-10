cmake_minimum_required(VERSION 2.8)
#add_definitions(-std=c++11)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread" )
add_executable(Server 
 server.cpp
 main.cpp
)