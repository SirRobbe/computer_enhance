#include <vector>
#include "Profiling.h"
#include "Timing.hpp"

namespace Profiling
{
    std::vector<BlockStats> Blocks;
    u64 SessionStart = 0;
    u64 SessionEnd = 0;
    u64 SessionLength = 0;

    static void Begin()
    {
        Blocks.clear();
        SessionStart = GetCpuTime();
    }

    static void End()
    {
        SessionEnd = GetCpuTime();
        SessionLength = SessionEnd - SessionStart;
    }

    static void PrintBlocks()
    {
        f64 cpuFrequency = static_cast<f64>(GetCpuFrequency());
        f64 speedInGHz = cpuFrequency / (1000 * 1000 * 1000);
        f64 totalMs = (static_cast<f64>(SessionLength) / cpuFrequency) * 1000;
        printf("\nTotal time: %fms (%fGHz)\n", totalMs, speedInGHz);

        for(i32 index = 0; index < Blocks.size(); index++)
        {
            auto block = Blocks[index];
            u64 delta = block.End - block.Start;
            f64 percentage = (static_cast<f64>(delta) / static_cast<f64>(SessionLength)) * 100.0;
            printf("Block '%s': %lld (%.2f%%)\n", block.Name.c_str(), delta, percentage);
        }  
    }
}

BlockProfiler::BlockProfiler(std::string name) : 
  Start(GetCpuTime()),
  End(0),
  Name(name)
{}

BlockProfiler::~BlockProfiler()
{
    End = GetCpuTime();
    Profiling::Blocks.push_back({Start, End, Name});
}
