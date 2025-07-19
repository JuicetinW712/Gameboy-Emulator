#include "gameboy.hpp"

Gameboy::Gameboy(Cartridge& cartridge) : cartridge(cartridge), mmu(cartridge), cpu(mmu), ppu(mmu) {}

void Gameboy::run() {
    // Main emulation loop
    while (true) {
        int cycles = cpu.cycle();
        ppu.tick(cycles);
    }
}