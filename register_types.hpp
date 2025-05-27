#pragma once

enum class R8 {
    A, F, B, C, D, E, H, L, HL_ADDR
};

enum class R16 {
    BC, DE, HL, SP
};

enum class R16STK {
    BC, DE, HL, AF
};

enum class R16MEM {
    BC, DE, HLI, HLD
};

enum class COND {
    NZ, Z, NC, C
};
