#pragma once
#include <string>
#include <vector>

class OllamaClient {
private:
  std::string url;
  std::string model;

public:
  OllamaClient();
  bool IsAlive();
  std::vector<std::string> ExpandQuery(std::string text);
};
