#pragma once

#include <array>
#include <cstdint>


class IO {
private:
    static constexpr uint16_t IO_START = 0xFF00;
    static constexpr uint16_t IO_END = 0xFF7F;
    static constexpr uint16_t IO_SIZE = IO_END - IO_START + 1;

    static constexpr uint16_t DIV_ADDRESS = 0xFF04;
    static constexpr uint16_t TIMA_ADDRESS = 0xFF05;
    static constexpr uint16_t TMA_ADDRESS = 0xFF06;
    static constexpr uint16_t TAC_ADDRESS = 0xFF07;

    std::array<uint8_t, 0x80> io{};
    int divCounter{0};
    int timaCounter{0};
    uint8_t directionButtons{0xFF}; // All unpressed
    uint8_t actionButtons{0xFF};    // All unpressed

public:
    IO() : divCounter(0), timaCounter(0) {}

    uint8_t read(uint16_t address);

    void write(uint16_t address, uint8_t val);

    void tick(int cycles);
    void requestInterrupt(uint8_t interrupt);

    void handleKeyDown(uint8_t key);
    void handleKeyUp(uint8_t key);
};

