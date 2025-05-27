#include "cpu.hpp"
#include <cassert>

// Getters & Setters

// r8
R8 CPU::getR8(uint8_t registerNum) {
    assert(registerNum < 8);
    switch (registerNum) {
        case 0: return R8::B;
        case 1: return R8::C;
        case 2: return R8::D;
        case 3: return R8::E;
        case 4: return R8::H;
        case 5: return R8::L;
        case 6: return R8::HL_ADDR;
        case 7: return R8::A;
        default:
            assert(false && "Invalid R8 index");
            return R8::A;
    }
}
uint8_t CPU::getR8Value(R8 r8) {
    switch (r8) {
        case R8::B: return registers.B;
        case R8::C: return registers.C;
        case R8::D: return registers.D;
        case R8::E: return registers.E;
        case R8::H: return registers.H;
        case R8::L: return registers.L;
        case R8::HL_ADDR: return bus.read(registers.H << 8 | registers.L);
        case R8::A: return registers.A;
        case R8::F: return registers.F;
        default: assert(false && "Invalid R8 index ");
    }
}
void CPU::setR8Value(R8 r8, uint8_t value) {
    switch (r8) {
        case R8::B: registers.B = value; break;
        case R8::C: registers.C = value; break;
        case R8::D: registers.D = value; break;
        case R8::E: registers.E = value; break;
        case R8::H: registers.H = value; break;
        case R8::L: registers.L = value; break;
        case R8::HL_ADDR: bus.write(registers.H << 8 | registers.L, value); break;
        case R8::A: registers.A = value; break;
        case R8::F: registers.F = value; break;
    }
}

// r16
R16 CPU::getR16(uint8_t registerNum) {
    assert(registerNum < 4);
    switch (registerNum) {
        case 0: return R16::BC;
        case 1: return R16::DE;
        case 2: return R16::HL;
        case 3: return R16::SP;
        default:
            assert(false && "Invalid R16 index");
            return R16::SP;
    }
}
uint16_t CPU::getR16Value(R16 r16) {
    switch (r16) {
        case R16::BC: return static_cast<uint16_t>((registers.B << 8) | registers.C);
        case R16::DE: return static_cast<uint16_t>((registers.D << 8) | registers.E);
        case R16::HL: return static_cast<uint16_t>((registers.H << 8) | registers.L);
        case R16::SP: return SP;
        default: assert(false && "Invalid R16 enum");
    }
}
void CPU::setR16Value(R16 r16, uint16_t value) {
    uint8_t low = static_cast<uint8_t>(value & 0xFF);
    uint8_t high = static_cast<uint8_t>(value >> 8);

    switch (r16) {
        case R16::BC: registers.B = high; registers.C = low; break;
        case R16::DE: registers.D = high; registers.E = low; break;
        case R16::HL: registers.H = high; registers.L = low; break;
        case R16::SP: SP = value; break;
    }
}
// [r16mem]
R16MEM CPU::getR16MEM(uint8_t registerNum) {
    assert(registerNum < 4);
    // BC, DE, HLI, HLD
    switch (registerNum) {
        case 0: return R16MEM::BC;
        case 1: return R16MEM::DE;
        case 2: return R16MEM::HLI;
        case 3: return R16MEM::HLD;
        default:
            assert(false && "Invalid R16MEM index");
            return R16MEM::HLD;
    }
}
uint8_t CPU::getR16MEMValue(R16MEM r16Mem) {
    uint16_t address = 0xFEA0; // init to unused memory

    switch (r16Mem) {
        case R16MEM::BC: address = getR16Value(R16::BC); break;
        case R16MEM::DE: address = getR16Value(R16::DE); break;
        case R16MEM::HLI:
            address = getR16Value(R16::HL);
            setR16Value(R16::HL, address + 1);
            break;
        case R16MEM::HLD:
            address = getR16Value(R16::HL);
            setR16Value(R16::HL, address - 1);
            break;
    }

    return bus.read(address);
}
void CPU::setR16MEMValue(R16MEM r16Mem, uint8_t value) {
    uint16_t address = 0xFEA0; // init to unused memory

    switch (r16Mem) {
        case R16MEM::BC: address = getR16Value(R16::BC); break;
        case R16MEM::DE: address = getR16Value(R16::DE); break;
        case R16MEM::HLI:
            address = getR16Value(R16::HL);
            setR16Value(R16::HL, address + 1);
            break;
        case R16MEM::HLD:
            address = getR16Value(R16::HL);
            setR16Value(R16::HL, address - 1);
            break;
    }

    bus.write(address, value);
}
// r16stk
R16STK CPU::getR16STK(uint8_t registerNum) {
    assert(registerNum < 4);
    switch (registerNum) {
        case 0: return R16STK::BC;
        case 1: return R16STK::DE;
        case 2: return R16STK::HL;
        case 3: return R16STK::AF;
        default:
            assert(false && "Invalid R16STK index");
            return R16STK::AF;
    }
}
uint16_t CPU::getR16STKValue(R16STK r16Mem) {
    switch (r16Mem) {
        case R16STK::BC: return getR16Value(R16::BC);
        case R16STK::DE: return getR16Value(R16::DE);
        case R16STK::HL: return getR16Value(R16::HL);
        case R16STK::AF: return static_cast<uint16_t>((registers.A << 8) | (registers.F & 0xF0));
        default: assert(false && "Invalid R16STK enum");
    }
}
void CPU::setR16STKValue(R16STK r16Mem, uint16_t value) {
    switch (r16Mem) {
        case R16STK::BC: setR16Value(R16::BC, value); break;
        case R16STK::DE: setR16Value(R16::DE, value); break;
        case R16STK::HL: setR16Value(R16::HL, value); break;
        case R16STK::AF: registers.A = (value >> 8); registers.F = (value & 0xF0); break;
    }
}
// cond
bool CPU::evaluateCondition(COND condition) {
    switch (condition) {
        case COND::NZ: return !getZeroFlag();
        case COND::Z:  return getZeroFlag();
        case COND::NC: return !getCarryFlag();
        case COND::C:  return getCarryFlag();
        default:
            assert(false && "Invalid condition index");
            return false;
    }
}

