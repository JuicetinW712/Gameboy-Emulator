#pragma once
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "cartridge.hpp"
#include "io.hpp"

enum class MemoryRegion : uint8_t {
    ROM_0,
    ROM_N,
    VRAM,
    ERAM,
    WRAM,
    OAM,
    IO,
    HRAM,
    INTERRUPT_REGISTER,
    UNKNOWN // For addresses that don't fall into defined regions
};

namespace MemoryMap {
    constexpr uint16_t ROM_0_START        = 0x0000;
    constexpr uint16_t ROM_0_END          = 0x3FFF;
    constexpr uint16_t ROM_N_START        = 0x4000;
    constexpr uint16_t ROM_N_END          = 0x7FFF;
    constexpr uint16_t VRAM_START         = 0x8000;
    constexpr uint16_t VRAM_END           = 0x9FFF;
    constexpr uint16_t ERAM_START         = 0xA000;
    constexpr uint16_t ERAM_END           = 0xBFFF;
    constexpr uint16_t WRAM_START         = 0xC000;
    constexpr uint16_t WRAM_END           = 0xDFFF;
    constexpr uint16_t OAM_START          = 0xFE00;
    constexpr uint16_t OAM_END            = 0xFE9F;
    constexpr uint16_t IO_START           = 0xFF00;
    constexpr uint16_t IO_END             = 0xFF7F;
    constexpr uint16_t HRAM_START         = 0xFF80;
    constexpr uint16_t HRAM_END           = 0xFFFE;
    constexpr uint16_t INTERRUPT_REGISTER = 0xFFFF;
} // namespace MemoryMap

class MMU {
private:
    Cartridge& cartridge;
    std::array<uint8_t, 8192> vram{}; // Using std::array and initializing with {}
    std::array<uint8_t, 8192> wram{};
    std::array<uint8_t, 0xA0> oam{}; // Size should be 0xA1 (FE9F - FE00)
    std::array<uint8_t, 0x7F> hram{}; // Size should be 0x81 (FFFE - FF80)
    IO io;
    uint8_t interruptRegister{0};

    static constexpr bool inRange(uint16_t address, uint16_t start, uint16_t end) {
        return address >= start && address <= end;
    }

    // Helper function to determine the memory region
    [[nodiscard]] static MemoryRegion getMemoryRegion(uint16_t address) {
        if (inRange(address, MemoryMap::ROM_0_START, MemoryMap::ROM_0_END)) {
            return MemoryRegion::ROM_0;
        } else if (inRange(address, MemoryMap::ROM_N_START, MemoryMap::ROM_N_END)) {
            return MemoryRegion::ROM_N;
        } else if (inRange(address, MemoryMap::VRAM_START, MemoryMap::VRAM_END)) {
            return MemoryRegion::VRAM;
        } else if (inRange(address, MemoryMap::ERAM_START, MemoryMap::ERAM_END)) {
            return MemoryRegion::ERAM;
        } else if (inRange(address, MemoryMap::WRAM_START, MemoryMap::WRAM_END)) {
            return MemoryRegion::WRAM;
        } else if (inRange(address, MemoryMap::OAM_START, MemoryMap::OAM_END)) {
            return MemoryRegion::OAM;
        } else if (inRange(address, MemoryMap::IO_START, MemoryMap::IO_END)) {
            return MemoryRegion::IO;
        } else if (inRange(address, MemoryMap::HRAM_START, MemoryMap::HRAM_END)) {
            return MemoryRegion::HRAM;
        } else if (address == MemoryMap::INTERRUPT_REGISTER) {
            return MemoryRegion::INTERRUPT_REGISTER;
        } else {
            return MemoryRegion::UNKNOWN;
        }
    }

public:
    // Constructor uses initializer list for mbc, consistent with good practice
    explicit MMU(Cartridge& cartridge);

    void tick(int cycles);

    void requestInterrupt(uint8_t interrupt);

    uint8_t read(uint16_t address);

    void write(uint16_t address, uint8_t value);
};