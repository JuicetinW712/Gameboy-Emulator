#pragma once
#include <cstdint>
#include <vector>

using namespace std;

#define MAX(a, b) (((a) >= (b)) ? (a) : (b))

class MBC {
public:
    virtual ~MBC() = default;

    virtual void write(uint16_t address, uint8_t value) = 0;
    [[nodiscard]] virtual uint8_t read(uint16_t address) const = 0;
};

class MBC1: public MBC {
private:
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