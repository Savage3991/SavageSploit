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

add_library(${PROJECT_NAME} SHARED entry.cpp
        src/core/execution/execution.cpp
        src/core/execution/execution.h
        src/rbx/engine/game.h
        src/rbx/engine/luau.h
        src/rbx/engine/hyperion.h
        src/rbx/taskscheduler/taskscheduler.cpp
        src/rbx/taskscheduler/taskscheduler.h
        src/rbx/engine/game_structures.h
        globals.h
        entry.h
        src/core/environment/environment.cpp
        src/core/environment/environment.h
        src/core/environment/libs/http.cpp
        src/core/environment/libs/misc.cpp
        src/core/environment/libs/closure.cpp
        src/core/render/render.cpp
        src/core/render/render.h
        src/core/render/user_interface/user_interface.cpp
        src/core/render/user_interface/user_interface.h
        src/core/environment/libs/metatable.cpp
        src/core/environment/libs/filesystem.cpp
        src/core/communication/communication.cpp
        src/core/communication/communication.h
        dependencies/nlohmann/json.h
        dependencies/httpstatus/httpstatus.h)

# DEPENDENCIES
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")

## LUAU
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luau")
target_link_libraries(${PROJECT_NAME} PUBLIC Luau.Compiler Luau.Ast Luau.VM Luau.VM.Internals)

## IMGUI
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/cpr")
target_link_libraries(${PROJECT_NAME} PUBLIC cpr::cpr)

## IMGUI
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui")
target_link_libraries(${PROJECT_NAME} PUBLIC Imgui)