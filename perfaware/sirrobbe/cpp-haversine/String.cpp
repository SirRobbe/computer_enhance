#include <stdarg.h>

#include <cmath>

String::String()
{
    mData = 0;
    mLength = 0;
    mSize = 0;
}

String::String(byte* bytes, u64 size)
{
    mSize = size;
    mData = REINTERPRET(byte*, malloc(mSize + 1));
    
    for(u64 i = 0; i < size; i++)
    {
        mData[i] = bytes[i];
    }
    
    mData[mSize] = 0;
    DetermineLength();
}

String::String(char* string)
{
   
    mData = reinterpret_cast<u8*>(string);    
    DetermineLength();
}


String::String(char character) : String(REINTERPRET(byte*, &character), 1) {}
String::String(Rune rune)
{
    int bytes = 0;
    int size = 0;
    bool success = CodepointToUTF8(rune, &bytes, &size);
    *this = String(REINTERPRET(byte*, &bytes), size);
}

void String::Free()
{
    free(mData);
    mData = nullptr;
    mSize = -1;
    mLength = -1;
}

String String::DeepCopy() const
{
    return String(mData, mSize);
}

i64 String::FirstIndexOf(Rune rune) const
{
    return FirstIndexOf(rune, 0);
}

i64 String::FirstIndexOf(Rune rune, i64 start) const
{
    StringIterator iterator = begin();
    
    for(i64 i = 0; i < start; i++)
    {
        ++iterator;
    }
    
    while(iterator != end())
    {
        if(*iterator == rune)
        {
            return iterator.Index();
        }
        
        ++iterator;
    }
    
    return -1;
}

i64 String::LastIndexOf(Rune rune) const
{
    for(StringIterator it = end(); it != begin(); --it)
    {
        if(*it == rune)
        {
            return it.Index();
        }
    }
    
    return -1;
}

i64 String::GetNextCodepointIndex(u64 start) const
{
    return start + GetCodepointSize(start);
}

i64 String::GetPrevCodepointByteOffset(u64 start) const
{
    int steps = 1;
    while((mData[start - steps] & 0b11000000) == 0b10000000)
    {
        steps++;
    }
    return start - steps;
}

Rune String::GetCodepointByIndex(i64 index) const
{
    // NOTE(Fabian): The index can not be valid if it is larger than the overall byte size
    //               of the string.
    if(index >= mLength)
    {
        // NOTE(Fabian): If invalid print the replacement sign
        return 65533;
    }

    // NOTE(Fabian): This implementation could use iterators. However, they have
    //               quite a bit of overhead, and we actually only need the size of
    //               a rune to advance the offset, so we can use this leaner implementation.
    i64 offset = GetByteOffsetOfIndex(index);
    return GetCodepointAtByteOffset(offset);
}

Rune String::GetCodepointAtByteOffset(u64 offset) const
{
    int size = GetCodepointSize(offset);
    switch(size)
    {
        case 1:
        default:
        {
            return mData[offset];
        }
        
        case 2:
        {
            return ((mData[offset] & 0x1F) << 6) + (mData[offset + 1] & 0x3F);
        }
        
        case 3:
        {
            return ((mData[offset] & 0xF) << 12) + ((mData[offset + 1] & 0x3F) << 6) +
            (mData[offset + 2] & 0x3F);
        }
        
        case 4:
        {
            return ((mData[offset] & 0x7) << 18) + ((mData[offset + 1] & 0x3F) << 12) +
            ((mData[offset + 2] & 0x3F) << 6) + (mData[offset + 3] & 0x3F);
        }
    }
}

i64 String::GetByteOffsetOfIndex(i64 index) const
{
    i64 offset = 0;
    for(i64 i = 0; i < index; i++)
    {
        offset += GetCodepointSize(offset);
    }

    return offset;
}

u32 String::BinarySearch(u32 arr[], u32 l, u32 r, Rune val) const
{
    if(r >= l)
    {
        u32 mid = l + (r - l) / 2;
        
        if(arr[mid] == val)
        {
            return mid;
        }
        
        if(arr[mid] > val)
        {
            return BinarySearch(arr, l, mid - 1, val);
        }
        
        return BinarySearch(arr, mid + 1, r, val);
    }
    
    return -1;
}

String String::Concat(const String& string) const
{
    const u64 size = mSize + string.mSize;
    byte* newString = CAST(byte*, malloc(size));
    defer(free(newString));

    u64 index = 0;
    for(auto it = begin(); it != end(); ++it)
    {
        int utf8Code = 0;
        int codepointSize = 0;
        bool success = CodepointToUTF8(*it, &utf8Code, &codepointSize);
        *REINTERPRET(int*, &newString[index]) = utf8Code;
        index += codepointSize;
    }
    
    for(auto it = string.begin(); it != string.end(); ++it)
    {
        int utf8Code = 0;
        int codepointSize = 0;
        bool success = CodepointToUTF8(*it, &utf8Code, &codepointSize);
        *REINTERPRET(int*, &newString[index]) = utf8Code;
        index += codepointSize;
    }
    
    return String(newString, size);
}

//String String::Concat(const char* string) const
//{
//    ScopedString stringWrapper(string);
//    return Concat(stringWrapper);
//}

String String::Concat(char character) const
{
    ScopedString stringWrapper(character);
    return Concat(stringWrapper);
}

String String::Substring(u64 startIndex, u64 length) const
{
    i64 startOffset = GetByteOffsetOfIndex(CAST(i64, startIndex));
    auto it = StringIterator(this, startOffset, startIndex);
    for(i32 i = 0; i < length; i++)
    {
        ++it;
    }

    return String(&mData[startOffset], it.Offset() - startOffset);
}

bool String::StartsWith(Rune val) const
{
    Rune first = GetCodepointByIndex(0);
    return first == val;
}

