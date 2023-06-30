#pragma once

/*
 * Contains base data types and C++ language extensions like a defer statement.
 */

#include <cstdint>

#ifdef GE_WIN32
#define EXPORT __declspec(dllexport)
#elif GE_LINUX
#define EXPORT __attribute__((visibility ("default")))
#endif

#define INTERNAL static
#define GLOBAL static
#define SHARED static

#define CAST(type, expression) static_cast<type>((expression))
#define TRY_CAST(type, expression) dynamic_cast<type>((expression));
#define REINTERPRET(type, expression) reinterpret_cast<type>((expression))

#define UNUSED(var) do { (void)(var); } while (0)
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t byte;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef i32 b32;

[[maybe_unused]] typedef u16 Utf16Rune;
typedef u32 Rune;

[[maybe_unused]] constexpr i32 MAX_I32 = 2147483647;
constexpr i32 I16_MIN = -32768;
constexpr i32 I16_MAX = 32767;
constexpr i32 U16_MAX = 65535;

constexpr u64 U64_MAX = 18446744073709551615u;

constexpr u64 DWORD_MAX = 4294967295;

struct Memory;
template<typename T> struct DeferStatement;
template<typename T> struct Optional;

// TODO(Fabi): Move into its own file
template<typename T> struct Nullable;

template<typename T> DeferStatement<T> MakeDeferStatement(T statement);
template<typename T> T Unpack(Optional<T> optional);

INTERNAL b32 IsDigit(Rune rune);
INTERNAL b32 IsAlphaNumeric(Rune rune);
constexpr INTERNAL i64 GigabytesToBytes(i64 gigabytes);

struct Memory
{
    u8* Location = nullptr;
    u64 Size = 0;
};

template<typename T> struct DeferStatement
{
    explicit DeferStatement(T statement) : Statement(statement) {}
    ~DeferStatement() { Statement(); }

    T Statement;
};

template<typename T> struct Optional
{
    const bool IsValid = false;
    const T Value;
};

template<typename T> struct Nullable
{
	Nullable& operator=(const T& value);

    const bool IsValid = false;
	T* const Value = nullptr;
};

// NOTE(Fabian): These multiple defines prevent warnings about redefinition,
//               but I don't know exactly why.
#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define defer(statement) \
auto DEFER_3(_defer_) = MakeDeferStatement([&](){statement;})

template<typename T> Nullable<T>& Nullable<T>::operator=(const T& value)
{
    *Value = value;
	return *this;
}

template <typename T>
DeferStatement<T> MakeDeferStatement(T statement)
{
	return DeferStatement<T>(statement);
}

template<typename T> T Unpack(Optional<T> optional)
{
    return optional.Value;
}

INTERNAL b32 IsDigit(Rune rune)
{
    return rune >= '0' && rune <= '9';
}

[[maybe_unused]] INTERNAL b32 IsAlphaNumeric(Rune rune)
{
    return IsDigit(rune) || (rune >= 'a' && rune <= 'z') || (rune >= 'A' && rune <= 'Z');
}

constexpr INTERNAL i64 GigabytesToBytes(i64 gigabytes)
{
    return gigabytes * 1024 * 1024 * 1024;
}