// flags
void CPU::setZeroFlag(bool value) {
    uint8_t flags = getR8Value(R8::F);
    if (value) {
        setR8Value(R8::F, flags | 0b10000000);
    } else {
        setR8Value(R8::F, flags & 0b01111111);
    }
}
void CPU::setSubtractionFlag(bool value) {
    uint8_t flags = getR8Value(R8::F);
    if (value) {
        setR8Value(R8::F, flags | 0b01000000);
    } else {
        setR8Value(R8::F, flags & 0b10111111);
    }
}
void CPU::setHalfCarryFlag(bool value) {
    uint8_t flags = getR8Value(R8::F);
    if (value) {
        setR8Value(R8::F, flags | 0b00100000);
    } else {
        setR8Value(R8::F, flags & 0b11011111);
    }
}
void CPU::setCarryFlag(bool value) {
    uint8_t flags = getR8Value(R8::F);
    if (value) {
        setR8Value(R8::F, flags | 0b00010000);
    } else {
        setR8Value(R8::F, flags & 0b11101111);
    }
}

bool CPU::getZeroFlag() {
    uint8_t flags = getR8Value(R8::F);
    return static_cast<bool>(flags >> 7);
}
bool CPU::getSubtractionFlag() {
    uint8_t flags = getR8Value(R8::F);
    return static_cast<bool>((flags >> 6) & 1);
}
bool CPU::getHalfCarryFlag() {
    uint8_t flags = getR8Value(R8::F);
    return static_cast<bool>((flags >> 5) & 1);
}
bool CPU::getCarryFlag() {
    uint8_t flags = getR8Value(R8::F);
    return static_cast<bool>((flags >> 4) & 1);
}

// Helpers
uint16_t CPU::loadImm16() {
    uint8_t low = bus.read(PC++);
    uint8_t high = bus.read(PC++);
    // cout << "Loading IMM16 0x" << std::hex << static_cast<uint16_t>((high << 8) | low) << endl;
    return static_cast<uint16_t>((high << 8) | low);
}
uint8_t CPU::loadImm8() {
    // cout << "Loading IMM8 0x" << std::hex << static_cast<int>(bus.read(PC)) << endl;
    return bus.read(PC++);
}
int8_t CPU::loadE8() {
    // cout << "Loading E8 0x" << std::hex << static_cast<int>(bus.read(PC)) << endl;
    return static_cast<int8_t>(bus.read(PC++));
}
void CPU::pushStack(uint8_t value) {
    // TODO assert SP is not out out of bounds
    bus.write(--SP, value);
}
uint8_t CPU::popStack() {
    // TODO assert SP is not out out of bounds
    return bus.read(SP++);
}

