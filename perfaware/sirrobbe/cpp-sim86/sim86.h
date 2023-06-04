#pragma once

#include <cstdint>

// NOTE(Fabi):
// Basic typedefs for the standardized sized integers for shorter
// more readable names.
typedef uint8_t u8;
typedef uint16_t u16;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

const char* PrintOption = "--print";
const char* SimulationOption = "--sim";

// List of all op codes that are supported by the simulation
enum struct OpCode
{
  MoveRegisterOrMemoryToOrFromRegister,
  ImmediateToRegisterOrMemory,
  MoveImmediateToRegister,
  MemoryToAccumulator,
  AccumulatorToMemory,
  AddRegisterOrMemoryAndRegister,
  AddImmediateToRegisterOrMemory,
  AddImmediateToAccumulator,
  SubRegisterOrMemoryAndRegister,
  SubImmediateToRegisterOrMemory,
  SubImmediateToAccumulator,
  CmpRegisterOrMemoryAndRegister,
  CmpImmediateToRegisterOrMemory,
  CmpImmediateToAccumulator,

  JumpOnEqual,
  JumpOnLess,
  JumpOnLessOrEqual,
  JumpOnBelow,
  JumpOnBelowOrEqual,
  JumpOnParity,
  JumpOnOverflow,
  JumpOnSign,
  JumpOnNotEqual,
  JumpOnNotLess,
  JumpOnNotLessOrEqual,
  JumpOnNotBelow,
  JumpOnNotBelowOrEqual,
  JumpOnNotParity,
  JumpOnNotOverflow,
  JumpOnNotSign,
  LoopCxTimes,
  LoopWhileZero,
  LoopWhileNotZero,
  JumpOnCxZero,

  Count = 0,
};

// Binary definition of a cpu instruction
struct InstructionLiteral
{
  // The op code that maps to the binary definition
  OpCode Code = OpCode::Count;

  // The binary code which encodes the instruction
  u8 Sequence = 0;

  // The Length of the binary code. This is important because the bit pattern
  // of all instructions is not of the same size.
  i32 Length = 0;
};

// Returns the mnemonic for any op code that is related to a jump instruction
const char* JumpOpCodeToMnemonic(OpCode code);

// Reads one ore two bytes from the specified position of the program
i32 ReadValue(u8* memory, u16* index, u8 w);

// Returns the name of the register or the memory address specified by the reg field
const char* GetRegisterOrMemory(u8 mod, u8 rm, u8 w, u8* memory, u16* index, char* buffer);

// Look up table for all register names
const char* regTable[][2] = {
    {"al", "ax"},
    {"cl", "cx"},
    {"dl", "dx"},
    {"bl", "bx"},
    {"ah", "sp"},
    {"ch", "bp"},
    {"dh", "si"},
    {"bh", "di"},
};

// Look up table for all effective address calculations
const char* effectiveAddressCalculationTable[] = {
    "bx + si",
    "bx + di",
    "bp + si",
    "bp + di",
    "si",
    "di",
    "bp",
    "bx",
};

InstructionLiteral instructions[] = {
      {OpCode::MoveRegisterOrMemoryToOrFromRegister, 0b100010, 6},
      {OpCode::ImmediateToRegisterOrMemory, 0b1100011, 7},
      {OpCode::MoveImmediateToRegister, 0b1011, 4},
      {OpCode::MemoryToAccumulator, 0b1010000, 7},
      {OpCode::AccumulatorToMemory, 0b1010001, 7},
      {OpCode::AddRegisterOrMemoryAndRegister, 0b000000, 6},
      {OpCode::AddImmediateToRegisterOrMemory, 0b100000, 6},
      {OpCode::AddImmediateToAccumulator, 0b0000010, 7},
      {OpCode::SubRegisterOrMemoryAndRegister, 0b001010, 6},
      {OpCode::SubImmediateToRegisterOrMemory, 0b100000, 6},
      {OpCode::SubImmediateToAccumulator, 0b0010110, 7},
      {OpCode::CmpRegisterOrMemoryAndRegister, 0b001110, 6},
      {OpCode::CmpImmediateToRegisterOrMemory, 0b100000, 6},
      {OpCode::CmpImmediateToAccumulator, 0b0011110, 7},

      {OpCode::JumpOnEqual, 0b01110100, 8},
      {OpCode::JumpOnLess, 0b01111100, 8},
      {OpCode::JumpOnLessOrEqual, 0b01111110, 8},
      {OpCode::JumpOnBelow, 0b01110010, 8},
      {OpCode::JumpOnBelowOrEqual, 0b01110110, 8},
      {OpCode::JumpOnParity, 0b01111010, 8},
      {OpCode::JumpOnOverflow, 0b01110000, 8},
      {OpCode::JumpOnSign, 0b01111000, 8},
      {OpCode::JumpOnNotEqual, 0b01110101, 8},
      {OpCode::JumpOnNotLess, 0b01111101, 8},
      {OpCode::JumpOnNotLessOrEqual, 0b01111111, 8},
      {OpCode::JumpOnNotBelow, 0b01110011, 8},
      {OpCode::JumpOnNotBelowOrEqual, 0b01110111, 8},
      {OpCode::JumpOnNotParity, 0b01111011, 8},
      {OpCode::JumpOnNotOverflow, 0b01110001, 8},
      {OpCode::JumpOnNotSign, 0b01111001, 8},
      {OpCode::LoopCxTimes, 0b11100010, 8},
      {OpCode::LoopWhileZero, 0b11100001, 8},
      {OpCode::LoopWhileNotZero, 0b11100000, 8},
      {OpCode::JumpOnCxZero, 0b11100011, 8},

  };

  i32 instructionCount = sizeof(instructions) / sizeof(InstructionLiteral);