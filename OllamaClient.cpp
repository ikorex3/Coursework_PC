#include "OllamaClient.h"
#include <httplib.h>

OllamaClient::OllamaClient() {
  const char *env = std::getenv("OLLAMA_HOST");
  url = env ? std::string("http://") + env : "http://localhost:11434";
  model = "llama3";
}

bool OllamaClient::IsAlive() {
  httplib::Client cli(url);
  cli.set_connection_timeout(1);
  auto res = cli.Get("/");
  return res && res->status == 200;
}

std::vector<std::string> OllamaClient::ExpandQuery(std::string text) {
  return {text};
}
