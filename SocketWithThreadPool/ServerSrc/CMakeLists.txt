cmake_minimum_required(VERSION 3.31.6)
project(Server)#set name

set(EXECUTABLE_OUTPUT_PATH ../../)
LINK_DIRECTORIES(../)
LINK_LIBRARIES(libThPool.so)#LINK_LIBRARIES(../libThPool.so) cant find so!

add_executable(server server.cpp)
