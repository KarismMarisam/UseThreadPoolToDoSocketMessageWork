cmake_minimum_required(VERSION 3.31.6)
project(practice)#set name
set(EXECUTABLE_OUTPUT_PATH ../)#set the directory to store final executable file,notice that current directory is Makefile's

# set(Cal Code/add.cpp Code/sub.cpp main.cpp)#make Cal presents the cpps to compile
# aux_source_directory(Code Cal)#search source files in specific directory(not including sub-directory)
file(GLOB test cpps/*.cpp)#if use GLOB_RECURSE,it will search in sub-directory
# file(GLOB Head Code/Include/*.h)
include_directories(./heads)#search heads 
add_executable(result ${test})