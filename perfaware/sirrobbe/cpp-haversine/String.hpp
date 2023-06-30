#pragma once

#include "Core.hpp"
#include "List.hpp"

#include <cstdlib>

class String;
class UTF16String;
class StringIterator;

b32 operator==(const String& a, const String& b);
b32 operator!=(const String& a, const String& b);

INTERNAL void Free(String* string);

/*
 * The base string type of the engine.
 * 
 * Internal representation:
 * It uses an internal UTF-8 encoding and is immutable. Therefore, each
 * operation on a string that changes its content will lead to the
 * creation and also the allocation of a new string. Consider this when
 * operating on strings in order to prevent code with poor performance.
 * An alternative to String does not exist yet. However, later on when
 * something like a StringBuilder is part of the engine, its usage would
 * be the preferred way to do operations that involve a lot of string
 * alterations.
 * 
 * Shallow Copy behavior:
 * Furthermore a string does not free its memory when it goes out of 
 * scope. This also allows a shallow copy behavior by default and spares
 * us of having to implement all the complicated move semantics.
 * Copies of string objects point to the same string in Memory. This makes
 * passing them very easy however one must be careful with ownership.
 * Ideally one string object is "marked" as the owner of the underlying
 * string. This string is the one that should free the memory when it
 * goes out of scope. To further enforce this style every string that does
 * not own the memory should be a const String or const pointer to a string.
 * Using this style does prohibit these objects access to the Free() method.
 * That way the compiler can help to enforce the correct string ownership
 */
class String
{
    friend class StringIterator;
    
    public:
    explicit String();
    explicit String(char* string);
    explicit String(byte* bytes, u64 size);
    explicit String(char character);
    explicit String(Rune rune);

    // TODO(Fabi): Deprecate this method in the long run as we require all types
    //             to have a freeing mechanism like 'Free(&string)' in order to make some
    //             macros work.
    void Free();
    
    // NOTE(Fabian): This is against our naming conventions, however these
    // functions have to be written with a lowercase otherwise the compiler
    // can't find them and use them for the foreach loop and other algorithms.
    // NOTE(Fabian): Also never add non-constant iterators! Since in UTF-8 not
    // every character has to be the same size. It could be very dangerous to just
    // change the value of a rune without comparing the sizes of the
    // characters.
    StringIterator begin() const;
    StringIterator end() const;
    
    const char* AsCString() const { return REINTERPRET(const char*, mData); }
    UTF16String ToUTF16() const;
    String DeepCopy() const;
    
    i64 Size() const { return mSize; }
    i64 Length() const {return mLength; }

    i64 FirstIndexOf(Rune rune) const;
    i64 FirstIndexOf(Rune rune, i64 start) const;
    i64 LastIndexOf(Rune rune) const;

    i64 GetNextCodepointIndex(u64 start) const;
    i64 GetPrevCodepointByteOffset(u64 start) const;
    Rune GetCodepointByIndex(i64 index) const;
    Rune GetCodepointAtByteOffset(u64 offset) const;

    i64 GetByteOffsetOfIndex(i64 index) const;

    String ToLower() const;
    String Replace(const String& stringToReplace, const String& newString) const;
    
    u32 BinarySearch(u32 arr[], u32 l, u32 r, Rune val) const;
    
    String Concat(const String& string) const;
    //String Concat(const char* string) const;
    String Concat(char character) const;

    String Substring(u64 startIndex, u64 length) const;
    String SubstringByOffset(i64 start, i64 end) const;
    String Trim();

    bool StartsWith(Rune val) const;
    bool EndsWith(Rune val) const;
    bool Contains(Rune val) const;
    
    Rune operator[](i64 index) const;

    protected:
    
    byte* mData;
    i64 mSize;
    i64 mLength;
    
    private:
    
    void DetermineLength();
    int GetCodepointSize(u64 index) const;
};

class StringIterator
{
    public:
    StringIterator(const String* string, u64 offset, u64 index);

    u64 Offset() const { return mOffset; }
    u64 Index() const { return mIndex; }

    bool operator==(const StringIterator& other) const;
    bool operator!=(const StringIterator& other) const;
    StringIterator operator++();
    StringIterator operator--();
    Rune operator*() const;
    
    private:
    
    u64 mOffset;
    u64 mIndex;
    const String* mString;
};

class ScopedString : public String
{
    public:
    
    explicit ScopedString(char* string) : String(string) {}
    explicit ScopedString(byte* bytes, u64 size) : String(bytes, size) {}
    explicit ScopedString(Rune rune) : String(rune) {}
    explicit ScopedString(String string);
    ~ScopedString() { Free(); }
    
    private:
    ScopedString operator=(const ScopedString& str);

};

INTERNAL int  GetCodepointUTF8Size(Rune rune);
INTERNAL bool CodepointToUTF8(Rune rune, int* utf8, int* size);

INTERNAL f64 ToF64(const String& string);

INTERNAL String ToString(int number);
INTERNAL String ToString(f32 number);

INTERNAL String FormatString(const char* format, ...);

#define SCOPED_FORMAT(format, ...) ScopedString(FormatString(format,  __VA_ARGS__))

b32 operator==(const String& a, const String& b)
{
    if(a.Size() != b.Size()) { return false; }

    StringIterator itA = a.begin();
    StringIterator itB = b.begin();

    for(i64 i = 0; i < a.Length(); i++)
    {
        Rune runeA = *itA;
        Rune runeB = *itB;
        if(runeA != runeB)
        {
            return false;
        }

        ++itA;
        ++itB;
    }

    return true;
}

b32 operator!=(const String& a, const String& b)
{
    return !(a == b);
}

#include "String.cpp"

[[maybe_unused]] INTERNAL void Free(String* string)
{
    string->Free();
}

String String::SubstringByOffset(i64 start, i64 end) const
{
    return String(&mData[start], end - start);
}