// Block 0-3
void CPU::cycle() {
    handleInterrupts();

    if (!halted) {
        uint8_t opcode = bus.read(PC++);
        // cout << "Executing opcode " << std::hex << static_cast<int>(opcode) << endl;
        executeInstruction(opcode);
    }
}
void CPU::handleInterrupts() {
    if (!interruptsEnabled) {
        return;
    }

    uint8_t interruptFlags = bus.read(0xFF0F);
    uint8_t enabledInterrupts = bus.read(0xFFFF);

    uint8_t pendingInterrupts = enabledInterrupts & interruptFlags;

    if (!pendingInterrupts) {
        return;
    }

    interruptsEnabled = false;

    // Call in order of priority and clear flag
    // VBlank (0x40), LCD STAT (0x48), Timer (0x50), Serial (0x58), Joypad (0x60)
    if (pendingInterrupts & 0x01) { // VBlank
        bus.write(0xFF0F, interruptFlags & ~0x01);
        CALL_IMM16(0x0040);
    } else if (pendingInterrupts & 0x02) { // LCD STAT
        bus.write(0xFF0F, interruptFlags & ~0x02);
        CALL_IMM16(0x0048);
    } else if (pendingInterrupts & 0x04) { // Timer
        bus.write(0xFF0F, interruptFlags & ~0x04);
        CALL_IMM16(0x0050);
    } else if (pendingInterrupts & 0x08) { // Serial
        bus.write(0xFF0F, interruptFlags & ~0x08);
        CALL_IMM16(0x0058);
    } else if (pendingInterrupts & 0x10) { // Joypad
        bus.write(0xFF0F, interruptFlags & ~0x10);
        CALL_IMM16(0x0060);
    }
}
void CPU::executeInstruction(uint8_t opcode) {
    if (opcode == 0xCB) {
        uint8_t cbOpcode = bus.read(PC++);
        executeBlockCB(cbOpcode);
    } else {
        uint8_t block = (opcode >> 6);
        switch (block) {
            case 0: executeBlock0(opcode); break;
            case 1: executeBlock1(opcode); break;
            case 2: executeBlock2(opcode); break;
            case 3: executeBlock3(opcode); break;
        }
    }
}
void CPU::executeBlock0(uint8_t opcode) {
    switch (opcode) {
        case 0x00: // NOP
            break;
        case 0x01: // LD BC, imm16
            LD_R16_IMM16(R16::BC, loadImm16());
            break;
        case 0x02: // LD [BC], A
            LD_R16MEM_A(R16MEM::BC);
            break;
        case 0x03: // INC BC
            INC_R16(R16::BC);
            break;
        case 0x04: // INC B
            INC_R8(R8::B);
            break;
        case 0x05: // DEC B
            DEC_R8(R8::B);
            break;
        case 0x06: // LD B, n8
            LD_R8_IMM8(R8::B, loadImm8());
            break;
        case 0x07: // RLCA
            RLCA();
            break;
        case 0x08: // LD [imm16],SP
            LD_IMM16MEM_SP(loadImm16());
            break;
        case 0x09: // ADD HL, BC
            ADD_HL_R16(R16::BC);
            break;
        case 0x0A: // LD A, [BC]
            LD_A_R16MEM(R16MEM::BC);
            break;
        case 0x0B: // DEC BC
            DEC_R16(R16::BC);
            break;
        case 0x0C: // INC C
            INC_R8(R8::C);
            break;
        case 0x0D: // DEC C
            DEC_R8(R8::C);
            break;
        case 0x0E: // LD C, imm8
            LD_R8_IMM8(R8::C, loadImm8());
            break;
        case 0x0F: // RRCA
            RRCA();
            break;
        case 0x10: // STOP
            STOP();
            break;
        case 0x11: // LD DE, n16
            LD_R16_IMM16(R16::DE, loadImm16());
            break;
        case 0x12: // LD [DE], A
            LD_R16MEM_A(R16MEM::DE);
            break;
        case 0x13: // INC DE
            INC_R16(R16::DE);
            break;
        case 0x14: // INC D
            INC_R8(R8::D);
            break;
        case 0x15: // DEC D
            DEC_R8(R8::D);
            break;
        case 0x16: // LD D, imm8
            LD_R8_IMM8(R8::D, loadImm8());
            break;
        case 0x17: // RLA
            RLA();
            break;
        case 0x18: // JR e8
            JR_IMM8(loadE8());
            break;
        case 0x19: // ADD HL, DE
            ADD_HL_R16(R16::DE);
            break;
        case 0x1A: // LD A, [DE]
            LD_A_R16MEM(R16MEM::DE);
            break;
        case 0x1B: // DEC DE
            DEC_R16(R16::DE);
            break;
        case 0x1C: // INC E
            INC_R8(R8::E);
            break;
        case 0x1D: // DEC E
            DEC_R8(R8::E);
            break;
        case 0x1E: // LD E, imm8
            LD_R8_IMM8(R8::E, loadImm8());
            break;
        case 0x1F: // RRA
            RRA();
            break;
        case 0x20: // JR NZ, e8
            JR_COND_IMM8(COND::NZ, loadE8());
            break;
        case 0x21: // LD HL, n16
            LD_R16_IMM16(R16::HL, loadImm16());
            break;
        case 0x22: // LD [HL+], A
            LD_R16MEM_A(R16MEM::HLI);
            break;
        case 0x23: // INC HL
            INC_R16(R16::HL);
            break;
        case 0x24: // INC H
            INC_R8(R8::H);
            break;
        case 0x25: // DEC H
            DEC_R8(R8::H);
            break;
        case 0x26: // LD H, n8
            LD_R8_IMM8(R8::H, loadImm8());
            break;
        case 0x27: // DAA
            DAA();
            break;
        case 0x28: // JR Z, e8
            JR_COND_IMM8(COND::Z, loadE8());
            break;
        case 0x29: // ADD HL, HL
            ADD_HL_R16(R16::HL);
            break;
        case 0x2A: // LD A, HL+
            LD_A_R16MEM(R16MEM::HLI);
            break;
        case 0x2B: // DEC HL
            DEC_R16(R16::HL);
            break;
        case 0x2C: // INC L
            INC_R8(R8::L);
            break;
        case 0x2D: // DEC L
            DEC_R8(R8::L);
            break;
        case 0x2E: // LD L, n8
            LD_R8_IMM8(R8::L, loadImm8());
            break;
        case 0x2F: // CPL
            CPL();
            break;
        case 0x30: // JR NC, e8
            JR_COND_IMM8(COND::NC, loadE8());
            break;
        case 0x31: // LD SP, n16
            LD_R16_IMM16(R16::SP, loadImm16());
            break;
        case 0x32: // LD [HL-], A
            LD_R16MEM_A(R16MEM::HLD);
            break;
        case 0x33: // INC SP
            INC_R16(R16::SP);
            break;
        case 0x34: // INC [HL]
            INC_R8(R8::HL_ADDR);
            break;
        case 0x35: // DEC [HL]
            DEC_R8(R8::HL_ADDR);
            break;
        case 0x36: // LD [HL], n8
            LD_R8_IMM8(R8::HL_ADDR, loadImm8());
            break;
        case 0x37: // SCF
            SCF();
            break;
        case 0x38: // JR C, e8
            JR_COND_IMM8(COND::C, loadE8());
            break;
        case 0x39: // ADD HL, SP
            ADD_HL_R16(R16::SP);
            break;
        case 0x3A: // LD A, [HL-]
            LD_A_R16MEM(R16MEM::HLD);
            break;
        case 0x3B: // DEC SP
            DEC_R16(R16::SP);
            break;
        case 0x3C: // INC A
            INC_R8(R8::A);
            break;
        case 0x3D: // DEC A
            DEC_R8(R8::A);
            break;
        case 0x3E: // LD A, n8
            LD_R8_IMM8(R8::A, loadImm8());
            break;
        case 0x3F: // CCF
            CCF();
            break;
        default:
            assert(false && "Opcode should not have been assigned to block 0");
    }
}
void CPU::executeBlock1(uint8_t opcode) {
    // Halt
    if (opcode == 0x76) {
        // TODO handle halt bug
        halted = true;
        return;
    }

    // LD r8, r8
    R8 src = getR8(opcode & 0b111);
    R8 dst = getR8((opcode >> 3) & 0b111);

    uint8_t srcValue = getR8Value(src);
    setR8Value(dst, srcValue);
}
void CPU::executeBlock2(uint8_t opcode) {
    uint8_t operationIdx = (opcode >> 3) & 0b111;
    uint8_t registerIdx = opcode & 0b111;

    R8 r8 = getR8(registerIdx);
    uint8_t r8Value = getR8Value(r8);

    // OPERATION a, r8
    switch (operationIdx) {
        case 0x0: ADD(r8Value); break;
        case 0x1: ADC(r8Value); break;
        case 0x2: SUB(r8Value); break;
        case 0x3: SBC(r8Value); break;
        case 0x4: AND(r8Value); break;
        case 0x5: XOR(r8Value); break;
        case 0x6: OR(r8Value); break;
        case 0x7: CP(r8Value); break;
    }
}
void CPU::executeBlock3(uint8_t opcode) {
    switch (opcode) {
        case 0xC0:
            RET_COND(COND::NZ);
            break;
        case 0xC1:
            POP_R16STK(R16STK::BC);
            break;
        case 0xC2:
            JP_COND_IMM16(COND::NZ, loadImm16());
            break;
        case 0xC3:
            JP_IMM16(loadImm16());
            break;
        case 0xC4:
            CALL_COND_IMM16(COND::NZ, loadImm16());
            break;
        case 0xC5:
            PUSH_R16STK(R16STK::BC);
            break;
        case 0xC6:
            ADD(loadImm8());
            break;
        case 0xC7:
            RST_TGT3(0x00);
            break;
        case 0xC8:
            RET_COND(COND::Z);
            break;
        case 0xC9:
            RET();
            break;
        case 0xCA:
            JP_COND_IMM16(COND::Z, loadImm16());
            break;
        case 0xCB:
            // Prefixed operator
            assert(false && "Should have been handled by CB block");
            break;
        case 0xCC:
            CALL_COND_IMM16(COND::Z, loadImm16());
            break;
        case 0xCD:
            CALL_IMM16(loadImm16());
            break;
        case 0xCE:
            ADC(loadImm8());
            break;
        case 0xCF:
            RST_TGT3(0x08);
            break;
        case 0xD0:
            RET_COND(COND::NC);
            break;
        case 0xD1:
            POP_R16STK(R16STK::DE);
            break;
        case 0xD2:
            JP_COND_IMM16(COND::NC, loadImm16());
            break;
        case 0xD4:
            CALL_COND_IMM16(COND::NC, loadImm16());
            break;
        case 0xD5:
            PUSH_R16STK(R16STK::DE);
            break;
        case 0xD6:
            SUB(loadImm8());
            break;
        case 0xD7:
            RST_TGT3(0x10);
            break;
        case 0xD8:
            RET_COND(COND::C);
            break;
        case 0xD9:
            RETI();
            break;
        case 0xDA:
            JP_COND_IMM16(COND::C, loadImm16());
            break;
        case 0xDC:
            CALL_COND_IMM16(COND::C, loadImm16());
            break;
        case 0xDE:
            SBC(loadImm8());
            break;
        case 0xDF:
            RST_TGT3(0x18);
            break;
        case 0xE0:
            LDH_IMM8MEM_A(loadImm8());
            break;
        case 0xE1:
            POP_R16STK(R16STK::HL);
            break;
        case 0xE2:
            LDH_CMEM_A();
            break;
        case 0xE5:
            PUSH_R16STK(R16STK::HL);
            break;
        case 0xE6:
            AND(loadImm8());
            break;
        case 0xE7:
            RST_TGT3(0x20);
            break;
        case 0xE8:
            ADD_SP_E8(loadE8());
            break;
        case 0xE9:
            JP_HL();
            break;
        case 0xEA:
            LD_IMM16MEM_A(loadImm16());
            break;
        case 0xEE:
            XOR(loadImm8());
            break;
        case 0xEF:
            RST_TGT3(0x28);
            break;
        case 0xF0:
            LDH_A_IMM8MEM(loadImm8());
            break;
        case 0xF1:
            POP_R16STK(R16STK::AF);
            break;
        case 0xF2:
            LDH_A_CMEM();
            break;
        case 0xF3:
            DI();
            break;
        case 0xF5:
            PUSH_R16STK(R16STK::AF);
            break;
        case 0xF6:
            OR(loadImm8());
            break;
        case 0xF7:
            RST_TGT3(0x30);
            break;
        case 0xF8:
            LD_HL_SP_PLUS_E8(loadE8());
            break;
        case 0xF9:
            LD_SP_HL();
            break;
        case 0xFA:
            LD_A_IMM16MEM(loadImm16());
            break;
        case 0xFB:
            EI();
            break;
        case 0xFE:
            CP(loadImm8());
            break;
        case 0xFF:
            RST_TGT3(0x38);
            break;
        default:
            assert("Invalid opcode in block 3");
    }
}
void CPU::executeBlockCB(uint8_t opcode) {
    uint8_t registerIdx = opcode & 0b111;
    R8 targetRegister = getR8(registerIdx);

    uint8_t bitIdx = (opcode >> 3) & 0b111;
    switch (opcode >> 6) {
        case 0: // Rotates and Shifts
            switch (opcode >> 3) {
                case 0: RLC(targetRegister); break;
                case 1: RRC(targetRegister); break;
                case 2: RL(targetRegister); break;
                case 3: RR(targetRegister); break;
                case 4: SLA(targetRegister); break;
                case 5: SRA(targetRegister); break;
                case 6: SWAP(targetRegister); break;
                case 7: SRL(targetRegister); break;
            } break;
        case 1: BIT(bitIdx, targetRegister); break;
        case 2: RES(bitIdx, targetRegister); break;
        case 3: SET(bitIdx, targetRegister); break;
    }
}

