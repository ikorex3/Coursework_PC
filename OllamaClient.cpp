#include "OllamaClient.h"
#include "crow/json.h"
#include <httplib.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>

using namespace std;

OllamaClient::OllamaClient()
{
    const char* env = getenv("OLLAMA_HOST");
    url = env ? string("http://") + env:"http://localhost:11434";

    const char* model_env = getenv("OLLAMA_MODEL");
    model = model_env ? model_env:"llama3.2:1b";
}

bool OllamaClient::IsAlive()
{
    httplib::Client cli(url);
    cli.set_connection_timeout(1);
    auto res = cli.Get("/");
    return res && res->status==200;
}

vector<string> OllamaClient::ExpandQuery(string text)
{
    vector<string> expanded_words;
    expanded_words.push_back(text);
    httplib::Client cli(url);
    cli.set_connection_timeout(2);
    cli.set_read_timeout(30);
    crow::json::wvalue body;
    body["model"] = model;
    body["prompt"] = "Extract 5 different single-word synonyms for the query: \"" + text + "\". Return ONLY a comma-separated list of words without any explanations or punctuation.";
    body["stream"] = false;
    auto res = cli.Post("/api/generate", body.dump(), "application/json");
    if (!res || res->status != 200)
    {
        return expanded_words;
    }
    auto json_res = crow::json::load(res->body);
    if (!json_res || !json_res.has("response"))
    {
        return expanded_words;
    }
    string response_text = json_res["response"].s();
    size_t colon = response_text.find(':');
    if (colon != string::npos)
    {
        response_text = response_text.substr(colon + 1);
    }
    stringstream ss(response_text);
    string item;
    while (getline(ss, item, ','))
    {
        string cleaned;
        for (unsigned char ch:item)
        {
            if (isalnum(ch) || ch == ' ')
            {
                cleaned.push_back(static_cast<char>(tolower(ch)));
            }
        }
        size_t start = cleaned.find_first_not_of(' ');
        size_t end = cleaned.find_last_not_of(' ');
        if (start != string::npos)
        {
            string word = cleaned.substr(start, end - start + 1);
            if (!word.empty() && find(expanded_words.begin(), expanded_words.end(), word)==expanded_words.end())
            {
                expanded_words.push_back(word);
            }
        }
    }
    return expanded_words;
}
