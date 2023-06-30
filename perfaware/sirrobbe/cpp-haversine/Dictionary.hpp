#pragma once

#include "Core.hpp"
#include "HashFunctions.hpp"

template<typename TKey, typename TValue>
struct KeyValuePair
{
    TKey* Key;
    TValue* Value;
};

template<typename TKey, typename TValue>
class DictionaryNode
{
    public:
    explicit DictionaryNode(const TKey& key, const TValue& value) :
        Next(nullptr),
        Value(value),
        mKey(key)
    {}
    
    TKey& Key() { return mKey; }
    
    DictionaryNode<TKey, TValue>* Next = nullptr;
    TValue Value;

    private:
    TKey mKey;
};

template<typename TKey, typename TValue>
class Dictionary;

template<typename TKey, typename TValue>
class DictionaryIterator
{
    public:
    explicit DictionaryIterator(const Dictionary<TKey, TValue>* dict, i64 index, DictionaryNode<TKey, TValue>* node)
        : mDictionary(dict), mCurrentIndex(index), mCurrentNode(node) {}

    bool operator==(const DictionaryIterator& other) const
    {
        return 
            mDictionary == other.mDictionary &&
            mCurrentNode == other.mCurrentNode &&
            mCurrentIndex == other.mCurrentIndex;
    }

    void operator++()
    {
        if (mCurrentNode->Next != nullptr)
        {
            mCurrentNode = mCurrentNode->Next;
        }
        else
        {
            i64 index = mCurrentIndex + 1;
            while(index < mDictionary->mCapacity)
            {
                if(mDictionary->mTable[index] == nullptr)
                {
                    index++;
                }
                else
                {
                    mCurrentIndex = index;
                    mCurrentNode = mDictionary->mTable[index];
                    return;
                }
            }

            mCurrentIndex = index;
            mCurrentNode = nullptr;
        }
    }

    bool operator!=(const DictionaryIterator& other) const
    {
        return !(*this == other);
    }

    KeyValuePair<TKey, TValue> operator*() const
    {
        return {&mCurrentNode->Key(), &mCurrentNode->Value};
    }

    private:
    const Dictionary<TKey, TValue>* mDictionary;
    i64 mCurrentIndex;
    DictionaryNode<TKey, TValue>* mCurrentNode;
};

// TODO(Fabi): Reimplement the dictionary with  offsets on collisions instead of a linked list at the hash
template<typename TKey, typename TValue>
class Dictionary
{   
    using Node = DictionaryNode<TKey, TValue>;
    friend class DictionaryIterator<TKey, TValue>;

    public:
    explicit Dictionary() :
        mCapacity(8),
        mCount(0),
        mTable(nullptr)
    {
        mTable = (Node**)calloc(mCapacity, sizeof(Node*));
    }

    explicit Dictionary(i64 initCapacity) :
        mCapacity(initCapacity),
        mCount(0),
        mTable(nullptr)
    {
        mTable = (Node**)calloc(mCapacity, sizeof(Node*));
    }
    
    void Free()
    {
        Clear();
        free(mTable);
    }

    void Clear()
    {
        for(i64 i = 0; i < mCapacity; i++)
        {
            Node* entry = mTable[i];
            while(entry != nullptr)
            {
                Node* temp = entry;
                entry = entry->Next;
                delete temp;
            }

            mTable[i] = nullptr;
        }

        mCount = 0;
    }

    void Add(const TKey& key, const TValue& value)
    {
        TryAdd(key, value);
    }
    
    bool TryAdd(const TKey& key, const TValue& value)
    {
        i64 hash = Hash(key, mCapacity);
        Node* prev = nullptr;
        Node* entry = mTable[hash];
        
        while(entry != nullptr && entry->Key() != key)
        {
            prev = entry;
            entry = entry->Next;
        }
        
        if(entry == nullptr)
        {
            entry = new Node(key, value);
            if(prev == nullptr)
            {
                mTable[hash] = entry;
            }
            else
            {
                prev->Next = entry;
            }

            mCount++;

            if(mCount == mCapacity)
            {
                Resize(2 * mCapacity);
            }

            return true;            
        }
        
        return false;
    }
    
    bool TryGet(const TKey& key, TValue& val)
    {
        Nullable<TValue> value = (*this)[key];
        if(value.IsValid)
        {
            val = *value.Value;
            return true;
        }
        
        return false;
    }

    [[maybe_unused]]
    bool TryGet(const TKey& key, TValue** val)
    {
        Nullable<TValue> value = (*this)[key];
        if(value.IsValid)
        {
            *val = value.Value;
            return true;
        }

        return false;
    }

    Nullable<TValue> operator[](const TKey& key) const
    {
        return (*this)[key];
    }
    
    Nullable<TValue> operator[](const TKey& key)
    {
        i64 hash = Hash(key, mCapacity);
        Node* entry = mTable[hash];
        
        while(entry != nullptr)
        {
            if(entry->Key() == key)
            {
                return { true, &entry->Value };
            }
            
            entry = entry->Next;
        }
        
        return { false, nullptr };
    }

    [[maybe_unused]]
    bool Remove(const TKey& key)
    {
        i64 hash = Hash(key, mCapacity);
        Node* prev = nullptr;
        Node* entry = mTable[hash];
        
        while(entry != nullptr && entry->Key() != key)
        {
            prev = entry;
            entry = entry->Next;
        }
        
        if(entry == nullptr)
        {
            return false;
        }
        if(prev == nullptr)
        {
            mTable[hash] = entry->Next;
        }
        else
        {
            prev->Next = entry->Next;
        }
        
        mCount--;
        delete entry;
        return true;    
    }
    
    void Resize(i64 newCapacity)
    {
        // TODO(Fabi): Replace assert with ASSERT
        assert(mCount <= newCapacity);
        Node** temp = mTable;

        mTable = (Node**)calloc(newCapacity, sizeof(Node*));

        for(i64 i = 0; i < mCapacity; i++)
        {
            Node* node = temp[i];
            while(node != nullptr)
            {
                i64 newHash = Hash(node->Key(), newCapacity);
                Node* newLocation = mTable[newHash];
                Node* previous = nullptr;
                
                while(newLocation != nullptr)
                {
                    previous = newLocation;
                    newLocation = newLocation->Next;
                }

                auto newNode = new Node(node->Key(), node->Value);
                if(previous == nullptr)
                {
                    mTable[newHash] = newNode;
                }
                else
                {
                    previous->Next = newNode;
                }

                Node* next = node->Next;
                free(node);
                node = next;
            }
        }

        free(temp);
        mCapacity = newCapacity;
    }

    b32 ContainsKey(const TKey& key)
    {
        i64 hash = Hash(key, mCapacity);
        Node* node = mTable[hash];

        if(node == nullptr)
        {
            return false;
        }

        while(node != nullptr)
        {
            if(node->Key() == key)
            {
                return true;
            }

            node = node->Next;
        }

        return false;
    }

    i64 Count() { return mCount; }
    i64 Capacity() { return mCapacity; }

    DictionaryIterator<TKey, TValue> begin() const
    {
        i64 index = 0;
        while(index < mCapacity)
        {
            if(mTable[index] == nullptr)
            {
                index++;
            }
            else
            {
                return DictionaryIterator<TKey, TValue>(this, index, mTable[index]);
            }
        }

        return DictionaryIterator<TKey, TValue>(this, index, nullptr);
    }

    DictionaryIterator<TKey, TValue> end() const
    {
        return DictionaryIterator<TKey, TValue>(this, mCapacity, nullptr);
    }
    
    private:
    i64 mCapacity;
    i64 mCount;
    Node** mTable;
};