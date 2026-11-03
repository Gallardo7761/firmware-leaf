cmake_minimum_required(VERSION 3.22)

# default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# compile_commands.json for clangd/CLion
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(firmware-leaf C ASM)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# executable binary
add_executable(${PROJECT_NAME}
        Core/Src/power.c)

# core sources
target_sources(${PROJECT_NAME} PRIVATE
        Core/Src/tasks.c
        Core/Src/power.c
)

add_subdirectory(cmake/stm32cubemx)
add_subdirectory(Drivers/SHT40)

target_link_libraries(${PROJECT_NAME}
        stm32cubemx
        sht40
)