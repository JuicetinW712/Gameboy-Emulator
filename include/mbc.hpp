#pragma once
#include <cassert>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>

using namespace std;

template <typename T>
constexpr const T& MAX(const T& a, const T& b) {
    return (a > b) ? a : b;
}

class MBC {
public:
    virtual ~MBC() = default;

    virtual void write(uint16_t address, uint8_t value) = 0;
    [[nodiscard]] virtual uint8_t read(uint16_t address) const = 0;
};

class ROMOnly: public MBC {
protected:
    vector<uint8_t>& rom;

public:
    ROMOnly(vector<uint8_t>& romData) : rom(romData) {}

    void write(uint16_t address, uint8_t value) override {
        // ROM Only cartridges do not have MBC, so writes to ROM are ignored.
    }

    [[nodiscard]] uint8_t read(uint16_t address) const override {
        return rom.at(address);
    }
};

class MBC1: public MBC {
protected:
    // Bank sizes
    static constexpr uint16_t ROM_BANK_SIZE = 0x4000;
    static constexpr uint16_t RAM_BANK_SIZE = 0x2000;

    // Addresses that modify the MBC state
    static constexpr uint16_t RAM_ENABLE_START   = 0x0000;
    static constexpr uint16_t RAM_ENABLE_END     = 0x1FFF;
    static constexpr uint16_t ROM_LOWER_START    = 0x2000;
    static constexpr uint16_t ROM_LOWER_END      = 0x3FFF;
    static constexpr uint16_t ROM_UPPER_START    = 0x4000;
    static constexpr uint16_t ROM_UPPER_END      = 0x5FFF;
    static constexpr uint16_t MODE_SELECT_START  = 0x6000;
    static constexpr uint16_t MODE_SELECT_END    = 0x7FFF;

    // ROM and RAM addresses
    static constexpr uint16_t ROM_BANK_0_START   = 0x0000;
    static constexpr uint16_t ROM_BANK_0_END     = 0x3FFF;
    static constexpr uint16_t ROM_BANK_N_START   = 0x4000;
    static constexpr uint16_t ROM_BANK_N_END     = 0x7FFF;
    static constexpr uint16_t RAM_BANK_START     = 0xA000;
    static constexpr uint16_t RAM_BANK_END       = 0xBFFF;

    // Cartridge data
    vector<uint8_t>& rom;
    vector<uint8_t>& ram;
    const uint8_t numRomBanks;
    const uint8_t numRamBanks;

    // Mode and bank state
    bool ramEnabled{false};
    bool ramBankingMode{false};
    uint8_t romBankNumber{1};
    uint8_t ramBankNumber{0};

    static constexpr bool inRange(uint16_t address, uint16_t start, uint16_t end) {
        return address >= start && address <= end;
    }

public:
    MBC1() = delete;
    MBC1(const MBC1&) = delete;
    MBC1& operator=(const MBC1&) = delete;
    MBC1(MBC1&&) = delete;
    virtual ~MBC1() = default;

    MBC1(vector<uint8_t>& romData, vector<uint8_t>& ramData):
        rom(romData),
        ram(ramData),
        numRomBanks(static_cast<uint8_t>(romData.size() / ROM_BANK_SIZE)),
        numRamBanks(static_cast<uint8_t>(ramData.size() / RAM_BANK_SIZE)) {
    }

    void write(uint16_t address, uint8_t value) override
    {
        if (inRange(address, RAM_ENABLE_START, RAM_ENABLE_END)) 
        {
            // Enable ram if 0xA is written to 0x0000 - 0x1FFF 
            ramEnabled = ((value & 0xF) == 0xA);
        } 
        else if (inRange(address, ROM_LOWER_START, ROM_LOWER_END))
        {
            // Set lower bits for rom bank
            romBankNumber = (romBankNumber & 0x60) | MAX(value & 0x1F, 1);
        } 
        else if (inRange(address, ROM_UPPER_START, ROM_UPPER_END))
        {
            // Set upper bits of rom bank or set ram bank (depends on mode)
            if (ramBankingMode) {
                ramBankNumber = (value & 0x3);
            } else if (numRomBanks >= 0x40) {
                uint8_t lowerRomBankBits = (romBankNumber & 0x1F);
                uint8_t newUpperRomBankBits = (value & 0x3);
                romBankNumber = (newUpperRomBankBits << 5) | lowerRomBankBits;
            }
        } 
        else if (inRange(address, MODE_SELECT_START, MODE_SELECT_END))
        {
            // Set ram banking mode
            ramBankingMode = static_cast<bool>(value & 0x1);
        } 
        else if (ramEnabled && inRange(address, RAM_BANK_START, RAM_BANK_END)) 
        {
            uint16_t bankOffset = (ramBankingMode ? ramBankNumber : 0) * RAM_BANK_SIZE;
            ram.at((address - RAM_BANK_START) + bankOffset) = value;
        }
        else
        {
            assert(false && "MBC should not be handling address for writes");
        }
    }

