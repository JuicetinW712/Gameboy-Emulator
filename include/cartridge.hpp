#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <memory>

#include "mbc.hpp"

constexpr size_t ROM_BANK_SIZE = 32768;
constexpr size_t KILOBYTE_SIZE = 1024;

enum class MBCType {
    ROM_ONLY,
    MBC1,
    MBC1_WITH_RAM,
    MBC2,
    UNSUPPORTED
};

class Cartridge {
private:
    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;
    std::string title;
    size_t romSize;
    size_t ramSize;
    std::string filename; // Add filename as a member variable

    MBCType mbcType;
    unique_ptr<MBC> mbc;

    MBCType getMBCType(uint8_t code);
    unique_ptr<MBC> getMBC(MBCType mbcType);
    size_t getRamSize(uint8_t code);
    std::string getMBCString(MBCType mbcType);

public:
    explicit Cartridge(std::vector<uint8_t>&& romData, const std::string& filename) :
        filename(filename) {
        // Extract info from cartridge
        rom = std::move(romData);
        title.assign(rom.begin() + 0x0134, rom.begin() + 0x0143);
        romSize = rom.size();
        ramSize = getRamSize(rom.at(0x0149));
        ram.resize(ramSize, 0);

        // Initialize MBC
        mbcType = getMBCType(rom.at(0x0147));
        mbc = getMBC(mbcType);
    }

    Cartridge(Cartridge&& other) noexcept :
        rom(std::move(other.rom)),
        ram(std::move(other.ram)),
        title(std::move(other.title)),
        romSize(other.romSize),
        ramSize(other.ramSize),
        filename(std::move(other.filename)),
        mbcType(other.mbcType) {
        mbc = getMBC(other.mbcType);
    }

    Cartridge& operator=(Cartridge&& other) noexcept {
        if (this != &other) {
            rom = std::move(other.rom);
            ram = std::move(other.ram);
            title = std::move(other.title);
            romSize = other.romSize;
            ramSize = other.ramSize;
            filename = std::move(other.filename);
            mbcType = other.mbcType;
            mbc = getMBC(other.mbcType);
        }
        return *this;
    }

    void printInfo() {
        std::cout << "Title: " << title << std::endl;
        std::cout << "MBC Type: " << getMBCString(mbcType) << std::endl;
        std::cout << "ROM Size: " << romSize << " Bytes" << std::endl;
        std::cout << "RAM Size: " << ramSize << " Bytes" << std::endl;
    }

    void write(uint16_t address, uint8_t value) {
        mbc->write(address, value);
    }

    uint8_t read(uint16_t address) {
        return mbc->read(address);
    }
};
