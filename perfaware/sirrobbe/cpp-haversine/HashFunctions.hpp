#pragma once

#include "Core.hpp"
#include "String.hpp"

INTERNAL i64 Hash(int value, i64 tableSize);
INTERNAL u64 Hash(const String& value);
INTERNAL i64 Hash(const String& value, i64 tableSize);

[[maybe_unused]] INTERNAL i64 Hash(int value, i64 tableSize)
{
    return value % tableSize;
}

[[maybe_unused]] INTERNAL u64 Hash(const String& value)
{
    // NOTE(Daniel): djb2 hashing function, but adapted to scale it
    // into the range of the hashtable
    u64 hash = 5318;

    auto it = value.begin();
    while (it != value.end())
    {
        auto ch = *it;
        hash = ((hash << 5) + hash) + ch;
        ++it;
    }

    return hash;
}

[[maybe_unused]] INTERNAL i64 Hash(const String& value, i64 tableSize)
{
    return CAST(i64, Hash(value) % tableSize);
}