#include <cstdint>
#include <cstdio>
#include <cstdlib>

enum struct OpCode
{
    MoveRegisterOrMemoryToOrFromRegister,

    Count = 0,
};

struct InstructionLiteral
{
    OpCode Code = OpCode::Count;
    uint8_t Sequence = 0;
    int32_t Length = 0;
};

int32_t main(int32_t argc, char* argv[])
{    
    FILE* file = nullptr;
    errno_t error = fopen_s(&file, argv[1], "rb");  // open file in binary mode
    if(error != 0)
    {
        fprintf(stderr, "Error: Could not open file\n");
        return 1;
    }

    // NOTE(Fabi): We use malloc as allocating the memory on the stack leads to a
    //             stack overflow.
    uint8_t* memory = static_cast<uint8_t*>(malloc(1024 * 1024));

    fseek(file, 0L, SEEK_END);
    int32_t fileSize = ftell(file);
    rewind(file);

    size_t bytesRead = fread_s(memory, 1024 * 1024, sizeof(uint8_t), fileSize, file);
    if(bytesRead != fileSize)
    {
        fprintf(stderr, "Error: Reading file\n");
        fclose(file);
        return 1;
    }

    // Close the file
    fclose(file);

    const char* regTable[][2] =
    {
        { "al", "ax" },
        { "cl", "cx"},
        { "dl", "dx" },
        { "bl", "bx" },
        { "ah", "sp" },
        { "ch", "bp" },
        { "dh", "si" },
        { "bh", "di" },
    };

    const char* effectiveAddressCalculationTable[] = 
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

    InstructionLiteral instructions[] =
    {
        { OpCode::MoveRegisterOrMemoryToOrFromRegister, 0b100010, 6 },
    };

    int32_t instructionCount = sizeof(instructions) / sizeof(InstructionLiteral);

    printf("bits 16\n");

    int32_t index = 0;
    while(index < fileSize)
    {
        uint8_t instruction = memory[index];
        for(int32_t instructionIndex = 0; instructionIndex < instructionCount; instructionIndex++)
        {
            InstructionLiteral literal = instructions[instructionIndex];
            uint8_t opCode = instruction >> (8 - literal.Length);
            if(opCode == literal.Sequence)
            {
                switch(literal.Code)
                {
                    case OpCode::MoveRegisterOrMemoryToOrFromRegister:
                    {
                        uint8_t d = (memory[index] >> 1) & 1;
                        uint8_t w = memory[index] & 1;
                        uint8_t mod = (memory[index + 1] >> 6) & 0b11;
                        uint8_t reg = (memory[index + 1] >> 3) & 0b111;
                        uint8_t rm = memory[index + 1] & 0b111;

                        index += 2;

                        const char* regNameA = regTable[reg][w];
                        const char* regNameB = nullptr;
                        char buffer[64];

                        if(mod == 0b11)
                        {
                            regNameB = regTable[rm][w];
                        }
                        else
                        {
                            if(mod == 0)
                            {
                                if(rm == 0b110)
                                {
                                    int32_t address = 0;
                                    if(w == 0)
                                    {
                                        address = memory[index];
                                        index += 1;
                                    }
                                    else
                                    {
                                        address = memory[index] << 8;
                                        address |= memory[index + 1];
                                        index += 2;
                                    }

                                    sprintf(buffer, "[%d]", address);
                                }
                                else
                                {
                                    const char* address = effectiveAddressCalculationTable[rm];
                                    sprintf(buffer, "[%s]", address);
                                }
                            }
                            else
                            {
                                int32_t offset = 0;
                                if(w == 0)
                                {
                                    offset = memory[index];
                                    index += 1;
                                }
                                else
                                {
                                    offset = memory[index] << 8;
                                    offset |= memory[index + 1];
                                    index += 2;
                                }
                                const char* address = effectiveAddressCalculationTable[rm];
                                sprintf(buffer, "[%s + %d]", address, offset);
                            }

                            regNameB = buffer;
                        }

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
                }
                break;
            }           
        }
    }

    return 0;
}