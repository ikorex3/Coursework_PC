#include "crow.h"
#include <iostream>

int main()
{
    crow::SimpleApp app;
    CROW_ROUTE(app, "/api/health")
    ([](){
        return crow::json::wvalue{{"status","ok"}};
    });
    app.port(8080).multithreaded().run();
    return 0;
}