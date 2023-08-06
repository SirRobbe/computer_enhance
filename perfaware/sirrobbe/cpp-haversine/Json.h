#pragma once

#include "Core.hpp"
#include "String.hpp"
#include "List.hpp"
#include "Dictionary.hpp"
#include "Profiling.h"

struct JsonObject;
class JsonValue;
struct JsonToken;

typedef List<JsonValue> JsonArray;

enum class JsonValueType
{
    Object = 0,
    Array,
    String,
    Number,
    Boolean,
    Null,
    Count
};

enum class JsonTokenType
{
    LeftBrace = 0,
    RightBrace,
    LeftSquareBracket,
    RightSquareBracket,
    Colon,
    Comma,
    String,
    Number,
    True,
    False,
    Null,
    Count
};

INTERNAL JsonObject ParseJson(String* json);
INTERNAL void FreeJsonArray(JsonArray* array);
INTERNAL void FreeJsonValue(JsonValue* value);

INTERNAL JsonObject ParseJsonObject(List<JsonToken>* tokens, u64* index);
INTERNAL JsonArray ParseJsonArray(List<JsonToken>*, u64* index);
INTERNAL JsonValue ParseJsonValue(List<JsonToken>*, u64* index);

INTERNAL String ParseStringLiteral(StringIterator* iterator, String* json);
INTERNAL String ParseNumberLiteral(StringIterator* iterator, String* json);
INTERNAL String ParseKeyword(StringIterator* iterator, String* json);

INTERNAL b32 IsLatinLetter(Rune rune);

struct JsonToken
{
    JsonTokenType Type = JsonTokenType::Count;
    String* Literal = nullptr;
    i32 Line = 0;
};

struct JsonObject
{
    public:

    explicit JsonObject(i32 capacity = 1) : m_Elements(capacity) {}

    void Free();

    void Add(String& key, JsonValue value);

    b32 ContainsKey(const String& key);

    JsonValue& operator[](const String& key);

    private:

    Dictionary<String, JsonValue> m_Elements;
};

class JsonValue
{
public:

    operator i32() const;
    operator String() const;
    operator JsonArray() const;
    operator JsonObject() const;

    JsonValueType Type = JsonValueType::Null;
    union
    {
        void* Null;
        JsonObject Object;
        JsonArray Array;
        String String;
        f64 Number;
        b32 Boolean;
    };
};

const JsonValue JsonNullValue = {JsonValueType::Null, {nullptr}};

INTERNAL JsonObject ParseJson(String* json)
{
    BlockProfiler profiler("ParseJson");

    List<JsonToken> tokens(256);

    // NOTE(Fabian): Clear the whole token structure after parsing.
    defer({
        for(i64 i = 0; i < tokens.Count(); i++)
        {
            if(tokens[i].Literal != nullptr)
            {
                tokens[i].Literal->Free();
                delete tokens[i].Literal;
                tokens[i].Literal = nullptr;
            }
        }

        tokens.Free();
    });

    i32 line = 1;
    StringIterator it = json->begin();
    while(it != json->end())
    {
        switch(*it)
        {
            case '{':
            {
                tokens.Add({JsonTokenType::LeftBrace, nullptr, line});
                break;
            }

            case '}':
            {
                tokens.Add({JsonTokenType::RightBrace, nullptr, line});
                break;
            }

            case '[':
            {
                tokens.Add({JsonTokenType::LeftSquareBracket, nullptr, line});
                break;
            }

            case ']':
            {
                tokens.Add({JsonTokenType::RightSquareBracket, nullptr, line});
                break;
            }

            case ':':
            {
                tokens.Add({JsonTokenType::Colon, nullptr, line});
                break;
            }

            case ',':
            {
                tokens.Add({JsonTokenType::Comma, nullptr, line});
                break;
            }

            case ' ':
            case '\t':
            case '\r':
            break;

            case '\n':
            {
                line++;
                break;
            }

            case '"':
            {
                auto* string = new String();
                *string = ParseStringLiteral(&it, json);
                tokens.Add({JsonTokenType::String, string, line});
                break;
            }

            default:
            {
                if(*it == '-' || IsDigit(*it))
                {
                    auto* number = new String();
                    *number = ParseNumberLiteral(&it, json);
                    tokens.Add({JsonTokenType::Number, number, line});
                    break;
                }
                else if(IsLatinLetter(*it))
                {
                    ScopedString keyword(ParseKeyword(&it, json));

                    const char* t = "true";
                    const char* f = "false";
                    const char* n = "null";    

                    if(keyword == String(const_cast<char*>(t)))
                    {
                        tokens.Add({JsonTokenType::True, nullptr, line});
                        break;
                    }
                    else if(keyword == String(const_cast<char*>(f)))
                    {
                        tokens.Add({JsonTokenType::False, nullptr, line});
                        break;
                    }
                    else if(keyword == String(const_cast<char*>(n)))
                    {
                        tokens.Add({JsonTokenType::Null, nullptr, line});
                        break;
                    }
                }

                break;
            }
        }

        ++it;
    }

    u64 index = 0;
    JsonObject rootObject = ParseJsonObject(&tokens, &index);
    return rootObject;
}

