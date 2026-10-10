#include "crow.h"
#include "ThreadPool.h"
#include "OllamaClient.h"
#include <iostream>

int main()
{
    crow::SimpleApp app;
    OllamaClient ollama;

    CROW_ROUTE(app, "/api/health")
    ([&ollama]() {
        crow::json::wvalue res;
        res["status"] = "ok";
        res["ollama"] = ollama.IsAlive();
        return res;
    });

    std::cout << "Starting server on port 8080...\n";
    app.port(8080).multithreaded().run();
    return 0;
}