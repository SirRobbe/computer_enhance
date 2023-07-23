#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <stdint.h>
#include <string.h>

#include "Json.h"

typedef int32_t i32;

static f64 DegToRad(f64 deg);
static f64 Square(f64 x);
static f64 Haversine(f64 x0, f64 y0, f64 x1, f64 y1, f64 radius);

static char* ReadFile(const char* name, bool isBinary);

static u64 s_RngState = 0;
static f64 UniformRange(f64 min, f64 max);

constexpr const char* GenerateCommand = "generate";
constexpr const char* ComputeCommand = "compute";
constexpr const char* UniformMode = "uniform";


i32 main(i32 argc, char* argv[])
{
  const char* command = argv[1];

  if(strcmp(command, GenerateCommand) == 0)
  {
    i32 pairs = atoi(argv[2]);
    s_RngState = strtoull(argv[3], nullptr, 10);
    const char* mode = argv[4];

    printf("Running generator:\nNumber of pairs: %d\nSeed: %lld\nMode: %s\n",
           pairs,
           s_RngState,
           mode);

    FILE* dataFile = fopen("data.json", "w");
    FILE* resultsFile = fopen("results.bin", "wb");
    f64 sum = 0.0;

    fprintf(dataFile, "{\n\t\"pairs\" :\n\t[\n");

    if(strcmp(mode, UniformMode) == 0)
    {
      for(i32 index = 0; index < pairs; index++)
      {
        f64 x0 = UniformRange(-180.0, 180.0);
        f64 y0 = UniformRange(-90.0, 90.0);
        f64 x1 = UniformRange(-180.0, 180.0);
        f64 y1 = UniformRange(-90.0, 90.0);

        // NOTE(Fabi): To be conform with json we are not allowed to have a trailing comma.
        if(index == pairs - 1)
        {
          fprintf(dataFile, "\t\t{ \"x0\" : %.17f, \"y0\" : %.17f, \"x1\" : %.17f, \"y1\" : %.17f }\n",
                x0,
                y0,
                x1,
                y1);
        }
        else
        {
          fprintf(dataFile, "\t\t{ \"x0\" : %.17f, \"y0\" : %.17f, \"x1\" : %.17f, \"y1\" : %.17f },\n",
                x0,
                y0,
                x1,
                y1);
        }

        f64 distance = Haversine(x0, y0, x1, y1, 6372.8);
        sum += distance;
        fwrite(&distance, sizeof(f64), 1, resultsFile);
      }
    }

    fprintf(dataFile, "\t]\n}\n");
    fclose(dataFile);

    f64 result = sum / pairs;
    printf("Result: %f\n", result);

    fwrite(&result, sizeof(f64), 1, resultsFile);
    fclose(resultsFile);
  }
  else if(strcmp(command, ComputeCommand) == 0)
  {
    printf("Compute dataset\n");

    char* data = ReadFile("data.json", false);
    printf("Read data\n");

    f64* results = reinterpret_cast<f64*>(ReadFile("results.bin", true));
    printf("Read results\n");

    auto dataAsString = String(data);
    auto json = ParseJson(&dataAsString);
    free(data);

    f32 sum = 0.f;
    
    auto pairsKey = ScopedString(const_cast<char*>("pairs"));
    auto x0Key = ScopedString(const_cast<char*>("x0"));
    auto y0Key = ScopedString(const_cast<char*>("y0"));
    auto x1Key = ScopedString(const_cast<char*>("x1"));
    auto y1Key = ScopedString(const_cast<char*>("y1"));

    auto pairs = json[pairsKey].Array;

    for(i64 index = 0; index < pairs.Count(); index++)
    {
      auto pair = pairs[index].Object;
      f32 x0 = pair[x0Key].Number;
      f32 y0 = pair[y0Key].Number;
      f32 x1 = pair[x1Key].Number;
      f32 y1 = pair[y1Key].Number;

      f32 distance = Haversine(x0, y0, x1, y1, 6372.8f);
      f32 expected = results[index];
      
      sum += distance;
    }

    f64 average = sum / pairs.Count();
    f64 expectedResult = results[pairs.Count()];
    printf("result: %f\n", average);
    printf("expected: %f\n", expectedResult);

    free(results);
  }

  return 0;
}

f64 DegToRad(f64 deg)
{
  f64 result = 0.01745329251994329577 * deg;
  return result;
}

f64 Square(f64 x)
{
  f64 result = x * x;
  return result;
}

f64 Haversine(f64 x0, f64 y0, f64 x1, f64 y1, f64 radius)
{
  f64 lat1 = y0;
  f64 lat2 = y1;
  f64 lon1 = x0;
  f64 lon2 = x1;
  
  f64 dLat = DegToRad(lat2 - lat1);
  f64 dLon = DegToRad(lon2 - lon1);
  lat1 = DegToRad(lat1);
  lat2 = DegToRad(lat2);
  
  f64 a = Square(sin(dLat / 2.0)) + cos(lat1) * cos(lat2) * Square(sin(dLon / 2));
  f64 c = 2.0 * asin(sqrt(a));
  
  f64 result = radius * c;
  
  return result;
}

// Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs"
f64 UniformRange(f64 min, f64 max)
{
	s_RngState ^= s_RngState >> 12;
	s_RngState ^= s_RngState << 25;
	s_RngState ^= s_RngState >> 27;

  u64 number = s_RngState * 0x2545F4914F6CDD1DULL;
	
  f64 value = static_cast<f64>(number) / static_cast<f64>(UINT64_MAX);
  return min + (value * (max - min));
}

static char* ReadFile(const char* name, bool isBinary)
{
  FILE* file = nullptr;

  if(isBinary)
  {
    file = fopen(name, "rb");
  }
  else
  {
    file = fopen(name, "r");
  }

  if(file == nullptr)
  {
    printf("Failed to open the file %s\n", name);
    return nullptr;
  }

  i32 success = fseek(file, 0, SEEK_END);
  i64 fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  i64 bufferSize = !isBinary ? fileSize + 1 : fileSize;
  char* buffer = (char*)malloc(bufferSize);
  
  if(buffer == nullptr)
  {
    printf("Failed to allocate memory.\n");
    fclose(file);
    return nullptr;
  }

  size_t bytesRead = fread(buffer, 1, fileSize, file);

  if(!isBinary)
  {
    buffer[bytesRead + 1] = '\0';
  }
  
  fclose(file);
  return buffer;
}