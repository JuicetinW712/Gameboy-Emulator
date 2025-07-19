#include "mbc.hpp"
#include <fstream>

MBC1Battery::MBC1Battery(vector<uint8_t>& romData, vector<uint8_t>& ramData, string filename) :
    MBC1(romData, ramData), filename(filename) {
    // Load RAM from file if it exists
    ifstream file(filename, ios::binary | ios::ate);
    if (file.is_open()) {
        file.seekg(0, ios::beg);
        file.read(reinterpret_cast<char*>(ram.data()), ram.size());
        file.close();
    }
}

MBC1Battery::~MBC1Battery() {
    // Save RAM to file
    ofstream file(filename, ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<char*>(ram.data()), ram.size());
        file.close();
    }
}

MBC2Battery::MBC2Battery(vector<uint8_t>& romData, vector<uint8_t>& ramData, string filename) :
    MBC2(romData, ramData), filename(filename) {
    // Load RAM from file if it exists
    ifstream file(filename, ios::binary | ios::ate);
    if (file.is_open()) {
        file.seekg(0, ios::beg);
        file.read(reinterpret_cast<char*>(ram.data()), ram.size());
        file.close();
    }
}

MBC2Battery::~MBC2Battery() {
    // Save RAM to file
    ofstream file(filename, ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<char*>(ram.data()), ram.size());
        file.close();
    }
}