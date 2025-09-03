// Empty file for now, will implement later
#include <cartridge.hpp>

MBCType Cartridge::getMBCType(uint8_t code) {
    switch (code) {
        case 0x0: return MBCType::ROM_ONLY;
        case 0x1: return MBCType::MBC1;
        case 0x2: return MBCType::MBC1_WITH_RAM;
        case 0x5: return MBCType::MBC2;
        default: return MBCType::UNSUPPORTED;
    }
}

unique_ptr<MBC> Cartridge::getMBC(MBCType mbcType) {
    switch (mbcType) {
        case MBCType::ROM_ONLY: return make_unique<ROMOnly>(rom);
        case MBCType::MBC1: return make_unique<MBC1>(rom, ram);
        case MBCType::MBC1_WITH_RAM: return make_unique<MBC1>(rom, ram);
        case MBCType::MBC2: return make_unique<MBC2>(rom, ram);
        default: return make_unique<ROMOnly>(rom); // Fallback for unsupported MBCs
    }
}

size_t Cartridge::getRamSize(uint8_t code) {
    switch (code) {
        case 0x0: return 0;
        case 0x2: return KILOBYTE_SIZE;
        case 0x3: return 4 * KILOBYTE_SIZE;
        case 0x4: return 16 * KILOBYTE_SIZE;
        case 0x5: return 8 * KILOBYTE_SIZE;
        case 0x6: return 512; // MBC2 has 512 Bytes of RAM
        default: assert(false && "Unknown RAM Code");
    }
}

std::string Cartridge::getMBCString(MBCType mbcType) {
    switch (mbcType) {
        case MBCType::ROM_ONLY: return "ROM ONLY";
        case MBCType::MBC1: return "MBC1";
        case MBCType::MBC1_WITH_RAM: return "MBC1_WITH_RAM";
        case MBCType::MBC2: return "MBC2";
        default: return "UNSUPPORTED";
    }
}

