#pragma once

#include <cstdint>

typedef uint8_t u8;
typedef int32_t i32;

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

struct InstructionLiteral
{
  OpCode Code = OpCode::Count;
  u8 Sequence = 0;
  i32 Length = 0;
};

const char* JumpOpCodeToMnemonic(OpCode code);
i32 ReadValue(u8* memory, i32* index, u8 w);
const char* GetRegisterOrMemory(u8 mod, u8 rm, u8 w, u8* memory, i32* index, char* buffer);

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

InstructionLiteral test = {OpCode::AccumulatorToMemory, 0b11, 2};