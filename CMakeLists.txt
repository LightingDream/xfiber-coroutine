cmake_minimum_required(VERSION 3.0.0)
project(xfiber-coroutine VERSION 0.1.0)

include_directories(./include)
file(GLOB_RECURSE CPP_FILES     
${PROJECT_SOURCE_DIR}/src/*.cc 
${PROJECT_SOURCE_DIR}/src/*.cpp
)
add_executable(xfiber-coroutine main.cpp ${CPP_FILES})