// Block 0
void CPU::NOOP() {

}

void CPU::LD_R16_IMM16(R16 reg, uint16_t imm16) {
    setR16Value(reg, imm16);
}
void CPU::LD_R16MEM_A(R16MEM address) {
    setR16MEMValue(address, registers.A);
}
void CPU::LD_A_R16MEM(R16MEM address) {
    uint8_t value = getR16MEMValue(address);
    setR8Value(R8::A, value);
}
void CPU::LD_IMM16MEM_SP(uint16_t address) {
    uint8_t low = SP & 0xFF;
    uint8_t high = SP >> 8;

    bus.write(address, low);
    bus.write(address + 1, high);
}

void CPU::INC_R16(R16 reg) {
    uint16_t value = getR16Value(reg);
    setR16Value(reg, value + 1);
}
void CPU::DEC_R16(R16 reg) {
    uint16_t value = getR16Value(reg);
    setR16Value(reg, value - 1);
}
void CPU::ADD_HL_R16(R16 reg) {
    uint16_t hlValue = getR16Value(R16::HL);
    uint16_t value = getR16Value(reg);

    uint16_t result = static_cast<uint16_t>(hlValue + value);
    setR16Value(R16::HL, result);

    setSubtractionFlag(false);
    setHalfCarryFlag((hlValue & 0xFFF) + (value & 0xFFF) > 0xFFF);
    setCarryFlag(hlValue + value > 0xFFFF);
}