bool String::EndsWith(Rune val) const
{
    Rune last = GetCodepointByIndex(mLength - 1);
    return last == val;
}

bool String::Contains(Rune val) const
{
    return FirstIndexOf(val) != -1;
}

String String::Trim()
{
    StringIterator start = begin();
    if(StartsWith(' '))
    {
        while(*start == ' ')
        {
            ++start;
        }
    }
    
    StringIterator endIt = end();
    // Move one back to get to actual last symbol
    --endIt;
    if(EndsWith(' '))
    {
        while(*endIt == ' ')
        {
            --endIt;
        }
    }
    // + 1 step to redeem the one we did in the previous step
    ++endIt;
    
    return String(&mData[start.Offset()], endIt.Offset() - start.Offset());
}

Rune String::operator[](i64 index) const
{
    return GetCodepointByIndex(index);
}

StringIterator String::begin() const
{
    return StringIterator(this, 0, 0);
}

StringIterator String::end() const
{
    return StringIterator(this, mSize, mLength);
}

void String::DetermineLength()
{
    mLength = 0;
    mSize = 0;
    while(mData[mSize] != '\0')
    {
        mLength++;
        int size = GetCodepointSize(mSize);
        
        if(size < 1)
        {
            Free();
            return;
        }
        else
        {
            mSize += size;
        }
    }
}

int String::GetCodepointSize(u64 index) const
{
    if(mData[index] < 0x80)
    {
        return 1;
    }
    else if(mData[index] < 0xE0)
    {
        return 2;
    }
    else if (mData[index] < 0xF0)
    {
        return 3;
    }
    else if(mData[index]  < 245)
    {
        return 4;
    }
    else
    {
        return -1;
    }
}

StringIterator::StringIterator(const String* string, u64 offset, u64 index) :
mOffset(offset),
mIndex(index),
mString(string)
{}

bool StringIterator::operator==(const StringIterator& other) const
{
    return !(*this != other);
}

bool StringIterator::operator!=(const StringIterator& other) const
{
    return mOffset != other.mOffset;
}

Rune StringIterator::operator*() const
{
    return mString->GetCodepointAtByteOffset(mOffset);
}

StringIterator StringIterator::operator++()
{
    mOffset = mString->GetNextCodepointIndex(mOffset);
    mIndex++;
    return StringIterator(mString, mOffset, mIndex);
}

StringIterator StringIterator::operator--()
{
    mOffset = mString->GetPrevCodepointByteOffset(mOffset);
    mIndex--;
    return StringIterator(mString, mOffset, mIndex);
}

ScopedString::ScopedString(String string)
{
    mData = REINTERPRET(ScopedString*, &string)->mData;
    mLength = REINTERPRET(ScopedString*, &string)->mLength;
    mSize = REINTERPRET(ScopedString*, &string)->mSize;   
}

int GetCodepointUTF8Size(Rune rune)
{
    if((rune > 0x0010FFFF) || ((rune >= 0xD800) && (rune <= 0xDBFF)))
    {
        return -1;
    }
    
    if(rune < 0x80)
    {
        return 1;
    }
    else if(rune < 0x800)
    {
        return 2;
    }
    else if(rune < 0x10000)
    {
        return 3;
    }
    else
    {
        return 4;
    }
}

bool CodepointToUTF8(Rune rune, int* utf8, int* size)
{
    const byte firstBytes[7] =
    {
        0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
    };
    
    // NOTE(Fabian): Check if the rune is invalid
    if((rune > 0x0010FFFF) || ((rune >= 0xD800) && (rune <= 0xDBFF)))
    {
        *utf8 = 0;
        *size = 0;
        return false;
    }
    else
    {
        *size = GetCodepointUTF8Size(rune);
        
        byte bytes[4];
        switch(*size)
        {
            case 4: bytes[3] = (rune | 0x80) & 0xBF; rune >>= 6;
            case 3: bytes[2] = (rune | 0x80) & 0xBF; rune >>= 6;
            case 2: bytes[1] = (rune | 0x80) & 0xBF; rune >>= 6;
            case 1: bytes[0] = CAST(byte, rune | firstBytes[*size]);
        }
        
        *utf8 = *(REINTERPRET(int*, bytes));
        return true;
    }
}

[[maybe_unused]] INTERNAL f64 ToF64(const String& string)
{
    f64 integer = 0;
    StringIterator cursor = string.begin();

    b32 isNegative = false;
    if(*cursor == '-')
    {
        isNegative = true;
        ++cursor;
    }

    while(IsDigit(*cursor))
    {
        integer *= 10;
        integer += CAST(f64, *cursor - 48);
        ++cursor;
    }

    if(*cursor == '\0' || cursor == string.end())
    {
        if(isNegative)
        {
            integer *= -1;
        }

        return integer;
    }
    else if(*cursor != '.')
    {
    }

    ++cursor;
    f64 fraction = 0;
    f64 divisor = 1;
    while(IsDigit(*cursor))
    {
        divisor *= 10;
        fraction += CAST(f64, *cursor - 48) / divisor;
        ++cursor;
    }

    i32 exponent = 0;
    if(*cursor != '\0' || cursor != string.end())
    {
        b32 isExponentNegative = false;
        if(*(++cursor) == '-')
        {
            isExponentNegative = true;
            ++cursor;
        }

        while(IsDigit(*cursor))
        {
            exponent *= 10;
            exponent += CAST(i32, *cursor - 48);
            ++cursor;
        }

        if(isExponentNegative)
        {
            exponent *= -1;
        }
    }

    f64 value = integer + fraction;
    if(isNegative)
    {
        value *= -1;
    }

    if(exponent != 0)
    {
        value *= pow(10, exponent);
    }

    return value;
}