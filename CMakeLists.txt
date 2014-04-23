cmake_minimum_required(VERSION 2.8)
project(Chess CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic -pthread")

include_directories(${CMAKE_SOURCE_DIR}/source/sexp)
include_directories(${CMAKE_SOURCE_DIR}/source)

file(GLOB_RECURSE CHESS_SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
	)

option(DEBUG_OUTPUT "Debug Output" ON)
option(STRICT_DEADLINE "Enable a more forceful deadline" OFF)

if(DEBUG_OUTPUT)
	add_definitions(-DDEBUG_OUTPUT)
endif()

if(STRICT_DEADLINE)
	add_definitions(-DSTRICT_DEADLINE)
endif()

add_executable(client ${CHESS_SOURCE})