void CPU::INC_R8(R8 reg) {
    uint8_t originalValue = getR8Value(reg);
    uint8_t result = originalValue + 1;
    setR8Value(reg, result);
    // TODO set flags
    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag((originalValue & 0x0F) == 0x0F);
}
void CPU::DEC_R8(R8 reg) {
    uint8_t originalValue = getR8Value(reg);
    uint8_t result = originalValue - 1;
    setR8Value(reg, result);
    // TODO set flags
    setZeroFlag(result == 0);
    setSubtractionFlag(true);
    setHalfCarryFlag((originalValue & 0x0F) == 0x00);
}

void CPU::LD_R8_IMM8(R8 reg, uint8_t imm8) {
    setR8Value(reg, imm8);
}

void CPU::RLCA() {
    uint8_t value = getR8Value(R8::A);
    uint8_t msb = value >> 7;

    uint8_t result = static_cast<uint8_t>((value << 1) | msb);
    setR8Value(R8::A, result);

    setZeroFlag(false);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(static_cast<bool>(msb));
}
void CPU::RRCA() {
    uint8_t value = getR8Value(R8::A);
    uint8_t lsb = value & 0b1;

    uint8_t result = static_cast<uint8_t>((value >> 1) | (lsb << 7));
    setR8Value(R8::A, result);

    setZeroFlag(false);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(static_cast<bool>(lsb));
}
void CPU::RLA() {
    uint8_t value = getR8Value(R8::A);
    uint8_t msb = value >> 7;
    uint8_t carry = static_cast<uint8_t>(getCarryFlag());

    uint8_t result = static_cast<uint8_t>((value << 1) | carry);
    setR8Value(R8::A, result);

    setZeroFlag(false);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(static_cast<bool>(msb));
}
void CPU::RRA() {
    uint8_t value = getR8Value(R8::A);
    uint8_t lsb = value & 0x1;
    uint8_t carry = static_cast<uint8_t>(getCarryFlag());

    uint8_t result = static_cast<uint8_t>((value >> 1) | (carry << 7));
    setR8Value(R8::A, result);

    setZeroFlag(false);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(static_cast<bool>(lsb));
}

