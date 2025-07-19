#include "gameboy.hpp"

Gameboy::Gameboy(Cartridge& cartridge) : cartridge(cartridge), mmu(cartridge), cpu(mmu) {}

void Gameboy::run() {
    // Main emulation loop
    while (true) {
        cpu.cycle();
    }
}