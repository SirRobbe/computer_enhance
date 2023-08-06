#pragma once

#include <string>
#include "Types.h"

namespace Profiling
{
    static void Begin();
    static void End();
    static void PrintBlocks();
} 

struct BlockStats
{
    u64 Start;
    u64 End;
    std::string Name;
};

class BlockProfiler
{
    public:
    BlockProfiler(std::string name);
    ~BlockProfiler();

    u64 Start;
    u64 End;
    std::string Name;
};

