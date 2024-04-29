cmake_minimum_required(VERSION 3.23)
project(LAB3)

set(CMAKE_CXX_STANDARD 23)

#add_compile_definitions(TRACY_ENABLE)


add_executable(LAB3 main.cpp TracyClient.cpp)

target_link_libraries(LAB3 PRIVATE ws2_32)
target_link_libraries(LAB3 PRIVATE dbghelp)