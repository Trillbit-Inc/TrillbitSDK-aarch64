cmake_minimum_required(VERSION 3.22)

project(trill_linux)

if(NOT DEFINED PLATFORM_OPTION)
  set(PLATFORM_OPTION "x86_64")
endif()

add_subdirectory("main")
