#pragma once

#include "cpu.hpp"
#include "mmu.hpp"
#include "cartridge.hpp"

class Gameboy {
private:
    Cartridge& cartridge;
    MMU mmu;
    CPU cpu;

public:
    explicit Gameboy(Cartridge& cartridge);
    void run();
};