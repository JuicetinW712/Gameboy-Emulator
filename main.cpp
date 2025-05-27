//
// Created by pleas on 5/20/2025.
//
// #include <stdio.h>
// #include <stdlib.h>
// #include <cpu.hpp>
#include <cartridge.hpp>
#include <vector>
#include <iostream>
#include <fstream>

#include "gameboy.hpp"

using namespace std;

vector<char> readFile(string fileName)
{
    ifstream file(fileName, ios::binary | ios::ate);

    if (!file.is_open())
    {
        throw runtime_error("Failed to open file.");
    }

    streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    vector<char> buffer(size);

    if (!file.read(buffer.data(), size))
    {
        throw runtime_error("Failed to read file.");
    }

    return buffer;
}


// TODO - move main loop into chip8 class
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cout << "Invalid Input. Should be ./gameboy {filename}\n";
        return 0;
    }

    string fileName = argv[1];
    vector<char> buffer = readFile(fileName);
    vector<uint8_t> uintBuffer(buffer.begin(), buffer.end());

    Cartridge cartridge(std::move(uintBuffer));
    cartridge.printInfo();

    Gameboy emu(cartridge);
    emu.run();
};