INTERNAL void FreeJsonArray(JsonArray* array)
{
    for(auto value : *array)
    {
        FreeJsonValue(&value);
    }

    array->Free();
}

INTERNAL void FreeJsonValue(JsonValue* value)
{
    switch(value->Type)
    {
        case JsonValueType::Array:
        {
            FreeJsonArray(&value->Array);
            break;
        }

        case JsonValueType::Object:
        {
            value->Object.Free();
            break;
        }

        case JsonValueType::String:
        {
            value->String.Free();
            break;
        }

        default:
        {
            break;
        }
    }
}

INTERNAL JsonObject ParseJsonObject(List<JsonToken>* tokens, u64* index)
{
    i64 start = CAST(i64, *index);
    (*index)++;

    // NOTE(Fabian): Empty object
    if((*tokens)[CAST(i64, *index)].Type == JsonTokenType::RightBrace)
    {
        return JsonObject();
    }

    // NOTE(Fabian): We peek ahead to see how many elements the object has. With this knowledge
    //               we can allocate the right amount of memory and can thereby be very memory-efficient.
    i32 depth = 1;
    i32 elementsCount = 1;
    while(*index < CAST(u64, tokens->Count()))
    {
        if((*tokens)[CAST(i64, *index)].Type == JsonTokenType::RightBrace)
        {
            depth -= 1;
            if(depth == 0)
            {
                break;
            }
        }
        else if((*tokens)[CAST(i64, *index)].Type == JsonTokenType::RightSquareBracket)
        {
            depth -= 1;
        }
        else if((*tokens)[CAST(i64, *index)].Type == JsonTokenType::LeftBrace ||
                (*tokens)[CAST(i64, *index)].Type == JsonTokenType::LeftSquareBracket)
        {
            depth++;
        }
        else if((*tokens)[CAST(i64, *index)].Type == JsonTokenType::Comma && depth == 1)
        {
            elementsCount++;
        }

        (*index)++;
    }

    JsonObject object(elementsCount);
    *index = start + 1;

    for(i32 i = 0; i < elementsCount; ++i)
    {
        String key = (*tokens)[CAST(u64, *index)].Literal->DeepCopy();
        (*index)++;

        (*index)++;
        JsonValue value = ParseJsonValue(tokens, index);
        object.Add(key, value);

        if((*tokens)[CAST(u64, *index)].Type == JsonTokenType::Comma)
        {
            ++(*index);
        }
    }

    ++(*index);
    return object;
}

INTERNAL JsonArray ParseJsonArray(List<JsonToken>* tokens, u64* index)
{
    ++(*index);
    u64 start = *index;

    // NOTE(Fabian): Empty array
    if((*tokens)[CAST(i64, *index)].Type == JsonTokenType::RightSquareBracket)
    {
        return {};
    }

    // NOTE(Fabian): We peek ahead to see how many elements the object has. With this knowledge
    //               we can allocate the right amount of memory and can thereby be very memory-efficient.
    i32 depth = 1;
    i32 elementsCount = 1;
    while(*index < CAST(u64, tokens->Count()))
    {
        switch((*tokens)[CAST(i64, *index)].Type)
        {
            case JsonTokenType::RightSquareBracket:
            case JsonTokenType::RightBrace:
            {
                depth--;
                break;
            }

            case JsonTokenType::LeftBrace:
            case JsonTokenType::LeftSquareBracket:
            {
                depth++;
                break;
            }

            case JsonTokenType::Comma:
            {
                if(depth == 1)
                {
                    elementsCount++;
                }

                break;
            }

            default:
            {
                break;
            }
        }

        if(depth == 0)
        {
            break;
        }

		++(*index);
    }

    JsonArray array(elementsCount);
    *index = start;
    for(i32 i = 0; i < elementsCount; ++i)
    {
        array.Add(ParseJsonValue(tokens, index));
        ++(*index);
    }

    if((*tokens)[CAST(i64, *index)].Type == JsonTokenType::RightSquareBracket)
    {
        ++(*index);
    }

    return array;
}

