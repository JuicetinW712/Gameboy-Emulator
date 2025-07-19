#include "io.hpp"

uint8_t IO::read(uint16_t address) {
    switch (address) {
        case IO::DIV_ADDRESS:
            return io.at(IO::DIV_ADDRESS - IO::IO_START);
        default:
            return io.at(address - IO::IO_START);
    }
}

void IO::write(uint16_t address, uint8_t val) {
    switch (address) {
        case IO::DIV_ADDRESS:
            io.at(IO::DIV_ADDRESS - IO::IO_START) = 0;
            break;
        default:
            io.at(address - IO::IO_START) = val;
            break;
    }
}

void IO::tick(int cycles) {
    IO::divCounter += cycles;
    if (IO::divCounter >= 256) {
        IO::divCounter -= 256;
        io.at(IO::DIV_ADDRESS - IO::IO_START)++;
    }

    if (io.at(IO::TAC_ADDRESS - IO::IO_START) & 0x4) {
        IO::timaCounter += cycles;
        int threshold;
        switch (io.at(IO::TAC_ADDRESS - IO::IO_START) & 0x3) {
            case 0: threshold = 1024; break;
            case 1: threshold = 16; break;
            case 2: threshold = 64; break;
            case 3: threshold = 256; break;
        }

        if (IO::timaCounter >= threshold) {
            IO::timaCounter -= threshold;
            if (io.at(IO::TIMA_ADDRESS - IO::IO_START) == 0xFF) {
                io.at(IO::TIMA_ADDRESS - IO::IO_START) = io.at(IO::TMA_ADDRESS - IO::IO_START);
                requestInterrupt(0x04); // Timer interrupt
            } else {
                io.at(IO::TIMA_ADDRESS - IO::IO_START)++;
            }
        }
    }
}

void IO::requestInterrupt(uint8_t interrupt) {
    io.at(0xFF0F - IO::IO_START) |= interrupt;
}
