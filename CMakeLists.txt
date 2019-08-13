project(placeholder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(dummy SHARED src/v_opt.cpp src/parser.cpp)
include_directories(include)

add_executable(test example/test.cpp)
target_link_libraries(test dummy)

find_package(fmt)
target_link_libraries(dummy fmt::fmt)
