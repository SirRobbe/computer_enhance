#pragma once

#include <cstdint>

// NOTE(Fabi):
// Basic typedefs for the standardized sized integers for shorter
// more readable names.
typedef uint8_t u8;
typedef int32_t i32;

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
i32 ReadValue(u8* memory, i32* index, u8 w);

// Returns the name of the register or the memory address specified by the reg field
const char* GetRegisterOrMemory(u8 mod, u8 rm, u8 w, u8* memory, i32* index, char* buffer);

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