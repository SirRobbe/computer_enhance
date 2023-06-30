#pragma once

#include <cassert>
#include <cstdlib>
#include "Core.hpp"

template<typename T> class ListIterator;

template<typename T>
class List
{
    friend class ListIterator<T>;

    public:
    List()
    {
        mRoot = (T*)calloc(1, sizeof(T));
        mCapacity = 1;
        mCount = 0;
    }

    List(i64 size)
    {
        mRoot = (T*)calloc(size, sizeof(T));
        mCapacity = size;
        mCount = 0;
    }

    List(const T* data, i64 size)
    {
        mRoot = (T*)calloc(size, sizeof(T));
        mCapacity = size;
        mCount = size;

        for(i64 i = 0; i < size; i++)
        {
            mRoot[i] = data[i];
        }
    }

    void Free()
    {
        free(mRoot);
        mRoot = nullptr;
        mCapacity = 0;
        mCount = 0;
    }
    
    void Add(const T& data)
    {
        if(mCount == mCapacity)
        {
            ExtendCapacity(mCount * 2);
        }
        
        mRoot[mCount] = data;
        mCount++;
    }

    void AddList(const List<T>& list)
    {
        if((list.Count() + this->Count()) > this->Capacity())
        {
            ExtendCapacity(list.Count() + this->Count());
        }
        for(i64 i = 0; i < list.Count(); i++)
        {
            mRoot[Count()] = list[i];
            mCount++;
        }
    }
    
    void Clear()
    {
        mCount = 0;
    }
    
    T& At(i64 index)
    {
        return (*this)[index];
    }

    const T& At(i64 index) const
    {
        return (*this)[index];
    }

    bool Contains(const T& data) const
    {
        for(i64 i = 0; i < mCount; i++)
        {
            if(mRoot[i] == data)
            {
                return true;
            }
        }
        
        return false;
    }
    
    bool Remove(const T& data)
    {
        for(i64 i = 0; i < mCount; i++)
        {
            if(mRoot[i] == data)
            {
                mCount--;
                for(i64 offset = 0; (offset + i) < mCount; offset++)
                {
                    mRoot[i + offset] = mRoot[i + offset + 1];
                }
                
                // NOTE(Daniel):Last Element will not be set to null as it is not accessible anymore
                return true;
            }
        }
        
        return false;
    }

    void RemoveUnorderedByIndex(i64 index)
    {
        assert(index >= 0 && index < mCount);
        mRoot[index] = mRoot[mCount - 1];
        mCount--;
    }
    
    i64 IndexOf(const T& data)
    {
        for(i64 i = 0; i < mCount; i++)
        {
            if(mRoot[i] == data)
            {
                return i;
            }
        }
        
        return -1;
    }

    void Reserve(i64 capacity)
    {
        if(mRoot != nullptr)
        {
            free(mRoot);
        }

        mRoot = CAST(T*, calloc(capacity, sizeof(T)));
        mCount = 0;
        mCapacity = capacity;
    }

    T& operator[](const i64 index)
    {
        assert(index >= 0);
        assert(index < mCount);
        return mRoot[index];
    }

    const T& operator[](const i64 index) const
    {
        assert(index >= 0);
        assert(index < mCount);
        return mRoot[index];
    }
    
    i64 Capacity() const { return mCapacity; }
    i64 Count() const { return mCount; }

    const ListIterator<T> begin() const
    {
        return ListIterator<T>(this, 0);
    }
    
    const ListIterator<T> end() const
    {
        return ListIterator<T>(this, mCount);
    }
    
    private:
    
    void ExtendCapacity(i64 newCapacity)
    {
        T* temp = mRoot;
        
        mRoot = (T*)calloc(newCapacity, sizeof(T));
        
        for(i64 i = 0; i < mCount; i++)
        {
            mRoot[i] = temp[i];
        }
        free(temp);
        
        mCapacity = newCapacity;
    }

    T* mRoot;
    i64 mCapacity;
    i64 mCount;
};

template<typename T>
class ListIterator
{
    public:
    ListIterator(const List<T>* list, u64 offset)
        : mOffset(offset), mList(list) {}

    u64 Offset() const { return mOffset; }

    bool operator!=(const ListIterator& other) const
    {
        return mOffset != other.mOffset;
    }

    ListIterator operator++()
    {
        mOffset++;
        return ListIterator(mList, mOffset);
    }

    T operator*() const
    {
        return (*mList)[mOffset];
    }

    private:
    u64 mOffset;
    const List<T>* mList;
};

#define FREE_LIST(list)                      \
{                                            \
    for(i32 i = 0; i < (list).Count(); i++)  \
    {                                        \
        Free(&(list)[i]);                    \
    }                                        \
                                             \
    (list).Free();                           \
}