#pragma once

#include <string>
#include <vector>

using namespace std;

template <typename T>
class HashMap
{
private:
    struct Entry
    {
        string key;
        T value;
    };

    vector<vector<Entry>> m_buckets;
    int m_size = 0;

    static unsigned int HashKey(const string& key)
    {
        unsigned int hash = 0;
        for (unsigned char ch:key)
        {
            hash = hash * 31 + ch;
        }
        return hash;
    }

    void Rehash(int new_cap)
    {
        vector<vector<Entry>> next_buckets(new_cap);
        for (auto& bucket:m_buckets)
        {
            for (auto& entry:bucket)
            {
                int idx = HashKey(entry.key) % new_cap;
                next_buckets[idx].push_back(move(entry));
            }
        }
        m_buckets = move(next_buckets);
    }

public:
    HashMap()
    {
        m_buckets.resize(16);
    }

    T& operator[](const string& key)
    {
        int cap = m_buckets.size();
        int idx = HashKey(key) % cap;
        for (auto& entry:m_buckets[idx])
        {
            if (entry.key == key)
            {
                return entry.value;
            }
        }

        if (m_size >= cap)
        {
            Rehash(cap * 2);
            cap = m_buckets.size();
            idx = HashKey(key) % cap;
        }

        m_buckets[idx].push_back(Entry{key, T{}});
        m_size++;
        return m_buckets[idx].back().value;
    }

    const T* Find(const string& key) const
    {
        int idx = HashKey(key) % m_buckets.size();
        for (const auto& entry:m_buckets[idx])
        {
            if (entry.key == key)
            {
                return &entry.value;
            }
        }
        return nullptr;
    }

    void Reserve(int expected)
    {
        int cap = m_buckets.size();
        while (cap < expected)
        {
            cap *= 2;
        }
        if (cap > m_buckets.size())
        {
            Rehash(cap);
        }
    }

    int Size() const
    {
        return m_size;
    }

    bool Empty() const
    {
        return m_size == 0;
    }

    void Clear()
    {
        for (auto& bucket:m_buckets)
        {
            bucket.clear();
        }
        m_size = 0;
    }

    template <typename Func>
    void ForEach(Func&& fn) const
    {
        for (const auto& bucket:m_buckets)
        {
            for (const auto& entry:bucket)
            {
                fn(entry.key, entry.value);
            }
        }
    }
};
