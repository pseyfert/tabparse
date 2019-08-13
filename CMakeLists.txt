cmake_minimum_required(VERSION 3.11) # honestly, this is a guess
cmake_policy(SET CMP0048 NEW)
project(tabparse
  VERSION 0
  LANGUAGES CXX
  DESCRIPTION "a c++ command line parser that integrates with zsh tab completions"
  )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tabparse SHARED src/v_opt.cpp src/parser.cpp)
include_directories(include)

add_executable(test example/test.cpp)
target_link_libraries(test tabparse)
# TODO: generate completion on install
# TODO: all that's necessary for installing

## dependencies
find_package(fmt)
target_link_libraries(tabparse fmt::fmt)
