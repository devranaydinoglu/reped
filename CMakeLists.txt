cmake_minimum_required(VERSION 3.20)

project(reped)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES 
    src/main.cpp
    src/application.cpp
    src/networking/server.cpp
    src/networking/client.cpp
    src/ui/window.cpp
    src/ui/setup_window.cpp
)

find_package(SDL3 REQUIRED)

set(IMGUI_PATH ${CMAKE_SOURCE_DIR}/vendor/imgui)
set(IMGUI_SOURCES 
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp
    ${IMGUI_PATH}/imgui_tables.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_impl_opengl3.cpp
    ${IMGUI_PATH}/imgui_impl_sdl3.cpp
    ${IMGUI_PATH}/imgui_demo.cpp
)

add_executable(${PROJECT_NAME} 
    ${SOURCES}
    ${IMGUI_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${IMGUI_PATH}
)

find_library(OpenGL_FRAMEWORK OpenGL)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    SDL3::SDL3
    ${OpenGL_FRAMEWORK}
)