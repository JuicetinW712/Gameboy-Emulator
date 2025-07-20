#include "mmu.hpp"
#include <iostream>
#include <stdexcept>

MMU::MMU(Cartridge& cartridge) : cartridge(cartridge), io(IO()) {}

void MMU::tick(int cycles) {
    io.tick(cycles);
}

void MMU::requestInterrupt(uint8_t interrupt) {
    io.requestInterrupt(interrupt);
}

uint8_t MMU::read(uint16_t address) {
    switch (getMemoryRegion(address)) {
        case MemoryRegion::ROM_0:
        case MemoryRegion::ROM_N:
        case MemoryRegion::ERAM: // ERAM reads also go through MBC
            return cartridge.read(address);
        case MemoryRegion::VRAM:
            return vram.at(address - MemoryMap::VRAM_START);
        case MemoryRegion::WRAM:
            return wram.at(address - MemoryMap::WRAM_START);
        case MemoryRegion::OAM:
            return oam.at(address - MemoryMap::OAM_START);
        case MemoryRegion::IO:
            // std::cout << "Attempted read from IO address: 0x" << std::hex << address << std::endl;
            // TODO handle IO
            return io.read(address);
            // return 0xFF;
        case MemoryRegion::HRAM:
            return hram.at(address - MemoryMap::HRAM_START);
        case MemoryRegion::INTERRUPT_REGISTER:
            return interruptRegister;
        case MemoryRegion::UNUSABLE:
            return 0xFF; // Reads from unusable memory return 0xFF
        case MemoryRegion::UNKNOWN:
            std::cerr << "Attempted read from unknown address: 0x" << std::hex << address << std::endl;
            throw std::out_of_range("Attempted read from unknown memory region");
        default:
            std::cerr << "Code should be unreachable - invalid enum state" << std::endl;
            throw std::out_of_range("Attempted read from unknown memory region");
    }
}

void MMU::write(uint16_t address, uint8_t value) {
    switch (getMemoryRegion(address)) {
        case MemoryRegion::ROM_0:
        case MemoryRegion::ROM_N:
        case MemoryRegion::ERAM: // ERAM writes also go through MBC
            cartridge.write(address, value);
            break;
        case MemoryRegion::VRAM:
            vram.at(address - MemoryMap::VRAM_START) = value;
            break;
        case MemoryRegion::WRAM:
            wram.at(address - MemoryMap::WRAM_START) = value;
            break;
        case MemoryRegion::OAM:
            oam.at(address - MemoryMap::OAM_START) = value;
            break;
        case MemoryRegion::IO:
            
            io.write(address, value);
            // std::cout << "Attempted write to IO address: 0x" << std::hex << address << " with value: 0x" << std::hex << (int)value << std::endl;
            // TODO Implement actual IO register writes here
            break;
        case MemoryRegion::HRAM:
            hram.at(address - MemoryMap::HRAM_START) = value;
            break;
        case MemoryRegion::INTERRUPT_REGISTER:
            interruptRegister = value;
            break;
        case MemoryRegion::UNUSABLE:
            // Writes to unusable memory are ignored
            break;
        case MemoryRegion::UNKNOWN:
            std::cerr << "Attempted write to unknown address: 0x" << std::hex << address << " with value: 0x" << std::hex << (int)value << std::endl;
            throw std::out_of_range("Attempted write to unknown memory region");
    }
}

void MMU::handleKeyDown(uint8_t key) {
    io.handleKeyDown(key);
}

void MMU::handleKeyUp(uint8_t key) {
    io.handleKeyUp(key);
}
