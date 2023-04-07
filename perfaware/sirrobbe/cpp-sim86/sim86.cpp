#include <cstdio>
#include <cstdlib>

#include "sim86.h"

i32 main(i32 argc, char* argv[])
{
  FILE* file = nullptr;
  errno_t error = fopen_s(&file, argv[1], "rb"); // open file in binary mode
  if(error != 0)
  {
    fprintf(stderr, "Error: Could not open file\n");
    return 1;
  }

  // NOTE(Fabi): We use malloc as allocating the memory on the stack leads to a
  //             stack overflow.
  u8* memory = static_cast<u8*>(malloc(1024 * 1024));

  fseek(file, 0L, SEEK_END);
  i32 fileSize = ftell(file);
  rewind(file);

  size_t bytesRead = fread_s(memory, 1024 * 1024, sizeof(u8), fileSize, file);
  if(bytesRead != fileSize)
  {
    fprintf(stderr, "Error: Reading file\n");
    fclose(file);
    return 1;
  }

  // Close the file
  fclose(file);

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

  printf("bits 16\n");

  i32 index = 0;
  while(index < fileSize)
  {
    u8 instruction = memory[index];
    bool foundInstruction = false;

    for(i32 instructionIndex = 0; instructionIndex < instructionCount; instructionIndex++)
    {
      InstructionLiteral literal = instructions[instructionIndex];
      u8 opCode = instruction >> (8 - literal.Length);
      if(opCode == literal.Sequence)
      {
        foundInstruction = true;
        switch(literal.Code)
        {
        case OpCode::MoveRegisterOrMemoryToOrFromRegister: {
          u8 d = (memory[index] >> 1) & 1;
          u8 w = memory[index] & 1;
          u8 mod = (memory[index + 1] >> 6) & 0b11;
          u8 reg = (memory[index + 1] >> 3) & 0b111;
          u8 rm = memory[index + 1] & 0b111;

          index += 2;

          char buffer[64];
          const char* regNameA = regTable[reg][w];
          const char* regNameB = GetRegisterOrMemory(mod, rm, w, memory, &index, buffer);

          const char* dest = nullptr;
          const char* src = nullptr;

          if(d == 0)
          {
            src = regNameA;
            dest = regNameB;
          } else
          {
            src = regNameB;
            dest = regNameA;
          }

          printf("mov %s, %s\n", dest, src);
          break;
        }

        case OpCode::ImmediateToRegisterOrMemory: {
          u8 w = memory[index] & 1;
          u8 mod = (memory[index + 1] >> 6) & 0b11;
          u8 rm = (memory[index + 1]) & 0b111;
          index += 2;

          char buffer[64];
          const char* dest = GetRegisterOrMemory(mod, rm, w, memory, &index, buffer);

          i32 value = ReadValue(memory, &index, w);

          if(mod == 0b11)
          {
            printf("mov %s, %d\n", dest, value);
          } else
          {
            char valueBuffer[32];
            const char* format = w == 0 ? "byte %d" : "word %d";
            sprintf(valueBuffer, format, value);
            printf("mov %s, %s\n", dest, valueBuffer);
          }

          break;
        }

        case OpCode::MoveImmediateToRegister: {
          u8 w = (memory[index] >> 3) & 1;
          u8 reg = memory[index] & 0b111;
          index += 1;
          i32 value = ReadValue(memory, &index, w);

          char buffer[64];
          const char* dest = regTable[reg][w];
          sprintf(buffer, "%d", value);

          printf("mov %s, %s\n", dest, buffer);
          break;
        }

        case OpCode::MemoryToAccumulator: {
          u8 w = memory[index] & 1;
          index += 1;
          i32 address = ReadValue(memory, &index, w);
          printf("mov ax, [%d]\n", address);
          break;
        }

        case OpCode::AccumulatorToMemory: {
          u8 w = memory[index] & 1;
          index += 1;
          i32 address = ReadValue(memory, &index, w);
          printf("mov [%d], ax\n", address);
          break;
        }

        case OpCode::AddRegisterOrMemoryAndRegister: {
          u8 d = (memory[index] >> 1) & 1;
          u8 w = memory[index] & 1;
          u8 mod = (memory[index + 1] >> 6) & 0b11;
          u8 reg = (memory[index + 1] >> 3) & 0b111;
          u8 rm = memory[index + 1] & 0b111;

          index += 2;

          char buffer[64];
          const char* regNameA = regTable[reg][w];
          const char* regNameB = GetRegisterOrMemory(mod, rm, w, memory, &index, buffer);

          const char* dest = nullptr;
          const char* src = nullptr;

          if(d == 0)
          {
            src = regNameA;
            dest = regNameB;
          } else
          {
            src = regNameB;
            dest = regNameA;
          }

          printf("add %s, %s\n", dest, src);

          break;
        }

        case OpCode::AddImmediateToRegisterOrMemory:
        case OpCode::SubImmediateToRegisterOrMemory:
        case OpCode::CmpImmediateToRegisterOrMemory: {
          u8 s = (memory[index] >> 1) & 1;
          u8 w = memory[index] & 1;
          u8 operation = (memory[index + 1] >> 3) & 0b111;
          u8 mod = (memory[index + 1] >> 6) & 0b11;
          u8 rm = memory[index + 1] & 0b111;

          index += 2;

          char buffer[64];
          const char* op = nullptr;

          switch(operation)
          {
          case 0b000:
            op = "add";
            break;
          case 0b101:
            op = "sub";
            break;
          case 0b111:
            op = "cmp";
            break;
          }

          const char* dest = GetRegisterOrMemory(mod, rm, w, memory, &index, buffer);

          if(w == 0)
          {
            i32 val = ReadValue(memory, &index, 0);

            if(mod == 0b11)
            {
              printf("%s %s, %d\n", op, dest, (int8_t)val);
            } else
            {
              printf("%s byte %s, %d\n", op, dest, (int8_t)val);
            }
          } else
          {
            u8 size = s == 1 ? 0 : 1;
            i32 val = ReadValue(memory, &index, size);
            if(s == 1)
            {

              if((val & 0b10000000) == 0b10000000)
              {
                val |= 0b1111111100000000;
              }
            }

            if(mod == 0b11)
            {
              printf("%s %s, %d\n", op, dest, (int16_t)val);
            } else
            {
              printf("%s word %s, %d\n", op, dest, (int16_t)val);
            }
          }

          break;
        }

        case OpCode::AddImmediateToAccumulator: {
          u8 w = memory[index] & 1;
          index += 1;

          i32 value = ReadValue(memory, &index, w);

          if(w == 0)
          {
            printf("add al, %d\n", (int8_t)value);
          } else
          {
            printf("add ax, %d\n", (int16_t)value);
          }

          break;
        }

        case OpCode::SubRegisterOrMemoryAndRegister: {
          u8 d = (memory[index] >> 1) & 1;
          u8 w = memory[index] & 1;
          u8 mod = (memory[index + 1] >> 6) & 0b11;
          u8 reg = (memory[index + 1] >> 3) & 0b111;
          u8 rm = memory[index + 1] & 0b111;

          index += 2;

          char buffer[64];
          const char* regNameA = regTable[reg][w];
          const char* regNameB = GetRegisterOrMemory(mod, rm, w, memory, &index, buffer);

          const char* dest = nullptr;
          const char* src = nullptr;

          if(d == 0)
          {
            src = regNameA;
            dest = regNameB;
          } else
          {
            src = regNameB;
            dest = regNameA;
          }

          printf("sub %s, %s\n", dest, src);

          break;
        }

        case OpCode::SubImmediateToAccumulator: {
          u8 w = memory[index] & 1;
          index += 1;

          i32 value = ReadValue(memory, &index, w);

          if(w == 0)
          {
            printf("sub al, %d\n", (int8_t)value);
          } else
          {
            printf("sub ax, %d\n", (int16_t)value);
          }

          break;
        }

        case OpCode::CmpRegisterOrMemoryAndRegister: {
          u8 d = (memory[index] >> 1) & 1;
          u8 w = memory[index] & 1;
          u8 mod = (memory[index + 1] >> 6) & 0b11;
          u8 reg = (memory[index + 1] >> 3) & 0b111;
          u8 rm = memory[index + 1] & 0b111;

          index += 2;

          char buffer[64];
          const char* regNameA = regTable[reg][w];
          const char* regNameB = GetRegisterOrMemory(mod, rm, w, memory, &index, buffer);

          const char* dest = nullptr;
          const char* src = nullptr;

          if(d == 0)
          {
            src = regNameA;
            dest = regNameB;
          } else
          {
            src = regNameB;
            dest = regNameA;
          }

          printf("cmp %s, %s\n", dest, src);

          break;
        }

        case OpCode::CmpImmediateToAccumulator: {
          u8 w = memory[index] & 1;
          index += 1;

          i32 value = ReadValue(memory, &index, w);

          if(w == 0)
          {
            printf("cmp al, %d\n", (int8_t)value);
          } else
          {
            printf("cmp ax, %d\n", (int16_t)value);
          }

          break;
        }

        case OpCode::JumpOnEqual:
        case OpCode::JumpOnLess:
        case OpCode::JumpOnLessOrEqual:
        case OpCode::JumpOnBelow:
        case OpCode::JumpOnBelowOrEqual:
        case OpCode::JumpOnParity:
        case OpCode::JumpOnOverflow:
        case OpCode::JumpOnSign:
        case OpCode::JumpOnNotEqual:
        case OpCode::JumpOnNotLess:
        case OpCode::JumpOnNotLessOrEqual:
        case OpCode::JumpOnNotBelow:
        case OpCode::JumpOnNotBelowOrEqual:
        case OpCode::JumpOnNotParity:
        case OpCode::JumpOnNotOverflow:
        case OpCode::JumpOnNotSign:
        case OpCode::LoopCxTimes:
        case OpCode::LoopWhileZero:
        case OpCode::LoopWhileNotZero:
        case OpCode::JumpOnCxZero: {
          index += 1;
          int8_t offset = ReadValue(memory, &index, 0);

          // NOTE(Fabi): We have to offset by the instruction length
          //             because nasm does, subtract the instruction
          //             length from the offset if you use the '$' to
          //             to mark an offset
          offset += 2;

          const char* op = JumpOpCodeToMnemonic(literal.Code);

          if(offset >= 0)
          {
            printf("%s $+%d\n", op, offset);
          } else
          {
            printf("%s $%d\n", op, offset);
          }
          break;
        }
        }
        break;
      }
    }

    if(!foundInstruction)
    {
      fprintf(stderr, "Unknown instruction\n");
      return 1;
    }
  }

  return 0;
}

