cmake_minimum_required(VERSION 3.10)
project(mandelbrot-set) 


# -- Build Libraries
add_subdirectory(raylib)

add_library(queue SHARED
        queue.c
)

target_include_directories(queue                                                  
        PUBLIC                                                                  
        ${PROJECT_SOURCE_DIR}
        
        ${PROJECT_SOURCE_DIR}/raylib/src
)

find_package (Threads REQUIRED)

# -- Build Executable

add_executable(Mandelbrot mandelbrot.c)

target_include_directories(Mandelbrot
        PUBLIC 
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/raylib/src
        
)


target_link_libraries(Mandelbrot
        PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
        queue
        raylib
        )
