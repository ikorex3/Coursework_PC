#include "SearchApp.h"
#include <chrono>
#include <iostream>
using namespace std;

SearchApp::SearchApp(int port, const string& data_path) : indexer(&index, data_path)
{
    this->port = port;
}

crow::json::wvalue SearchApp::GetHealth()
{
    crow::json::wvalue res;
    res["status"] = "ok";
    res["ollama"] = llm.IsAlive();
    return res;
}

crow::json::wvalue SearchApp::PostIndex(int threads)
{
    auto start = chrono::steady_clock::now();
    int docs = indexer.BuildIndex(threads);
    auto end = chrono::steady_clock::now();
    long long time_ms = chrono::duration_cast<chrono::milliseconds>(end-start).count();
    last_build_time_ms = time_ms;
    last_threads_used = threads>0 ? threads:1;
    crow::json::wvalue res;
    res["status"] = "ok";
    res["documents_indexed"] = docs;
    res["time_ms"] = time_ms;
    return res;
}

crow::json::wvalue SearchApp::PostSearch(const string& query, const string& mode)
{
    auto start = chrono::steady_clock::now();
    vector<string> keywords;
    if (mode == "expanded")
    {
        keywords = llm.ExpandQuery(query);
    }
    else
    {
        keywords.push_back(query);
    }
    auto items = index.Search(keywords);
    auto end = chrono::steady_clock::now();
    long long time_ms = chrono::duration_cast<chrono::milliseconds>(end-start).count();
    vector<crow::json::wvalue> results_list;
    results_list.reserve(items.size());
    for (const auto& item:items)
    {
        crow::json::wvalue entry;
        entry["doc_id"] = item.id;
        entry["score"] = item.score;
        entry["snippet"] = item.text;
        results_list.push_back(move(entry));
    }
    crow::json::wvalue res;
    res["results"] = move(results_list);
    res["time_ms"] = time_ms;
    return res;
}

crow::json::wvalue SearchApp::GetStats()
{
    crow::json::wvalue res;
    res["documents"] = index.GetCount();
    res["index_build_time_ms"] = last_build_time_ms;
    res["threads_used"] = last_threads_used;
    return res;
}

void SearchApp::WebSocketSearch(crow::websocket::connection& conn, const string& query)
{
    if (query.empty())
    {
        crow::json::wvalue done_msg;
        done_msg["stage"] = "done";
        conn.send_text(done_msg.dump());
        return;
    }
    auto expanded = llm.ExpandQuery(query);
    vector<crow::json::wvalue> terms_list;
    terms_list.reserve(expanded.size());
    for (const auto& term:expanded)
    {
        terms_list.push_back(term);
    }

    crow::json::wvalue stage1;
    stage1["stage"] = "query_expansion";
    stage1["data"]["original"] = query;
    stage1["data"]["expanded_terms"] = move(terms_list);
    conn.send_text(stage1.dump());

    auto start = chrono::steady_clock::now();
    auto items = index.Search(expanded);
    auto end = chrono::steady_clock::now();
    long long time_ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    vector<crow::json::wvalue> results_list;
    results_list.reserve(items.size());
    for (const auto& item:items)
    {
        crow::json::wvalue entry;
        entry["doc_id"] = item.id;
        entry["score"] = item.score;
        entry["snippet"] = item.text;
        results_list.push_back(move(entry));
    }
    crow::json::wvalue stage2;
    stage2["stage"] = "keyword_search";
    stage2["data"]["results"] = move(results_list);
    stage2["data"]["time_ms"] = time_ms;
    conn.send_text(stage2.dump());
    crow::json::wvalue stage3;
    stage3["stage"] = "done";
    conn.send_text(stage3.dump());
}

void SearchApp::Run()
{
    crow::SimpleApp app;
    CROW_ROUTE(app, "/api/health")
    ([this]()
    {
        return GetHealth();
    });
    CROW_ROUTE(app, "/api/index").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& req)
    {
        int threads = 4;
        auto body = crow::json::load(req.body);
        if (body && body.has("threads"))
        {
            threads = static_cast<int>(body["threads"].i());
        }
        return crow::response(PostIndex(threads));
    });

    CROW_ROUTE(app, "/api/search").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& req)
    {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("query"))
        {
            return crow::response(400, "{\"error\":\"Missing query\"}");
        }
        string query = body["query"].s();
        string mode = body.has("mode") ? string(body["mode"].s()) : "keyword";
        return crow::response(PostSearch(query, mode));
    });

    CROW_ROUTE(app, "/api/stats")
    ([this]()
    {
        return GetStats();
    });

    CROW_WEBSOCKET_ROUTE(app, "/ws/search")
    .onmessage([this](crow::websocket::connection& conn, const string& data, bool is_binary)
    {
        auto body = crow::json::load(data);
        string query = (body && body.has("query")) ? string(body["query"].s()) : data;
        WebSocketSearch(conn, query);
    });

    cout << "Starting server on port " << port << "...\n";
    app.port(port).multithreaded().run();
}
