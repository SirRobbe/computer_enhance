#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

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

  const char* mode = argv[2];
  bool isSimMode = strcmp(mode, SimulationOption) == 0;

  i16 registers[8] = {};
  bool zeroFlag = false;
  bool signedFlag = false;
  char flagString[3] = {'0', '0', '\0'};

  if(strcmp(mode, PrintOption) == 0)
  {
    printf("bits 16\n");
  }

  u16 instructionPointer = 0;
  while(instructionPointer < fileSize)
  {
    u8 instruction = memory[instructionPointer];
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
          u8 d = (memory[instructionPointer] >> 1) & 1;
          u8 w = memory[instructionPointer] & 1;
          u8 mod = (memory[instructionPointer + 1] >> 6) & 0b11;
          u8 reg = (memory[instructionPointer + 1] >> 3) & 0b111;
          u8 rm = memory[instructionPointer + 1] & 0b111;

          instructionPointer += 2;

          u16 oldIp = instructionPointer;
          u16 address = 0;

          char buffer[64];
          const char* regNameA = regTable[reg][w];
          const char* regNameB = GetRegisterOrMemory(mod, rm, w, memory, &instructionPointer, buffer);

          const char* dest = nullptr;
          const char* src = nullptr;

          if(d == 0)
          {
            src = regNameA;
            dest = regNameB;

            if(isSimMode)
            {
              i16 newValue = registers[reg];
              i16 oldValue = 0;

              if(mod == 0b11)
              {
                i16 oldValue = registers[rm];
              }
              else
              {
                if(mod == 0)
                {
                  switch(rm)
                  {
                  case 0b000: {
                    address = registers[3] + registers[6];
                    break;
                  }

                  case 0b001: {
                    address = registers[3] + registers[7];
                    break;
                  }

                  case 0b010: {
                    address = registers[5] + registers[6];
                    break;
                  }

                  case 0b011: {
                    address = registers[5] + registers[7];
                    break;
                  }

                  case 0b100: {
                    address = registers[6];
                    break;
                  }

                  case 0b101: {
                    address = registers[7];
                    break;
                  }

                  case 0b110: {
                    address = ReadValue(memory, &oldIp, w);
                    break;
                  }

                  case 0b111: {
                    address = registers[3];
                    break;
                  }
                  }

                  i8 low = memory[address];
                  i8 high = memory[address + 1];
                  oldValue = high;
                  oldValue = oldValue << 8;
                  oldValue = oldValue | low;
                }
                else
                {
                  u8 size = mod == 1 ? 0 : 1;
                  i16 offset = ReadValue(memory, &oldIp, size);
                  i16 baseAddress = registers[rm];
                  u16 address = baseAddress + offset;
                  i8 low = memory[address];
                  i8 high = memory[address + 1];
                  oldValue = high;
                  oldValue = oldValue << 8;
                  oldValue = oldValue | low;
                }
              }
              
              if(mod == 0b11)
              {
                registers[rm] = newValue;
              }
              else
              {
                memory[address] = newValue & 0xff;
                memory[address + 1] = (newValue & 0xff00) >> 8;
              }
              
              printf("mov %s, %s ; %s:0x%x->0x%x\n", dest, src, dest, oldValue, newValue);
              break;
            }
          } else
          {
            src = regNameB;
            dest = regNameA;

            if(isSimMode)
            {
              i16 newValue = 0;
              i16 oldValue = registers[reg];

              if(mod == 0b11)
              {
                i16 newValue = registers[rm];
              }
              else
              {
                if(mod == 0)
                {
                  if(rm == 0b110)
                  {
                    u16 address = ReadValue(memory, &oldIp, w);
                    i8 low = memory[address];
                    i8 high = memory[address + 1];
                    newValue = high;
                    newValue = newValue << 8;
                    newValue = newValue | low;
                  } 
                  else
                  {
                    assert(false);
                    // const char* address = effectiveAddressCalculationTable[rm];
                    // sprintf(buffer, "[%s]", address);
                  }
                } else
                {
                  u8 size = mod == 1 ? 0 : 1;
                  i16 offset = ReadValue(memory, &oldIp, size);
                  i16 baseAddress = registers[rm];
                  u16 address = baseAddress + offset;
                  i8 low = memory[address];
                  i8 high = memory[address + 1];
                  newValue = high;
                  newValue = newValue << 8;
                  newValue = newValue | low;
                }
              }

              registers[reg] = newValue;
              printf("mov %s, %s ; %s:0x%x->0x%x\n", dest, src, dest, oldValue, newValue);
              break;
            }
          }

          printf("mov %s, %s\n", dest, src);
          break;
        }

        case OpCode::ImmediateToRegisterOrMemory: {
          u8 w = memory[instructionPointer] & 1;
          u8 mod = (memory[instructionPointer + 1] >> 6) & 0b11;
          u8 rm = (memory[instructionPointer + 1]) & 0b111;
          instructionPointer += 2;

          u16 oldIp = instructionPointer;
          char buffer[64];
          const char* dest = GetRegisterOrMemory(mod, rm, w, memory, &instructionPointer, buffer);

          i32 value = ReadValue(memory, &instructionPointer, w);

          if(mod == 0b11)
          {
            if(isSimMode)
            {
              printf("mov %s, %d; %s:%d->%d\n", dest, value, dest, registers[rm], value);
              registers[rm] = value;
            }
            else
            {
              printf("mov %s, %d\n", dest, value);
            }
            
          } else
          {
            char valueBuffer[32];
            const char* format = w == 0 ? "byte %d" : "word %d";
            sprintf(valueBuffer, format, value);

            if(isSimMode)
            {
              i8 high = (value & 0xff00) >> 8;
              i8 low = value & 0xff;
              u16 address = 0;

              if(mod == 0)
              {
                if(rm == 0b110)
                {
                  address = ReadValue(memory, &oldIp, w);
                } else
                {
                  assert(false);
                  //const char* address = effectiveAddressCalculationTable[rm];
                  //sprintf(buffer, "[%s]", address);
                }
              } else
              {
                u8 size = mod == 1 ? 0 : 1;
                i16 offset = ReadValue(memory, &oldIp, size);
                i16 baseAddress = 0;
                if(rm == 0b111)
                {
                  baseAddress = registers[3];
                }
                address = baseAddress + offset;                
              }

              memory[address] = low;
              memory[address + 1] = high;

              printf("mov %s, %s ; ->%d\n", dest, valueBuffer, value);
            } else
            {
              printf("mov %s, %s\n", dest, valueBuffer);
            }
          }

          break;
        }

        case OpCode::MoveImmediateToRegister: {
          u8 w = (memory[instructionPointer] >> 3) & 1;
          u8 reg = memory[instructionPointer] & 0b111;
          instructionPointer += 1;
          i32 value = ReadValue(memory, &instructionPointer, w);

          char buffer[64];
          const char* dest = regTable[reg][w];
          sprintf(buffer, "%d", value);

          if(isSimMode)
          {
            i16 oldValue = registers[reg];
            registers[reg] = value;
            printf("mov %s, %d ; %s:0x%x->0x%x\n", dest, value, dest, oldValue, value);
              
          }
          else
          {
            printf("mov %s, %s\n", dest, buffer);
          }

          break;
        }

        case OpCode::MemoryToAccumulator: {
          u8 w = memory[instructionPointer] & 1;
          instructionPointer += 1;
          i32 address = ReadValue(memory, &instructionPointer, w);
          printf("mov ax, [%d]\n", address);
          break;
        }

        case OpCode::AccumulatorToMemory: {
          u8 w = memory[instructionPointer] & 1;
          instructionPointer += 1;
          i32 address = ReadValue(memory, &instructionPointer, w);
          printf("mov [%d], ax\n", address);
          break;
        }

        case OpCode::AddRegisterOrMemoryAndRegister:
        case OpCode::SubRegisterOrMemoryAndRegister:
        case OpCode::CmpRegisterOrMemoryAndRegister: {
          u8 operation = (memory[instructionPointer] >> 3) & 0b111;
          u8 d = (memory[instructionPointer] >> 1) & 1;
          u8 w = memory[instructionPointer] & 1;
          u8 mod = (memory[instructionPointer + 1] >> 6) & 0b11;
          u8 reg = (memory[instructionPointer + 1] >> 3) & 0b111;
          u8 rm = memory[instructionPointer + 1] & 0b111;

          instructionPointer += 2;

          char buffer[64];
          const char* regNameA = regTable[reg][w];
          const char* regNameB = GetRegisterOrMemory(mod, rm, w, memory, &instructionPointer, buffer);

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

          const char* dest = nullptr;
          const char* src = nullptr;

          if(d == 0)
          {
            src = regNameA;
            dest = regNameB;

            if(isSimMode)
            {
              char oldFlags[3] = {flagString[0], flagString[1], flagString[2]};
              i16 a = registers[rm];
              i16 b = registers[reg];
              i16 result = 0;

              if(operation == 0b000)
              {
                result = a + b;
                registers[rm] = result;

                zeroFlag = result == 0 ? true : false;
                signedFlag = result < 0 ? true : false;

                flagString[0] = zeroFlag ? 'Z' : '0';
                flagString[1] = signedFlag ? 'S' : '0';

                printf("%s %s, %s ; %s:0x%x->0x%x flags:%s->%s\n", op, dest, src, dest, a, result, oldFlags, flagString);
                break;
              }
              else
              {
                result = a - b;

                if(operation == 0b101)
                {
                  zeroFlag = result == 0 ? true : false;
                  signedFlag = result < 0 ? true : false;
                  flagString[0] = zeroFlag ? 'Z' : '0';
                  flagString[1] = signedFlag ? 'S' : '0';

                  registers[rm] = result;
                  printf("%s %s, %s ; %s:0x%x->0x%x flags:%s->%s\n", op, dest, src, dest, a, result, oldFlags, flagString);
                  break;
                }
                else
                {
                  zeroFlag = result == 0 ? true : false;
                  flagString[0] = zeroFlag ? 'Z' : '0';
                  flagString[1] = '0';
                  printf("%s %s, %s ; flags:%s->%s\n", op, dest, src, oldFlags, flagString);
                  break;
                }
              }
            }

          } else
          {
            src = regNameB;
            dest = regNameA;

            if(isSimMode)
            {
              char oldFlags[3] = {flagString[0], flagString[1], flagString[2]};
              i16 a = registers[reg];
              i16 b = registers[rm];
              i16 result = 0;

              if(operation == 0b000)
              {
                result = a + b;
                registers[reg] = result;

                zeroFlag = result == 0 ? true : false;
                signedFlag = result < 0 ? true : false;

                flagString[0] = zeroFlag ? 'Z' : '0';
                flagString[1] = signedFlag ? 'S' : '0';

                printf("%s %s, %s ; %s:0x%x->0x%x flags:%s->%s\n", op, dest, src, dest, a, result, oldFlags, flagString);
                break;
              }
              else
              {
                result = a - b;

                if(operation == 0b101)
                {
                  zeroFlag = result == 0 ? true : false;
                  signedFlag = result < 0 ? true : false;
                  flagString[0] = zeroFlag ? 'Z' : '0';
                  flagString[1] = signedFlag ? 'S' : '0';

                  registers[rm] = result;
                  printf("%s %s, %s ; %s:0x%x->0x%x flags:%s->%s\n", op, dest, src, dest, a, result, oldFlags, flagString);
                  break;
                }
                else
                {
                  zeroFlag = result == 0 ? true : false;
                  flagString[0] = zeroFlag ? 'Z' : '0';
                  flagString[1] = '0';
                  printf("%s %s, %s ; flags:%s->%s\n", op, dest, src, oldFlags, flagString);
                  break;
                }
              }
            }
          }

          printf("%s %s, %s\n", op, dest, src);

          break;
        }

        case OpCode::AddImmediateToRegisterOrMemory:
        case OpCode::SubImmediateToRegisterOrMemory:
        case OpCode::CmpImmediateToRegisterOrMemory: {
          u8 s = (memory[instructionPointer] >> 1) & 1;
          u8 w = memory[instructionPointer] & 1;
          u8 operation = (memory[instructionPointer + 1] >> 3) & 0b111;
          u8 mod = (memory[instructionPointer + 1] >> 6) & 0b11;
          u8 rm = memory[instructionPointer + 1] & 0b111;

          instructionPointer += 2;

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

          const char* dest = GetRegisterOrMemory(mod, rm, w, memory, &instructionPointer, buffer);

          if(w == 0)
          {
            i32 val = ReadValue(memory, &instructionPointer, 0);

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
            i32 val = ReadValue(memory, &instructionPointer, size);
            if(s == 1)
            {

              if((val & 0b10000000) == 0b10000000)
              {
                val |= 0b1111111100000000;
              }
            }

            if(mod == 0b11)
            {
              if(isSimMode)
              {
                 char oldFlags[3] = {flagString[0], flagString[1], flagString[2]};
                i16 a = registers[rm];
                i16 b = val;

                i16 result = 0;

                if(operation == 0b000)
                {
                  result = a + b;
                  registers[rm] = result;

                  zeroFlag = result == 0 ? true : false;
                  signedFlag = result < 0 ? true : false;

                  flagString[0] = zeroFlag ? 'Z' : '0';
                  flagString[1] = signedFlag ? 'S' : '0';

                  printf("%s %s, %d ; %s:0x%x->0x%x flags:%s->%s\n", op, dest, b, dest, a, result, oldFlags, flagString);
                  break;
                } else
                {
                  result = a - b;

                  if(operation == 0b101)
                  {
                    zeroFlag = result == 0 ? true : false;
                    signedFlag = result < 0 ? true : false;
                    flagString[0] = zeroFlag ? 'Z' : '0';
                    flagString[1] = signedFlag ? 'S' : '0';

                    registers[rm] = result;
                    printf("%s %s, %d ; %s:0x%x->0x%x flags:%s->%s\n",
                           op,
                           dest,
                           b,
                           dest,
                           a,
                           result,
                           oldFlags,
                           flagString);
                    break;
                  } else
                  {
                    zeroFlag = result == 0 ? true : false;
                    flagString[0] = zeroFlag ? 'Z' : '0';
                    flagString[1] = '0';
                    printf("%s %s, %d ; flags:%s->%s\n", op, dest, b, oldFlags, flagString);
                    break;
                  }
                }
              }

              printf("%s %s, %d\n", op, dest, (int16_t)val);
            } else
            {
              printf("%s word %s, %d\n", op, dest, (int16_t)val);
            }
          }

          break;
        }

        case OpCode::AddImmediateToAccumulator: {
          u8 w = memory[instructionPointer] & 1;
          instructionPointer += 1;

          i32 value = ReadValue(memory, &instructionPointer, w);

          if(w == 0)
          {
            printf("add al, %d\n", (int8_t)value);
          } else
          {
            printf("add ax, %d\n", (int16_t)value);
          }

          break;
        }

        case OpCode::SubImmediateToAccumulator: {
          u8 w = memory[instructionPointer] & 1;
          instructionPointer += 1;

          i32 value = ReadValue(memory, &instructionPointer, w);

          if(w == 0)
          {
            printf("sub al, %d\n", (int8_t)value);
          } else
          {
            printf("sub ax, %d\n", (int16_t)value);
          }

          break;
        }

        case OpCode::CmpImmediateToAccumulator: {
          u8 w = memory[instructionPointer] & 1;
          instructionPointer += 1;

          i32 value = ReadValue(memory, &instructionPointer, w);

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
          instructionPointer += 1;
          int8_t offset = ReadValue(memory, &instructionPointer, 0);
          const char* op = JumpOpCodeToMnemonic(literal.Code);

          if(literal.Code == OpCode::JumpOnNotEqual && isSimMode)
          {
            u16 oldInstructionPointer = instructionPointer;
            if(!zeroFlag)
            {
              instructionPointer += offset;
            }

            printf("%s $%d ; ip:0x%x->0x%x\n", op, offset + 2, oldInstructionPointer, oldInstructionPointer);
          } else
          {
            offset += 2;
            if(offset >= 0)
            {
              // NOTE(Fabi): We have to offset by the instruction length
              //             because nasm does, subtract the instruction
              //             length from the offset if you use the '$' to
              //             to mark an offset
              printf("%s $+%d\n", op, offset);
            } else
            {
              // NOTE(Fabi): We have to offset by the instruction length
              //             because nasm does, subtract the instruction
              //             length from the offset if you use the '$' to
              //             to mark an offset
              printf("%s $%d\n", op, offset);
            }
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

  if(isSimMode)
  {
    printf("\nFinal registers:\n");
    for(i32 i = 0; i < 8; i++)
    {
      printf("\t%s: 0x%0*x (%d)\n", regTable[i][1], 4, registers[i], registers[i]);
    }

    printf("\tip: 0x%0*x (%d)\n", 4, instructionPointer, instructionPointer);
    printf("\tflags:%s\n", flagString);
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

i32 ReadValue(u8* memory, u16* index, u8 w)
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

const char* GetRegisterOrMemory(u8 mod, u8 rm, u8 w, u8* memory, u16* index, char* buffer)
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