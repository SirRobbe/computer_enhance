#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <stdint.h>

#include "Json.h"

typedef int32_t i32;

f32 DegToRad(f32 deg) { return 0.01745329238f * deg; }

f32 Square(f32 x) { return x * x; }

f32 Haversine(f32 x0, f32 y0, f32 x1, f32 y1, f32 radius)
{
  f32 lat1 = y0;
  f32 lat2 = y1;
  f32 lon1 = x0;
  f32 lon2 = x1;

  f32 dLat = DegToRad(lat2 - lat1);
  f32 dLon = DegToRad(lon2 - lon1);
  lat1 = DegToRad(lat1);
  lat2 = DegToRad(lat2);

  f32 a = Square(sin(dLat / 2.0)) + cos(lat1) * cos(lat2) * Square(sin(dLon / 2));
  f32 c = 2.0 * asin(sqrt(a));

  f32 result = radius * c;
  return result;
}

i32 main(i32 argc, char* argv[])
{
  const char* fileName = argv[1];

  FILE* file = fopen(fileName, "r");
  if(file == NULL)
  {
    printf("Failed to open the file.\n");
    return 1;
  }

  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* buffer = (char*)malloc(fileSize + 1);
  if(buffer == NULL)
  {
    printf("Failed to allocate memory.\n");
    fclose(file);
    return 1;
  }

  size_t bytesRead = fread(buffer, 1, fileSize, file);
  if(bytesRead != fileSize)
  {
    printf("Failed to read the file.\n");
    fclose(file);
    free(buffer);
    return 1;
  }

  buffer[fileSize] = '\0'; // Add a null terminator at the end of the buffer for string operations

  fclose(file); // Close the file

  f32* results = nullptr;
  {
    const char* validatonFileName = argv[2];
    FILE* file = fopen(validatonFileName, "rb");
    if(file == NULL)
    {
      printf("Failed to open the file.\n");
      return 1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(fileSize);
    if(buffer == NULL)
    {
      printf("Failed to allocate memory.\n");
      fclose(file);
      return 1;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if(bytesRead != fileSize)
    {
      printf("Failed to read the file.\n");
      fclose(file);
      free(buffer);
      return 1;
    }
    
    fclose(file);

    results = reinterpret_cast<f32*>(buffer);
  }

  printf("Hello, %s!\n", fileName);

  auto data = String(buffer);
  auto json = ParseJson(&data);

  free(buffer);

  const char* p = "pairs";
  String pairKey(const_cast<char*>(p));
  auto pairs = json[pairKey].Array;
  printf("pairs: %lld\n", pairs.Count());

  const char* a = "x0";
  const char* b = "y0";
  const char* c = "x1";   
  const char* d = "y1";   
  String x0Key(const_cast<char*>(a));
  String y0Key(const_cast<char*>(b));
  String x1Key(const_cast<char*>(c));
  String y1Key(const_cast<char*>(d));

  f32 sum = 0.f;
  for(i64 index = 0; index < pairs.Count(); index++)
  {
    auto pair = pairs[index].Object;
    f32 x0 = pair[x0Key].Number;
    f32 y0 = pair[y0Key].Number;
    f32 x1 = pair[x1Key].Number;
    f32 y1 = pair[y1Key].Number;

    f32 distance = Haversine(x0, y0, x1, y1, 6372.8f);
    f32 expected = results[index];
    if(abs(distance - expected) > 0.1f)
    {
      printf("index: %lld ; distance: %f ; expected: %f\n", index, distance, expected);
    }

    sum += distance;
  }

  f32 average = sum / pairs.Count();
  printf("result: %f\n", average);

  return 0;
}
