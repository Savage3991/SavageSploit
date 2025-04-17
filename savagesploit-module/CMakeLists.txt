cmake_minimum_required(VERSION 3.30)
project(savagesploit_module)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 23)

# disable and enable shi for performance :P
add_compile_options(
        #/MT # not supported by our mapper atm... :(
        /MP
        /cgthreads8
        /arch:AVX2
)

add_library(${PROJECT_NAME} SHARED entry.cpp)