#pragma once

#include <cstring>

#include "cpu.hpp"
#include "ppu.hpp"
#include "cartridge.hpp"

class Gameboy {
private:
    MMU mmu;
    CPU cpu;
    PPU ppu;

public:
    Gameboy(Cartridge& cartridge):
        mmu(cartridge),
        cpu(mmu),
        ppu(mmu) {}

    void run() {
        while (true) {
            int cycles = cpu.cycle();
            ppu.tick(cycles);
            mmu.tick(cycles);
        }
    }
};