#pragma once

#include <array>
#include <cstdint>


class IO {
private:
    static constexpr uint16_t IO_START = 0xFF00;
    static constexpr uint16_t IO_END = 0xFF7F;
    static constexpr uint16_t IO_SIZE = IO_START - IO_END + 1;

    std::array<uint8_t, IO_SIZE> io{};
public:
    IO() {}

    uint8_t read(uint16_t address) {
        return io.at(address - IO_START);
    }

    void write(uint16_t address, uint8_t val) {
        io.at(address - IO_START) = val;
    }
};