void CPU::DAA() {
    uint8_t a = getR8Value(R8::A);
    uint8_t correction = 0;
    bool setCarry = false;

    if (getSubtractionFlag()) {
        if (getHalfCarryFlag()) correction |= 0x06;
        if (getCarryFlag()) {
            correction |= 0x60;
            setCarry = true;
        }
        a = static_cast<uint8_t>(a - correction);
    } else {
        if (getHalfCarryFlag() || (a & 0x0F) > 0x09) correction |= 0x06;
        if (getCarryFlag() || a > 0x99) {
            correction |= 0x60;
            setCarry = true;
        }
        a = static_cast<uint8_t>(a + correction);
    }

    setZeroFlag(a == 0);
    setHalfCarryFlag(false);
    if (setCarry) {
        setCarryFlag(true);
    }

    setR8Value(R8::A, a);
}

void CPU::CPL() {
    uint8_t value = getR8Value(R8::A);
    setR8Value(R8::A, ~value);

    setSubtractionFlag(true);
    setHalfCarryFlag(true);
}
void CPU::SCF() {
    setCarryFlag(true);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
}
void CPU::CCF() {
    setCarryFlag(!getCarryFlag());
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
}

void CPU::JR_IMM8(int8_t imm8) {
    PC = static_cast<uint16_t>(PC + imm8);
}
void CPU::JR_COND_IMM8(COND condition, int8_t imm8) {
    if (evaluateCondition(condition)) {
        PC = static_cast<uint16_t>(PC + imm8);
    }
}

void CPU::STOP() {
    // assert(0 != 0);
}

