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

unique_ptr<MBC> Cartridge::getMBC(MBCType mbcType, const std::string& filename) {
    switch (mbcType) {
        case MBCType::ROM_ONLY: return make_unique<ROMOnly>(rom);
        case MBCType::MBC1: return make_unique<MBC1>(rom, ram);
        case MBCType::MBC1_WITH_RAM: return make_unique<MBC1>(rom, ram);
        case MBCType::MBC2: return make_unique<MBC2>(rom, ram);
        default: return make_unique<ROMOnly>(rom); // Fallback for unsupported MBCs
    }
}



