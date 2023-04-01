#include <cstdint>
#include <cstdio>
#include <cstdlib>

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
    uint8_t Sequence = 0;
    int32_t Length = 0;
};

const char* JumpOpCodeToMnemonic(OpCode code);
int32_t ReadValue(uint8_t *memory, int32_t *index, uint8_t w);
const char* GetRegisterOrMemory(uint8_t mod, uint8_t rm, uint8_t w,
                                uint8_t* memory,
                                int32_t* index,
                                char* buffer);

const char *regTable[][2] =
{
    {"al", "ax"},
    {"cl", "cx"},
    {"dl", "dx"},
    {"bl", "bx"},
    {"ah", "sp"},
    {"ch", "bp"},
    {"dh", "si"},
    {"bh", "di"},
};

const char *effectiveAddressCalculationTable[] =
{
    "bx + si",
    "bx + di",
    "bp + si",
    "bp + di",
    "si",
    "di",
    "bp",
    "bx",
};

int32_t main(int32_t argc, char *argv[])
{
    FILE *file = nullptr;
    errno_t error = fopen_s(&file, argv[1], "rb"); // open file in binary mode
    if (error != 0)
    {
        fprintf(stderr, "Error: Could not open file\n");
        return 1;
    }

    // NOTE(Fabi): We use malloc as allocating the memory on the stack leads to a
    //             stack overflow.
    uint8_t *memory = static_cast<uint8_t *>(malloc(1024 * 1024));

    fseek(file, 0L, SEEK_END);
    int32_t fileSize = ftell(file);
    rewind(file);

    size_t bytesRead = fread_s(memory, 1024 * 1024, sizeof(uint8_t), fileSize, file);
    if (bytesRead != fileSize)
    {
        fprintf(stderr, "Error: Reading file\n");
        fclose(file);
        return 1;
    }

    // Close the file
    fclose(file);

    InstructionLiteral instructions[] =
        {
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

    int32_t instructionCount = sizeof(instructions) / sizeof(InstructionLiteral);

    printf("bits 16\n");

    int32_t index = 0;
    while (index < fileSize)
    {
        uint8_t instruction = memory[index];
        bool foundInstruction = false;

        for (int32_t instructionIndex = 0; instructionIndex < instructionCount; instructionIndex++)
        {
            InstructionLiteral literal = instructions[instructionIndex];
            uint8_t opCode = instruction >> (8 - literal.Length);
            if (opCode == literal.Sequence)
            {
                foundInstruction = true;
                switch (literal.Code)
                {
                case OpCode::MoveRegisterOrMemoryToOrFromRegister:
                {
                    uint8_t d = (memory[index] >> 1) & 1;
                    uint8_t w = memory[index] & 1;
                    uint8_t mod = (memory[index + 1] >> 6) & 0b11;
                    uint8_t reg = (memory[index + 1] >> 3) & 0b111;
                    uint8_t rm = memory[index + 1] & 0b111;

                    index += 2;

                    char buffer[64];
                    const char* regNameA = regTable[reg][w];                    
                    const char* regNameB = GetRegisterOrMemory(mod, rm, w, memory,
                                                               &index,
                                                               buffer);

                    const char* dest = nullptr;
                    const char* src = nullptr;

                    if(d == 0)
                    {
                        src = regNameA;
                        dest = regNameB;
                    }
                    else
                    {
                        src = regNameB;
                        dest = regNameA;
                    }

                    printf("mov %s, %s\n", dest, src);
                    break;
                }

                case OpCode::ImmediateToRegisterOrMemory:
                {
                    uint8_t w = memory[index] & 1;
                    uint8_t mod = (memory[index + 1] >> 6) & 0b11;
                    uint8_t rm = (memory[index + 1]) & 0b111;
                    index += 2;

                    char buffer[64];
                    const char* dest = GetRegisterOrMemory(mod, rm, w, memory, &index,
                                                           buffer);

                    int32_t value = ReadValue(memory, &index, w);

                    if (mod == 0b11)
                    {
                        printf("mov %s, %d\n", dest, value);
                    }
                    else
                    {
                        char valueBuffer[32];
                        const char *format = w == 0 ? "byte %d" : "word %d";
                        sprintf(valueBuffer, format, value);
                        printf("mov %s, %s\n", dest, valueBuffer);
                    }

                    break;
                }

                case OpCode::MoveImmediateToRegister:
                {
                    uint8_t w = (memory[index] >> 3) & 1;
                    uint8_t reg = memory[index] & 0b111;
                    index += 1;
                    int32_t value = ReadValue(memory, &index, w);

                    char buffer[64];
                    const char *dest = regTable[reg][w];
                    sprintf(buffer, "%d", value);

                    printf("mov %s, %s\n", dest, buffer);
                    break;
                }

                case OpCode::MemoryToAccumulator:
                {
                    uint8_t w = memory[index] & 1;
                    index += 1;
                    int32_t address = ReadValue(memory, &index, w);
                    printf("mov ax, [%d]\n", address);
                    break;
                }

                case OpCode::AccumulatorToMemory:
                {
                    uint8_t w = memory[index] & 1;
                    index += 1;
                    int32_t address = ReadValue(memory, &index, w);
                    printf("mov [%d], ax\n", address);
                    break;
                }

                case OpCode::AddRegisterOrMemoryAndRegister:
                {
                    uint8_t d = (memory[index] >> 1) & 1;
                    uint8_t w = memory[index] & 1;
                    uint8_t mod = (memory[index + 1] >> 6) & 0b11;
                    uint8_t reg = (memory[index + 1] >> 3) & 0b111;
                    uint8_t rm = memory[index + 1] & 0b111;

                    index += 2;

                    char buffer[64];
                    const char* regNameA = regTable[reg][w];                    
                    const char* regNameB = GetRegisterOrMemory(mod, rm, w, memory,
                                                               &index,
                                                               buffer);

                    const char* dest = nullptr;
                    const char* src = nullptr;

                    if(d == 0)
                    {
                        src = regNameA;
                        dest = regNameB;
                    }
                    else
                    {
                        src = regNameB;
                        dest = regNameA;
                    }

                    printf("add %s, %s\n", dest, src);

                    break;
                }

                case OpCode::AddImmediateToRegisterOrMemory:
                case OpCode::SubImmediateToRegisterOrMemory:
                case OpCode::CmpImmediateToRegisterOrMemory:
                {
                    uint8_t s = (memory[index] >> 1) & 1;
                    uint8_t w = memory[index] & 1;
                    uint8_t operation = (memory[index + 1] >> 3) & 0b111;
                    uint8_t mod = (memory[index + 1] >> 6) & 0b11;
                    uint8_t rm = memory[index + 1] & 0b111;

                    index += 2;

                    char buffer[64];   
                    const char* op = nullptr;

                    switch(operation)
                    {
                        case 0b000: op = "add"; break;
                        case 0b101: op = "sub"; break;
                        case 0b111: op = "cmp"; break;
                    }

                    const char* dest = GetRegisterOrMemory(mod, rm, w, memory,
                                                           &index,
                                                           buffer);

                    if(w == 0)
                    {
                        int32_t val = ReadValue(memory, &index, 0);
                        
                        if(mod == 0b11)
                        {
                            printf("%s %s, %d\n", op, dest, (int8_t)val);
                        }
                        else
                        {
                            printf("%s byte %s, %d\n", op, dest, (int8_t)val);
                        }
                    }
                    else
                    {
                        uint8_t size = s == 1 ? 0 : 1;
                        int32_t val = ReadValue(memory, &index, size);
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
                        }
                        else
                        {
                            printf("%s word %s, %d\n", op, dest, (int16_t)val);
                        }
                    }

                    break;
                }

                case OpCode::AddImmediateToAccumulator:
                {
                    uint8_t w = memory[index] & 1;
                    index += 1;

                    int32_t value = ReadValue(memory, &index, w);

                    if(w == 0)
                    {
                        printf("add al, %d\n", (int8_t)value);
                    }
                    else
                    {
                        printf("add ax, %d\n", (int16_t)value);
                    }

                    break;
                }

                case OpCode::SubRegisterOrMemoryAndRegister:
                {
                    uint8_t d = (memory[index] >> 1) & 1;
                    uint8_t w = memory[index] & 1;
                    uint8_t mod = (memory[index + 1] >> 6) & 0b11;
                    uint8_t reg = (memory[index + 1] >> 3) & 0b111;
                    uint8_t rm = memory[index + 1] & 0b111;

                    index += 2;

                    char buffer[64];
                    const char* regNameA = regTable[reg][w];                    
                    const char* regNameB = GetRegisterOrMemory(mod, rm, w, memory,
                                                               &index,
                                                               buffer);

                    const char* dest = nullptr;
                    const char* src = nullptr;

                    if(d == 0)
                    {
                        src = regNameA;
                        dest = regNameB;
                    }
                    else
                    {
                        src = regNameB;
                        dest = regNameA;
                    }

                    printf("sub %s, %s\n", dest, src);

                    break;
                }

                case OpCode::SubImmediateToAccumulator:
                {
                    uint8_t w = memory[index] & 1;
                    index += 1;

                    int32_t value = ReadValue(memory, &index, w);

                    if(w == 0)
                    {
                        printf("sub al, %d\n", (int8_t)value);
                    }
                    else
                    {
                        printf("sub ax, %d\n", (int16_t)value);
                    }

                    break;
                }

                case OpCode::CmpRegisterOrMemoryAndRegister:
                {
                    uint8_t d = (memory[index] >> 1) & 1;
                    uint8_t w = memory[index] & 1;
                    uint8_t mod = (memory[index + 1] >> 6) & 0b11;
                    uint8_t reg = (memory[index + 1] >> 3) & 0b111;
                    uint8_t rm = memory[index + 1] & 0b111;

                    index += 2;

                    char buffer[64];
                    const char* regNameA = regTable[reg][w];                    
                    const char* regNameB = GetRegisterOrMemory(mod, rm, w, memory,
                                                               &index,
                                                               buffer);

                    const char* dest = nullptr;
                    const char* src = nullptr;

                    if(d == 0)
                    {
                        src = regNameA;
                        dest = regNameB;
                    }
                    else
                    {
                        src = regNameB;
                        dest = regNameA;
                    }

                    printf("cmp %s, %s\n", dest, src);

                    break;
                }

                case OpCode::CmpImmediateToAccumulator:
                {
                    uint8_t w = memory[index] & 1;
                    index += 1;

                    int32_t value = ReadValue(memory, &index, w);

                    if(w == 0)
                    {
                        printf("cmp al, %d\n", (int8_t)value);
                    }
                    else
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
                case OpCode::JumpOnCxZero:
                {
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
                    }
                    else
                    {
                        printf("%s $%d\n", op, offset);    
                    }
                    break;
                }

                }
                break;
            }
        }

        if (!foundInstruction)
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
        case OpCode::JumpOnEqual: return "je";
        case OpCode::JumpOnLess: return "jl";
        case OpCode::JumpOnLessOrEqual: return "jle";
        case OpCode::JumpOnBelow: return "jb";
        case OpCode::JumpOnBelowOrEqual: return "jbe";
        case OpCode::JumpOnParity: return "jp";
        case OpCode::JumpOnOverflow: return "jo";
        case OpCode::JumpOnSign: return "js";
        case OpCode::JumpOnNotEqual: return "jnz";
        case OpCode::JumpOnNotLess: return "jnl";
        case OpCode::JumpOnNotLessOrEqual: return "jnle";
        case OpCode::JumpOnNotBelow: return "jnb";
        case OpCode::JumpOnNotBelowOrEqual: return "jnbe";
        case OpCode::JumpOnNotParity: return "jnp";
        case OpCode::JumpOnNotOverflow: return "jno";
        case OpCode::JumpOnNotSign: return "jns";
        case OpCode::LoopCxTimes: return "loop";
        case OpCode::LoopWhileZero: return "loopz";
        case OpCode::LoopWhileNotZero: return "loopnz";
        case OpCode::JumpOnCxZero: return "jcxz";
        default: return "error";
    }
}

