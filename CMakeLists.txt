cmake_minimum_required(VERSION 3.22.1)
project(gameboy)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(gameboy
        cpu.hpp
        mbc.hpp
        mmu.hpp
        main.cpp
        register_types.hpp
        cpu.cpp
        gameboy.hpp
        cartridge.hpp
        io.hpp
        ppu.hpp)
