#pragma once
#include <functional>
#include <iomanip>

#include "mmu.hpp"
#include "register_types.hpp"

struct Registers {
    uint8_t A;
    uint8_t F;
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;

    Registers():
        A(0x01),
        F(0xB0),
        B(0x00),
        C(0x13),
        D(0x00),
        E(0xD8),
        H(0x01),
        L(0x4D) {}
};

class CPU {
private:
    Registers registers = Registers();
    uint16_t PC{0x100};
    uint16_t SP{0xFFE};

    MMU& bus;
    bool halted{false};
    bool interruptsEnabled{true};

    // Getters & Setters

    // r8
    R8 getR8(uint8_t registerNum);
    uint8_t getR8Value(R8 r8);
    void setR8Value(R8 r8, uint8_t value);

    // r16
    R16 getR16(uint8_t registerNum);
    uint16_t getR16Value(R16 r16);
    void setR16Value(R16 r16, uint16_t value);

    // [r16mem]
    R16MEM getR16MEM(uint8_t registerNum);
    uint8_t getR16MEMValue(R16MEM r16);
    void setR16MEMValue(R16MEM r16Mem, uint8_t value);

    // r16stk
    R16STK getR16STK(uint8_t registerNum);
    uint16_t getR16STKValue(R16STK r16Mem);
    void setR16STKValue(R16STK r16Mem, uint16_t value);

    // cond
    bool evaluateCondition(COND condition);

    // flags
    void setZeroFlag(bool value);
    void setSubtractionFlag(bool value);
    void setHalfCarryFlag(bool value);
    void setCarryFlag(bool value);

    bool getZeroFlag();
    bool getSubtractionFlag();
    bool getHalfCarryFlag();
    bool getCarryFlag();

    // Helpers
    uint16_t loadImm16();
    uint8_t loadImm8();
    int8_t loadE8();
    void pushStack(uint8_t value);
    uint8_t popStack();

    // Code blocks
    void cycle();
    void handleInterrupts();
    void executeInstruction(uint8_t instruction);
    void executeBlock0(uint8_t opcode);
    void executeBlock1(uint8_t opcode);
    void executeBlock2(uint8_t opcode);
    void executeBlock3(uint8_t opcode);
    void executeBlockCB(uint8_t opcode);

    // Block 0
    void NOOP();

    void LD_R16_IMM16(R16 reg, uint16_t imm16);
    void LD_R16MEM_A(R16MEM address);
    void LD_A_R16MEM(R16MEM address);
    void LD_IMM16MEM_SP(uint16_t address);

    void INC_R16(R16 reg);
    void DEC_R16(R16 reg);
    void ADD_HL_R16(R16 reg);

    void INC_R8(R8 reg);
    void DEC_R8(R8 reg);

    void LD_R8_IMM8(R8 reg, uint8_t value);

    void RLCA();
    void RRCA();
    void RLA();
    void RRA();
    void DAA();
    void CPL();
    void SCF();
    void CCF();

    void JR_IMM8(int8_t value);
    void JR_COND_IMM8(COND condition, int8_t value);

    void STOP();

    // Block 2
    void ADD(uint8_t value);
    void ADC(uint8_t value);
    void SUB(uint8_t value);
    void SBC(uint8_t value);
    void AND(uint8_t value);
    void XOR(uint8_t value);
    void OR(uint8_t value);
    void CP(uint8_t value);

    void RET_COND(COND condition);
    void RET();
    void RETI();
    void JP_COND_IMM16(COND condition, uint16_t address);
    void JP_IMM16(uint16_t address);
    void JP_HL();
    void CALL_COND_IMM16(COND condition, uint16_t address);
    void CALL_IMM16(uint16_t address);
    void RST_TGT3(uint8_t address);

    void POP_R16STK(R16STK reg);
    void PUSH_R16STK(R16STK reg);

    void LDH_CMEM_A();
    void LDH_IMM8MEM_A(uint8_t offset);
    void LD_IMM16MEM_A(uint16_t address);
    void LDH_A_CMEM();
    void LDH_A_IMM8MEM(uint8_t offset);
    void LD_A_IMM16MEM(uint16_t address);

    // TODO add other operators in necessary
    void ADD_SP_E8(int8_t value);
    void LD_HL_SP_PLUS_E8(int8_t offset);
    void LD_SP_HL();

    // interrupts
    void DI();
    void EI();

    // Prefixed operators
    void RLC(R8 reg);
    void RRC(R8 reg);
    void RL(R8 reg);
    void RR(R8 reg);
    void SLA(R8 reg);
    void SRA(R8 reg);
    void SWAP(R8 reg);
    void SRL(R8 reg);
    void BIT(uint8_t n, R8 reg);
    void RES(uint8_t n, R8 reg);
    void SET(uint8_t n, R8 reg);
public:
    CPU(MMU& bus): bus(bus) {}
    void run() {
        while (true) {
            cycle();
        }
    }

    void printInfo() {
        std::cout << "--- CPU Info ---" << std::endl;
        std::cout << std::hex << std::uppercase << std::setfill('0');

        std::cout << "Registers:" << std::endl;
        std::cout << "  A: " << std::setw(2) << static_cast<unsigned int>(registers.A)
                  << "  F: " << std::setw(2) << static_cast<unsigned int>(registers.F) << std::endl;
        std::cout << "  B: " << std::setw(2) << static_cast<unsigned int>(registers.B)
                  << "  C: " << std::setw(2) << static_cast<unsigned int>(registers.C) << std::endl;
        std::cout << "  D: " << std::setw(2) << static_cast<unsigned int>(registers.D)
                  << "  E: " << std::setw(2) << static_cast<unsigned int>(registers.E) << std::endl;
        std::cout << "  H: " << std::setw(2) << static_cast<unsigned int>(registers.H)
                  << "  L: " << std::setw(2) << static_cast<unsigned int>(registers.L) << std::endl;

        std::cout << "  AF: " << std::setw(4) << ((static_cast<uint16_t>(registers.A) << 8) | registers.F) << std::endl;
        std::cout << "  BC: " << std::setw(4) << ((static_cast<uint16_t>(registers.B) << 8) | registers.C) << std::endl;
        std::cout << "  DE: " << std::setw(4) << ((static_cast<uint16_t>(registers.D) << 8) | registers.E) << std::endl;
        std::cout << "  HL: " << std::setw(4) << ((static_cast<uint16_t>(registers.H) << 8) | registers.L) << std::endl;

        std::cout << "Program Counter (PC): " << std::setw(4) << PC << std::endl;
        std::cout << "Stack Pointer (SP):   " << std::setw(4) << SP << std::endl;

        std::cout << "Flags:" << std::endl;
        std::cout << "  Zero (Z):       " << (getZeroFlag() ? "1" : "0") << std::endl;
        std::cout << "  Subtraction (N):" << (getSubtractionFlag() ? "1" : "0") << std::endl;
        std::cout << "  Half Carry (H): " << (getHalfCarryFlag() ? "1" : "0") << std::endl;
        std::cout << "  Carry (C):      " << (getCarryFlag() ? "1" : "0") << std::endl;

        std::cout << "Halted:           " << (halted ? "True" : "False") << std::endl;
        std::cout << "Interrupts Enabled: " << (interruptsEnabled ? "True" : "False") << std::endl;
        std::cout << "------------------" << std::endl << std::endl;

        // Reset formatting to default for subsequent outputs
        std::cout << std::dec << std::nouppercase << std::setfill(' ');
    }
};