int32_t ReadValue(uint8_t *memory, int32_t *index, uint8_t w)
{
    int32_t value = 0;
    if (w == 0)
    {
        value = memory[*index];
        *index += 1;
    }
    else
    {
        value = memory[*index + 1] << 8;
        value |= memory[*index];
        *index += 2;
    }

    return value;
}

const char *GetRegisterOrMemory(uint8_t mod, uint8_t rm, uint8_t w,
                                uint8_t* memory,
                                int32_t* index,
                                char* buffer)
{
    if(mod == 0b11)
    {
        return regTable[rm][w];
    }
    else
    {
        if(mod == 0)
        {
            if(rm == 0b110)
            {
                int32_t address = ReadValue(memory, index, w);
                sprintf(buffer, "[%d]", address);
            }
            else
            {
                const char *address = effectiveAddressCalculationTable[rm];
                sprintf(buffer, "[%s]", address);
            }
        }
        else
        {
            uint8_t size = mod == 1 ? 0 : 1;
            int32_t offset = ReadValue(memory, index, size);
            const char *address = effectiveAddressCalculationTable[rm];

            if (size == 0)
            {
                sprintf(buffer, "[%s + %d]", address,
                        static_cast<int8_t>(offset));
            }
            else
            {
                sprintf(buffer, "[%s + %d]", address, offset);
            }
        }

        return buffer;
    }
}