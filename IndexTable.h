#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

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
    unordered_map<string, vector<IndexItem>> m_table_data;
    unordered_map<string, string> m_doc_snippets;
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