const char* JumpOpCodeToMnemonic(OpCode code)
{
  switch(code)
  {
  case OpCode::JumpOnEqual:
    return "je";
  case OpCode::JumpOnLess:
    return "jl";
  case OpCode::JumpOnLessOrEqual:
    return "jle";
  case OpCode::JumpOnBelow:
    return "jb";
  case OpCode::JumpOnBelowOrEqual:
    return "jbe";
  case OpCode::JumpOnParity:
    return "jp";
  case OpCode::JumpOnOverflow:
    return "jo";
  case OpCode::JumpOnSign:
    return "js";
  case OpCode::JumpOnNotEqual:
    return "jnz";
  case OpCode::JumpOnNotLess:
    return "jnl";
  case OpCode::JumpOnNotLessOrEqual:
    return "jnle";
  case OpCode::JumpOnNotBelow:
    return "jnb";
  case OpCode::JumpOnNotBelowOrEqual:
    return "jnbe";
  case OpCode::JumpOnNotParity:
    return "jnp";
  case OpCode::JumpOnNotOverflow:
    return "jno";
  case OpCode::JumpOnNotSign:
    return "jns";
  case OpCode::LoopCxTimes:
    return "loop";
  case OpCode::LoopWhileZero:
    return "loopz";
  case OpCode::LoopWhileNotZero:
    return "loopnz";
  case OpCode::JumpOnCxZero:
    return "jcxz";
  default:
    return "error";
  }
}

i32 ReadValue(u8* memory, i32* index, u8 w)
{
  i32 value = 0;
  if(w == 0)
  {
    value = memory[*index];
    *index += 1;
  } else
  {
    value = memory[*index + 1] << 8;
    value |= memory[*index];
    *index += 2;
  }

  return value;
}

const char* GetRegisterOrMemory(u8 mod, u8 rm, u8 w, u8* memory, i32* index, char* buffer)
{
  if(mod == 0b11)
  {
    return regTable[rm][w];
  } else
  {
    if(mod == 0)
    {
      if(rm == 0b110)
      {
        i32 address = ReadValue(memory, index, w);
        sprintf(buffer, "[%d]", address);
      } else
      {
        const char* address = effectiveAddressCalculationTable[rm];
        sprintf(buffer, "[%s]", address);
      }
    } else
    {
      u8 size = mod == 1 ? 0 : 1;
      i32 offset = ReadValue(memory, index, size);
      const char* address = effectiveAddressCalculationTable[rm];

      if(size == 0)
      {
        sprintf(buffer, "[%s + %d]", address, static_cast<int8_t>(offset));
      } else
      {
        sprintf(buffer, "[%s + %d]", address, offset);
      }
    }

    return buffer;
  }
}