cmake_minimum_required(VERSION 3.22.1)
project(gameboy)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED COMPONENTS SDL2)
include_directories(include ${SDL2_INCLUDE_DIRS})

add_executable(gameboy
        src/main.cpp
        src/cpu.cpp
        src/io.cpp
        src/ppu.cpp
        src/cartridge.cpp
        src/mbc.cpp
        src/mmu.cpp
        gameboy.cpp)

target_link_libraries(gameboy ${SDL2_LIBRARIES})
