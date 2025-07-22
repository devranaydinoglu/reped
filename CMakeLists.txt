cmake_minimum_required(VERSION 3.20)

project(reped)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES 
    src/main.cpp
    src/networking/server.cpp
    src/networking/client.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
