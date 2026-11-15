#include "DataIndexer.h"
#include <filesystem>
#include <fstream>
using namespace std;

DataIndexer::DataIndexer(IndexTable* index, const string& data_path)
{
    m_thread_pool = nullptr;
    m_index = index;
    m_data_path = data_path;
}

DataIndexer::~DataIndexer()
{
    if (m_thread_pool != nullptr)
    {
        delete m_thread_pool;
        m_thread_pool = nullptr;
    }
}

void DataIndexer::IndexFile(const string& file_path)
{
    ifstream file(file_path, ios::in | ios::binary | ios::ate);
    if (!file.is_open())
    {
        return;
    }

    auto size = file.tellg();
    if (size <= 0)
    {
        return;
    }

    string text(size, '\0');
    file.seekg(0);
    file.read(&text[0], size);

    string doc_id = filesystem::relative(file_path, m_data_path).generic_string();
    m_index->AddDoc(doc_id, text);
}

int DataIndexer::BuildIndex(int threads)
{
    if (threads <= 0)
    {
        threads = 1;
    }

    if (!filesystem::exists(m_data_path) || !filesystem::is_directory(m_data_path))
    {
        return 0;
    }

    m_index->Clear();
    m_thread_pool = new ThreadPool(threads);

    for (const auto& entry:filesystem::recursive_directory_iterator(m_data_path))
    {
        if (entry.is_regular_file())
        {
            string file_path = entry.path().string();
            m_thread_pool->AddTask([this, file_path]()
            {
                IndexFile(file_path);
            });
        }
    }

    delete m_thread_pool;
    m_thread_pool = nullptr;

    return m_index->GetCount();
}
