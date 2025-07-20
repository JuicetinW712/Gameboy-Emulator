//
// Created by pleas on 5/20/2025.
//

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
    bool isTestMode = false;
    std::string fileName;

    if (argc < 2 || argc > 3) {
        std::cout << "Invalid Input. Usage: ./gameboy {filename} [--test]\n";
        return 0;
    }

    fileName = argv[1];

    if (argc == 3 && std::string(argv[2]) == "--test") {
        isTestMode = true;
    } else if (argc == 3) {
        std::cout << "Invalid flag. Usage: ./gameboy {filename} [--test]\n";
        return 0;
    }

    std::vector<char> buffer = readFile(fileName);
    std::vector<uint8_t> uintBuffer(buffer.begin(), buffer.end());

    Cartridge cartridge(std::move(uintBuffer), fileName);
    if (isTestMode) {
        cartridge.printInfo();
    }

    Gameboy emu(cartridge);
    emu.run();
};
