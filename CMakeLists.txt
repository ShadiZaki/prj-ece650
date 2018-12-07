cmake_minimum_required(VERSION 2.8)
project(prj-ece650)

if (EXISTS "${CMAKE_SOURCE_DIR}/minisat")
  add_subdirectory(minisat)
else()
  message (FATAL_ERROR "MiniSat not found. Use `git clone https://github.com/agurfinkel/minisat` to check it out.")
endif()

set (CMAKE_CXX_FLAGS "-std=c++11 -lpthread -Wall ${CMAKE_CXX_FLAGS}")

include_directories(${CMAKE_SOURCE_DIR}/minisat)


add_executable(prj-ece650 prj-ece650.cpp parser.cpp parser.hpp edge.cpp edge.hpp)
target_link_libraries(prj-ece650 minisat-lib-static)