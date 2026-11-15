#include "crow/json.h"
#include <httplib.h>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Usage: ./search \"<query>\" [--expanded]\n";
        return 1;
    }
    string query = argv[1];
    string mode = "keyword";
    if (argc >= 3 && string(argv[2]) == "--expanded")
    {
        mode = "expanded";
    }
    const char* env_url = getenv("SEARCH_URL");
    string url = env_url ? env_url : "http://localhost:8080";
    httplib::Client cli(url);
    cli.set_connection_timeout(3);
    cli.set_read_timeout(120);
    crow::json::wvalue body;
    body["query"] = query;
    body["mode"] = mode;
    auto res = cli.Post("/api/search", body.dump(), "application/json");
    if (!res)
    {
        cerr << "Error: failed to connect to " << url << "\n";
        return 1;
    }
    if (res->status != 200)
    {
        cerr << "Error: server returned status " << res->status << "\n";
        return 1;
    }
    auto data = crow::json::load(res->body);
    if (!data || !data.has("results"))
    {
        cerr << "Error: invalid server response\n";
        return 1;
    }
    auto results = data["results"];
    long long time_ms = data.has("time_ms") ? data["time_ms"].i():0;
    cout << "Found " << results.size() << " results in " << time_ms << " ms (mode: " << mode << ")\n\n";
    for (int i = 0; i < results.size(); ++i)
    {
        string doc_id = results[i]["doc_id"].s();
        double score = results[i]["score"].d();
        string snippet = results[i]["snippet"].s();

        cout << (i + 1) << ". " << doc_id << " (score: " << fixed << setprecision(2) << score << ")\n";
        cout << "   " << snippet << "\n\n";
    }
    return 0;
}
