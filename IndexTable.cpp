#include "IndexTable.h"

#include <algorithm>
#include <cctype>
#include <cmath>

using namespace std;

vector<string> IndexTable::TokenizeAndNormalize(const string& text)
{
    vector<string> tokens;
    string current;

    for (unsigned char ch:text)
    {
        if (isalnum(ch))
        {
            current.push_back(static_cast<char>(tolower(ch)));
        }
        else
        {
            if (!current.empty())
            {
                if (current.length() >= 2)
                {
                    tokens.push_back(current);
                }
                current.clear();
            }
        }
    }

    if (!current.empty()&&current.length() >= 2)
    {
        tokens.push_back(current);
    }

    return tokens;
}

void IndexTable::AddDoc(const string& doc_id, const string& text)
{
    auto tokens = TokenizeAndNormalize(text);
    HashMap<int> local_counts;
    local_counts.Reserve(tokens.size());

    for (const auto& token:tokens)
    {
        local_counts[token]++;
    }

    string snippet;
    size_t snippet_len = min<size_t>(text.size(), 160);
    snippet.reserve(snippet_len + 3);

    for (size_t i = 0; i<snippet_len; ++i)
    {
        char ch = text[i];
        if (ch == '\r' || ch == '\n' || ch == '\t')
        {
            if (!snippet.empty()&&snippet.back() != ' ')
            {
                snippet.push_back(' ');
            }
        }
        else
        {
            snippet.push_back(ch);
        }
    }

    if (text.size()>snippet_len)
    {
        snippet += "...";
    }

    unique_lock<shared_mutex> lock(m_mutex);

    local_counts.ForEach([&](const string& term, int count)
    {
        m_table_data[term].push_back(IndexItem{doc_id, count});
    });

    m_doc_snippets[doc_id] = move(snippet);
    m_total_docs++;
}

vector<SearchItem> IndexTable::Search(const vector<string>& keywords) const
{
    vector<string> search_terms;
    for (const auto& kw:keywords)
    {
        auto tokens = TokenizeAndNormalize(kw);
        search_terms.insert(search_terms.end(), tokens.begin(), tokens.end());
    }

    if (search_terms.empty())
    {
        return {};
    }

    sort(search_terms.begin(), search_terms.end());
    search_terms.erase(unique(search_terms.begin(), search_terms.end()), search_terms.end());

    shared_lock<shared_mutex> lock(m_mutex);

    if (m_total_docs == 0)
    {
        return {};
    }

    const double total_docs_f = static_cast<double>(m_total_docs);

    HashMap<double> doc_scores;
    double max_raw_score = 0.0;

    for (const auto& term:search_terms)
    {
        const auto* postings = m_table_data.Find(term);
        if (postings != nullptr)
        {
            double doc_freq = static_cast<double>(postings->size());
            double idf = log(1.0 + (total_docs_f / doc_freq));

            for (const auto& item:*postings)
            {
                double tf = 1.0 + log(static_cast<double>(item.count));
                double tf_idf = tf * idf;

                double& score = doc_scores[item.doc_id];
                score += tf_idf;
                if (score > max_raw_score)
                {
                    max_raw_score = score;
                }
            }
        }
    }

    if (doc_scores.Empty() || max_raw_score <= 0.0)
    {
        return {};
    }

    vector<SearchItem> results;
    results.reserve(doc_scores.Size());

    doc_scores.ForEach([&](const string& doc_id, double raw_score)
    {
        double normalized_score = round((raw_score / max_raw_score) * 100.0) / 100.0;

        string snippet;
        const string* snip = m_doc_snippets.Find(doc_id);
        if (snip != nullptr)
        {
            snippet = *snip;
        }

        results.push_back(SearchItem{doc_id, normalized_score, move(snippet)});
    });

    sort(results.begin(), results.end(), [](const SearchItem& a, const SearchItem& b)
    {
        return a.score > b.score;
    });

    if (results.size() > 50)
    {
        results.resize(50);
    }
    return results;
}

int IndexTable::GetCount() const
{
    shared_lock<shared_mutex> lock(m_mutex);
    return m_total_docs;
}

void IndexTable::Clear()
{
    unique_lock<shared_mutex> lock(m_mutex);
    m_table_data.Clear();
    m_doc_snippets.Clear();
    m_total_docs = 0;
}
