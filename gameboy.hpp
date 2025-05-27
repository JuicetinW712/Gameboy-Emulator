#pragma once

#include <cstring>

#include "cpu.hpp"
#include "cartridge.hpp"

class Gameboy {
private:
    MMU mmu;
    CPU cpu;

public:
    Gameboy(Cartridge& cartridge):
        mmu(cartridge),
        cpu(mmu) {}

    void run() {
        cpu.run();
    }
};