    [[nodiscard]] uint8_t read(uint16_t address) const override
    {
        if (inRange(address, ROM_BANK_0_START, ROM_BANK_0_END)) 
        {
            return rom.at(address);
        } 
        else if (inRange(address, ROM_BANK_N_START, ROM_BANK_N_END))
        {
            // Need to mask out the top 2 bits if ramBanking mode is on
            uint16_t bankIndex = ramBankingMode ? (romBankNumber & 0x1F): (romBankNumber);
            return rom.at(address + (ROM_BANK_SIZE * (bankIndex - 1)));
        } 
        else if (ramEnabled && inRange(address, RAM_BANK_START, RAM_BANK_END)) 
        {
            // No offset if ram banking mode is not turned on
            uint16_t bankOffset = ramBankingMode ? (RAM_BANK_SIZE * ramBankNumber) : 0;
            return ram.at((address - RAM_BANK_START) + bankOffset);
        } 
        else 
        {
            return 0xFF;
        }
    }
};

class MBC2: public MBC {
protected:
    // Bank sizes
    static constexpr uint16_t ROM_BANK_SIZE = 0x4000;
    static constexpr uint16_t RAM_SIZE = 0x200;

    // Addresses that modify the MBC state
    static constexpr uint16_t RAM_ENABLE_START = 0x0000;
    static constexpr uint16_t RAM_ENABLE_END = 0x1FFF;
    static constexpr uint16_t ROM_BANK_SELECT_START = 0x2000;
    static constexpr uint16_t ROM_BANK_SELECT_END = 0x3FFF;

    // ROM and RAM addresses
    static constexpr uint16_t ROM_BANK_0_START = 0x0000;
    static constexpr uint16_t ROM_BANK_0_END = 0x3FFF;
    static constexpr uint16_t ROM_BANK_N_START = 0x4000;
    static constexpr uint16_t ROM_BANK_N_END = 0x7FFF;
    static constexpr uint16_t RAM_BANK_START = 0xA000;
    static constexpr uint16_t RAM_BANK_END = 0xA1FF;

    // Cartridge data
    vector<uint8_t>& rom;
    vector<uint8_t>& ram;
    const uint8_t numRomBanks;

    // Mode and bank state
    bool ramEnabled{false};
    uint8_t romBankNumber{1};

    static constexpr bool inRange(uint16_t address, uint16_t start, uint16_t end) {
        return address >= start && address <= end;
    }

public:
    MBC2() = delete;
    MBC2(const MBC2&) = delete;
    MBC2& operator=(const MBC2&) = delete;
    MBC2(MBC2&&) = delete;
    virtual ~MBC2() = default;

    MBC2(vector<uint8_t>& romData, vector<uint8_t>& ramData) :
        rom(romData),
        ram(ramData),
        numRomBanks(static_cast<uint8_t>(romData.size() / ROM_BANK_SIZE)) {
    }

    void write(uint16_t address, uint8_t value) override {
        if (inRange(address, RAM_ENABLE_START, RAM_ENABLE_END)) {
            if (!((address >> 8) & 0x1)) { // Check if bit 8 is 0
                ramEnabled = ((value & 0xF) == 0xA);
            }
        } else if (inRange(address, ROM_BANK_SELECT_START, ROM_BANK_SELECT_END)) {
            if (((address >> 8) & 0x1)) { // Check if bit 8 is 1
                romBankNumber = MAX(value & 0xF, 1);
            }
        } else if (ramEnabled && inRange(address, RAM_BANK_START, RAM_BANK_END)) {
            ram.at(address - RAM_BANK_START) = value & 0xF; // MBC2 RAM is 4 bits wide
        } else {
            assert(false && "MBC2 should not be handling address for writes");
        }
    }

    [[nodiscard]] uint8_t read(uint16_t address) const override {
        if (inRange(address, ROM_BANK_0_START, ROM_BANK_0_END)) {
            return rom.at(address);
        } else if (inRange(address, ROM_BANK_N_START, ROM_BANK_N_END)) {
            return rom.at(address + (ROM_BANK_SIZE * (romBankNumber - 1)));
        } else if (ramEnabled && inRange(address, RAM_BANK_START, RAM_BANK_END)) {
            return ram.at(address - RAM_BANK_START) | 0xF0; // Upper 4 bits are always 1
        } else {
            return 0xFF;
        }
    }
};