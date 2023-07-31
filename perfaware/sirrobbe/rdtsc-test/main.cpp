#include "Types.h"
#include "Windows.h"

i32 main(i32 argc, char* argv[])
{
  LARGE_INTEGER counter = {};
  QueryPerformanceCounter(&counter);

  return counter.LowPart;
}
