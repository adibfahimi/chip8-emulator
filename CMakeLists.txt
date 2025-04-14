cmake_minimum_required(VERSION 3.15)
project(chip8_emulator LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT TARGET raylib)
    find_package(raylib QUIET)
    
    if(NOT raylib_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            raylib
            GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
            GIT_TAG "5.0"
        )
        
        set(BUILD_EXAMPLES OFF CACHE BOOL "")
        set(BUILD_GAMES OFF CACHE BOOL "")
        
        FetchContent_MakeAvailable(raylib)
    endif()
endif()

add_executable(${PROJECT_NAME} src/main.c src/chip8.c src/input.c)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm gdi32 shell32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(OPENGL_LIBRARY OpenGL)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${OPENGL_LIBRARY})
elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE X11 OpenGL::GL pthread dl m)
endif()
