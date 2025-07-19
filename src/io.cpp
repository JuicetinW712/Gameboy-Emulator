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

void IO::handleKeyDown(uint8_t key) {
    uint8_t joypad_state = io.at(0xFF00 - IO::IO_START);
    switch (key) {
        case 0: // Right
            if (!((joypad_state >> 4) & 1)) { // If direction keys are selected
                joypad_state &= ~(1 << 0);
            }
            break;
        case 1: // Left
            if (!((joypad_state >> 4) & 1)) {
                joypad_state &= ~(1 << 1);
            }
            break;
        case 2: // Up
            if (!((joypad_state >> 4) & 1)) {
                joypad_state &= ~(1 << 2);
            }
            break;
        case 3: // Down
            if (!((joypad_state >> 4) & 1)) {
                joypad_state &= ~(1 << 3);
            }
            break;
        case 4: // A
            if (!((joypad_state >> 5) & 1)) { // If action keys are selected
                joypad_state &= ~(1 << 0);
            }
            break;
        case 5: // B
            if (!((joypad_state >> 5) & 1)) {
                joypad_state &= ~(1 << 1);
            }
            break;
        case 6: // Select
            if (!((joypad_state >> 5) & 1)) {
                joypad_state &= ~(1 << 2);
            }
            break;
        case 7: // Start
            if (!((joypad_state >> 5) & 1)) {
                joypad_state &= ~(1 << 3);
            }
            break;
    }
    io.at(0xFF00 - IO::IO_START) = joypad_state;
    requestInterrupt(0x10); // Request joypad interrupt
}

void IO::handleKeyUp(uint8_t key) {
    uint8_t joypad_state = io.at(0xFF00 - IO::IO_START);
    switch (key) {
        case 0: // Right
            if (!((joypad_state >> 4) & 1)) {
                joypad_state |= (1 << 0);
            }
            break;
        case 1: // Left
            if (!((joypad_state >> 4) & 1)) {
                joypad_state |= (1 << 1);
            }
            break;
        case 2: // Up
            if (!((joypad_state >> 4) & 1)) {
                joypad_state |= (1 << 2);
            }
            break;
        case 3: // Down
            if (!((joypad_state >> 4) & 1)) {
                joypad_state |= (1 << 3);
            }
            break;
        case 4: // A
            if (!((joypad_state >> 5) & 1)) {
                joypad_state |= (1 << 0);
            }
            break;
        case 5: // B
            if (!((joypad_state >> 5) & 1)) {
                joypad_state |= (1 << 1);
            }
            break;
        case 6: // Select
            if (!((joypad_state >> 5) & 1)) {
                joypad_state |= (1 << 2);
            }
            break;
        case 7: // Start
            if (!((joypad_state >> 5) & 1)) {
                joypad_state |= (1 << 3);
            }
            break;
    }
    io.at(0xFF00 - IO::IO_START) = joypad_state;
}
