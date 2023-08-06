#include "Timing.hpp"

#include <intrin.h>
#include <Windows.h>

u64 GetCpuTime()
{
    return __rdtsc();
}

u64 GetCpuFrequency()
{
    LARGE_INTEGER frequency = {};
    QueryPerformanceFrequency(&frequency);

    u64 start = GetCpuTime();

    LARGE_INTEGER performanceCounterStart = {};
    LARGE_INTEGER performanceCounterEnd = {};
    QueryPerformanceCounter(&performanceCounterStart);

    while((performanceCounterEnd.QuadPart - performanceCounterStart.QuadPart)
        < frequency.QuadPart)
    {
        QueryPerformanceCounter(&performanceCounterEnd);
    }

    u64 end = GetCpuTime();
    return end - start;
}
