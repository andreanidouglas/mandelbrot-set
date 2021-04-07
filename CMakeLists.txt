cmake_minimum_required(VERSION 3.10)
project(mandelbrot-set) 


# -- Build Libraries

add_library(img SHARED
        img.c
)

target_include_directories(img
        PUBLIC
        ${PROJECT_SOURCE_DIR}
)

add_library(queue SHARED
        queue.c
)

target_include_directories(queue                                                  
        PUBLIC                                                                  
        ${PROJECT_SOURCE_DIR}                                                  
)

find_package (PNG REQUIRED)
find_package (Threads REQUIRED)

if (PNG_FOUND)
        message ( "PNG FOUND AT:" ${PNG_INCLUDE_DIRS})
endif()

# -- Build Executable

add_executable(Mandelbrot mandelbrot.c)

target_include_directories(Mandelbrot
        PUBLIC 
        ${PROJECT_SOURCE_DIR}
)

include_directories(${PNG_INCLUDE_DIRS})

target_link_libraries(Mandelbrot
        PRIVATE
        ${PNG_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        queue
        img
        )
