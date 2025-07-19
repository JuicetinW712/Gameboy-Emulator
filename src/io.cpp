#include "io.hpp"

uint8_t IO::read(uint16_t address) {
    switch (address) {
        case IO::DIV_ADDRESS:
            return io.at(IO::DIV_ADDRESS - IO::IO_START);
        case 0xFF00: // Joypad register
            {
                uint8_t joypad_state = io.at(address - IO::IO_START);
                uint8_t result = joypad_state; // Preserve selection bits

                // If direction keys are selected (bit 4 is 0)
                if (!((joypad_state >> 4) & 1)) {
                    result = (result & 0xF0) | (directionButtons & 0x0F);
                }
                // If action keys are selected (bit 5 is 0)
                if (!((joypad_state >> 5) & 1)) {
                    result = (result & 0xF0) | (actionButtons & 0x0F);
                }
                return result;
            }
        default:
            return io.at(address - IO::IO_START);
    }
}

void IO::write(uint16_t address, uint8_t val) {
    switch (address) {
        case IO::DIV_ADDRESS:
            io.at(IO::DIV_ADDRESS - IO::IO_START) = 0;
            break;
        case 0xFF00: // Joypad register
            // Only bits 4 and 5 are writable (selection bits)
            io.at(address - IO::IO_START) = (io.at(address - IO::IO_START) & 0x0F) | (val & 0xF0);
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

void IO::handleKeyDown(uint8_t key) {
    switch (key) {
        case 0: // Right
            directionButtons &= ~(1 << 0);
            break;
        case 1: // Left
            directionButtons &= ~(1 << 1);
            break;
        case 2: // Up
            directionButtons &= ~(1 << 2);
            break;
        case 3: // Down
            directionButtons &= ~(1 << 3);
            break;
        case 4: // A
            actionButtons &= ~(1 << 0);
            break;
        case 5: // B
            actionButtons &= ~(1 << 1);
            break;
        case 6: // Select
            actionButtons &= ~(1 << 2);
            break;
        case 7: // Start
            actionButtons &= ~(1 << 3);
            break;
    }
    requestInterrupt(0x10); // Request joypad interrupt
}

void IO::handleKeyUp(uint8_t key) {
    switch (key) {
        case 0: // Right
            directionButtons |= (1 << 0);
            break;
        case 1: // Left
            directionButtons |= (1 << 1);
            break;
        case 2: // Up
            directionButtons |= (1 << 2);
            break;
        case 3: // Down
            directionButtons |= (1 << 3);
            break;
        case 4: // A
            actionButtons |= (1 << 0);
            break;
        case 5: // B
            actionButtons |= (1 << 1);
            break;
        case 6: // Select
            actionButtons |= (1 << 2);
            break;
        case 7: // Start
            actionButtons |= (1 << 3);
            break;
    }
}
