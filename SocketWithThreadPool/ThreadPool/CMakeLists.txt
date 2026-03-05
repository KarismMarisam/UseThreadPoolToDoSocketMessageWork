cmake_minimum_required(VERSION 3.31.6)
project(practice)#set name

file(GLOB POOL cpps/*.cpp)#if use GLOB_RECURSE,it will search in sub-directory

INCLUDE_DIRECTORIES(./heads)

set(LIBRARY_OUTPUT_PATH ../)

add_library(ThPool SHARED ${POOL})