// Block 2
void CPU::ADD(uint8_t value) {
    uint8_t originalValue = getR8Value(R8::A);

    uint8_t result = static_cast<uint8_t>(originalValue + value);
    setR8Value(R8::A, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag((originalValue & 0xF) + (value & 0xF) > 0xF);
    setCarryFlag(result < originalValue);
}
void CPU::ADC(uint8_t value) {
    uint8_t originalValue = getR8Value(R8::A);
    uint8_t carry = static_cast<uint8_t>(getCarryFlag());

    uint8_t result = static_cast<uint8_t>(originalValue + value + carry);
    setR8Value(R8::A, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag((originalValue & 0xF) + (value & 0xF) + carry > 0xF);
    setCarryFlag(originalValue + value + carry > 0xFF);
}
void CPU::SUB(uint8_t value) {
    uint8_t originalValue = getR8Value(R8::A);

    uint8_t result = static_cast<uint8_t>(originalValue - value);
    setR8Value(R8::A, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(true);
    setHalfCarryFlag((originalValue & 0xF) < (value & 0xF));
    setCarryFlag(value > originalValue);
}
void CPU::SBC(uint8_t value) {
    uint8_t originalValue = getR8Value(R8::A);
    uint8_t carry = static_cast<uint8_t>(getCarryFlag());

    uint8_t result = static_cast<uint8_t>(originalValue - value - carry);
    setR8Value(R8::A, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(true);
    setHalfCarryFlag((originalValue & 0xF) < ((value & 0xF) + carry));
    setCarryFlag((value + carry) > originalValue);
}
void CPU::AND(uint8_t value) {
    uint8_t result = getR8Value(R8::A) & value;
    setR8Value(R8::A, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag(true);
    setCarryFlag(false);
}
void CPU::XOR(uint8_t value) {
    uint8_t result = getR8Value(R8::A) ^ value;
    setR8Value(R8::A, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(false);
}
void CPU::OR(uint8_t value) {
    uint8_t result = getR8Value(R8::A) | value;
    setR8Value(R8::A, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(false);
}
void CPU::CP(uint8_t value) {
    uint8_t originalValue = getR8Value(R8::A);
    uint8_t result = originalValue - value;

    setZeroFlag(result == 0);
    setSubtractionFlag(true);
    setHalfCarryFlag((originalValue & 0xF) < (value & 0xF));
    setCarryFlag(value > originalValue);
}

void CPU::RET_COND(COND condition) {
    if (evaluateCondition(condition)) {
        RET();
    }
}
void CPU::RET() {
    uint8_t low = popStack();
    uint8_t high = popStack();
    PC = static_cast<uint16_t>((high << 8) | low);
}
void CPU::RETI() {
    RET();
    interruptsEnabled = true;
}
void CPU::JP_COND_IMM16(COND condition, uint16_t value) {
    if (evaluateCondition(condition)) {
        PC = value;
    }
}
void CPU::JP_IMM16(uint16_t address) {
    PC = address;
}
void CPU::JP_HL() {
    PC = getR16Value(R16::HL);
}
void CPU::CALL_COND_IMM16(COND condition, uint16_t address) {
    if (evaluateCondition(condition)) {
        CALL_IMM16(address);
    }
}
void CPU::CALL_IMM16(uint16_t address) {
    uint8_t low = PC & 0xFF;
    uint8_t high = PC >> 8;

    // TODO assert that SP doesn't underflow
    pushStack(high);
    pushStack(low);
    PC = address;
}
void CPU::RST_TGT3(uint8_t address) {
    assert((address % 8) == 0 && (address <= 0x38));
    uint8_t low = PC & 0xFF;
    uint8_t high = PC >> 8;

    pushStack(high);
    pushStack(low);
    PC = address;
}

void CPU::POP_R16STK(R16STK reg) {
    // TODO assert that SP doesn't underflow
    uint8_t low = popStack();
    uint8_t high = popStack();
    setR16STKValue(reg, static_cast<uint16_t>((high << 8) | low));
}
void CPU::PUSH_R16STK(R16STK reg) {
    uint16_t regValue = getR16STKValue(reg);

    uint8_t low = regValue & 0xFF;
    uint8_t high = regValue >> 8;
    pushStack(high);
    pushStack(low);
}

void CPU::LDH_CMEM_A() {
    uint8_t offset = getR8Value(R8::C);
    uint8_t value = getR8Value(R8::A);
    bus.write(static_cast<uint16_t>(0xFF00 + offset), value);
}
void CPU::LDH_IMM8MEM_A(uint8_t offset) {
    uint8_t value = getR8Value(R8::A);
    bus.write(static_cast<uint16_t>(0xFF00 + offset), value);
}
void CPU::LD_IMM16MEM_A(uint16_t address) {
    uint8_t value = getR8Value(R8::A);
    bus.write(address, value);
}
void CPU::LDH_A_CMEM() {
    uint8_t offset = getR8Value(R8::C);
    uint8_t value = bus.read(0xFF00 + offset);
    setR8Value(R8::A, value);
}
void CPU::LDH_A_IMM8MEM(uint8_t offset) {
    uint8_t value = bus.read(0xFF00 + offset);
    setR8Value(R8::A, value);
}
void CPU::LD_A_IMM16MEM(uint16_t address) {
    uint8_t value = bus.read(address);
    setR8Value(R8::A, value);
}

void CPU::ADD_SP_E8(int8_t value) {
    // TODO handle flags
    uint16_t originalValue = SP;
    uint16_t result = static_cast<uint16_t>(SP + value);
    SP = result;

    setZeroFlag(false);
    setSubtractionFlag(false);
    setHalfCarryFlag(((originalValue ^ value ^ result) & 0x10) != 0);
    setCarryFlag(((originalValue ^ value ^ result) & 0x100) != 0);
}
void CPU::LD_HL_SP_PLUS_E8(int8_t value) {
    uint16_t sum = static_cast<uint16_t>(SP + value);
    setR16Value(R16::HL, sum);

    setZeroFlag(false);
    setSubtractionFlag(false);
    setHalfCarryFlag(((SP ^ value ^ sum) & 0x10) != 0);
    setCarryFlag(((SP ^ value ^ sum) & 0x100) != 0);
}
void CPU::LD_SP_HL() {
    uint16_t value = getR16Value(R16::HL);
    setR16Value(R16::SP, value);
}

void CPU::DI() {
    interruptsEnabled = false;
}
void CPU::EI() {
    interruptsEnabled = true;
}

void CPU::RLC(R8 reg) {
    uint8_t value = getR8Value(reg);
    uint8_t msb = value >> 7;

    uint8_t result = static_cast<uint8_t>((value << 1) | msb);
    setR8Value(reg, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(static_cast<bool>(msb));
}
void CPU::RRC(R8 reg) {
    uint8_t value = getR8Value(reg);
    uint8_t lsb = value & 0x1;

    uint8_t result = static_cast<uint8_t>((value >> 1) | (lsb << 7));
    setR8Value(reg, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(static_cast<bool>(lsb));
}
void CPU::RL(R8 reg) {
    uint8_t value = getR8Value(reg);
    uint8_t msb = value >> 7;
    uint8_t carry = static_cast<uint8_t>(getCarryFlag());

    uint8_t result = static_cast<uint8_t>((value << 1) | carry);
    setR8Value(reg, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(static_cast<bool>(msb));
}
void CPU::RR(R8 reg) {
    uint8_t value = getR8Value(reg);
    uint8_t lsb = value & 0x1;
    uint8_t carry = static_cast<uint8_t>(getCarryFlag());

    uint8_t result = static_cast<uint8_t>((value >> 1) | (carry << 7));
    setR8Value(reg, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(static_cast<bool>(lsb));
}
void CPU::SLA(R8 reg) {
    uint8_t value = getR8Value(reg);
    uint8_t msb = value >> 7;

    uint8_t result = static_cast<uint8_t>(value << 1);
    setR8Value(reg, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(static_cast<bool>(msb));
}
void CPU::SRA(R8 reg) {
    uint8_t value = getR8Value(reg);
    uint8_t lsb = value & 0x1;

    uint8_t result = static_cast<uint8_t>((value >> 1) | (value & 0x80));
    setR8Value(reg, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(static_cast<bool>(lsb));
}
void CPU::SWAP(R8 reg) {
    uint8_t value = getR8Value(reg);

    uint8_t result = static_cast<uint8_t>((value << 4) | (value >> 4));
    setR8Value(reg, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(false);
}
void CPU::SRL(R8 reg) {
    uint8_t value = getR8Value(reg);
    uint8_t lsb = value & 0x1;

    uint8_t result = static_cast<uint8_t>(value >> 1);
    setR8Value(reg, result);

    setZeroFlag(result == 0);
    setSubtractionFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(static_cast<bool>(lsb));
}
void CPU::BIT(uint8_t n, R8 reg) {
    assert(n < 8);
    uint8_t value = getR8Value(reg);
    setZeroFlag(static_cast<bool>(!((value >> n) & 0x1)));
    setSubtractionFlag(false);
    setHalfCarryFlag(true);
}
void CPU::RES(uint8_t n, R8 reg) {
    assert(n < 8);
    uint8_t value = getR8Value(reg);
    uint8_t result = value & ~(1 << n);
    setR8Value(reg, result);
}
void CPU::SET(uint8_t n, R8 reg) {
    assert(n < 8);
    uint8_t value = getR8Value(reg);
    uint8_t result = value | (1 << n);
    setR8Value(reg, result);
}