INTERNAL JsonValue ParseJsonValue(List<JsonToken>* tokens, u64* index)
{
    JsonValue value = {JsonValueType::Null, {nullptr}};

    switch((*tokens)[CAST(i64, *index)].Type)
    {
        case JsonTokenType::LeftBrace:
        {
            value.Type = JsonValueType::Object;
            value.Object = ParseJsonObject(tokens, index);
            break;
        }

        case JsonTokenType::LeftSquareBracket:
        {
            value.Type = JsonValueType::Array;
            value.Array = ParseJsonArray(tokens, index);
            break;
        }

        case JsonTokenType::String:
        {
            value.Type = JsonValueType::String;
            value.String = (*tokens)[CAST(i64, *index)].Literal->DeepCopy();
            ++(*index);
            break;
        }

        case JsonTokenType::Number:
        {
            value.Type = JsonValueType::Number;
            value.Number = ToF64(*(*tokens)[CAST(i64, *index)].Literal);
            ++(*index);
            break;
        }

        case JsonTokenType::True:
        {
            value.Type = JsonValueType::Boolean;
            value.Boolean = true;
            ++(*index);
            break;
        }

        case JsonTokenType::False:
        {
            value.Type = JsonValueType::Boolean;
            value.Boolean = false;
            ++(*index);
            break;
        }

        case JsonTokenType::Null:
        {
            value.Type = JsonValueType::Null;
            value.Null = nullptr;
            ++(*index);
            break;
        }

        default:
        {
            break;
        }
    }

    return value;
}

INTERNAL String ParseStringLiteral(StringIterator* iterator, String* json)
{
    StringIterator last = *iterator;
    ++(*iterator);

    u64 start = iterator->Offset();
    while(**iterator != '\"' && *last != '\\')
    {
        last = *iterator;
        ++(*iterator);
    }

    return json->SubstringByOffset(start, iterator->Offset());
}

INTERNAL String ParseNumberLiteral(StringIterator* iterator, String* json)
{
    u64 start = iterator->Offset();

    if(**iterator == '-')
    {
        ++(*iterator);
    }

    while(IsDigit(**iterator))
    {
        ++(*iterator);
    }

    if(**iterator != '.')
    {
        String string = json->SubstringByOffset(start, iterator->Offset());;
        --(*iterator);
        return string;
    }

    ++(*iterator);
    while(IsDigit(**iterator))
    {
        ++(*iterator);
    }

    i64 index = CAST(i64, iterator->Index());
    if((**iterator == 'E' || **iterator == 'e') &&
       ((*json)[index + 1] == '+' || (*json)[index + 1] == '-'))
    {
        ++(*iterator);
        ++(*iterator);
        while(IsDigit(**iterator))
        {
            ++(*iterator);
        }
    }

    String string = json->SubstringByOffset(start, iterator->Offset());
    --(*iterator);
    return string;
}

INTERNAL String ParseKeyword(StringIterator* iterator, String* json)
{
    u64 start = iterator->Index();
    ++(*iterator);
    while(IsLatinLetter(**iterator))
    {
        ++(*iterator);
    }

    --(*iterator);
    return json->Substring(start, (iterator->Index() + 1) - start);
}

INTERNAL b32 IsLatinLetter(Rune rune)
{
    return (rune >= 'a' && rune <= 'z') || (rune >= 'A' && rune <= 'Z');
}

void JsonObject::Free()
{
    for(auto kvp : m_Elements)
    {
        kvp.Key->Free();
        FreeJsonValue(kvp.Value);
    }

    m_Elements.Free();
}

void JsonObject::Add(String& key, JsonValue value)
{
    m_Elements.Add(key, value);
}

b32 JsonObject::ContainsKey(const String& key)
{
    return m_Elements.ContainsKey(key);
}

JsonValue& JsonObject::operator[](const String& key)
{
    if(!m_Elements.ContainsKey(key))
    {
        // TODO(Fabi): This is hard bullshit.
        JsonValue* val = new JsonValue{};
        return *val;
    }

    return *m_Elements[key].Value;
}

JsonValue::operator i32() const
{
    return CAST(i32, Number);
}

JsonValue::operator class String() const
{
    return String;
}

JsonValue::operator JsonArray() const
{
    return Array;
}

JsonValue::operator JsonObject() const
{
    return Object;
}