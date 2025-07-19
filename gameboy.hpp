#pragma once

#include "cpu.hpp"
#include "mmu.hpp"
#include "cartridge.hpp"
#include "ppu.hpp"

class Gameboy {
private:
    Cartridge& cartridge;
    MMU mmu;
    CPU cpu;
    PPU ppu;

public:
    explicit Gameboy(Cartridge& cartridge);
    void run();
};