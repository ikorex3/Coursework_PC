#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <shared_mutex>

#include "HashMap.h"

using namespace std;

struct IndexItem
{
    string doc_id;
    int count = 0;
};

struct SearchItem
{
    string id;
    double score = 0.0;
    string text;
};

class IndexTable
{
private:
    HashMap<vector<IndexItem>> m_table_data;
    HashMap<string> m_doc_snippets;
    int m_total_docs = 0;
    mutable shared_mutex m_mutex;

public:
    IndexTable() = default;
    ~IndexTable() = default;
    IndexTable(const IndexTable&) = delete;
    IndexTable& operator=(const IndexTable&) = delete;

    static vector<string> TokenizeAndNormalize(const string& text);
    void AddDoc(const string& doc_id, const string& text);
    vector<SearchItem> Search(const vector<string>& keywords) const;
    int GetCount() const;
    void Clear();
};
