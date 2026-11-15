#pragma once
#include "crow.h"
#include "IndexTable.h"
#include "DataIndexer.h"
#include "OllamaClient.h"
#include <string>
using namespace std;

class SearchApp
{
private:
    IndexTable index;
    DataIndexer indexer;
    OllamaClient llm;
    int port;
    long long last_build_time_ms = 0;
    int last_threads_used = 0;

public:
    SearchApp(int port = 8080, const string& data_path = "./data");
    void Run();
    crow::json::wvalue GetHealth();
    crow::json::wvalue PostIndex(int threads);
    crow::json::wvalue PostSearch(const string& query, const string& mode);
    crow::json::wvalue GetStats();
    void WebSocketSearch(crow::websocket::connection& conn, const string& query);
};
