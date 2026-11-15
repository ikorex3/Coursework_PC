#pragma once
#include <string>
#include "ThreadPool.h"
#include "IndexTable.h"
using namespace std;

class DataIndexer
{
private:
    ThreadPool* m_thread_pool;
    IndexTable* m_index;
    string m_data_path;
    void IndexFile(const string& file_path);

public:
    DataIndexer(IndexTable* index, const string& data_path = "./data");
    ~DataIndexer();
    int BuildIndex(int threads);
};
