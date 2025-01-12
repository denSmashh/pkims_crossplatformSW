#include "GraphLoader.hpp"

#include <fstream>
#include <unordered_map>
#include <sstream>
#include <filesystem>

Graphs GraphLoader::loadFromFile(const std::string &filename) {
  std::filesystem::path path(filename);
  if (!exists(path)) return {};

  try {
    if (path.extension() == ".csv") {
      return loadCSV(filename);
    }
  } catch (const std::exception& exception) {
    return {};
  }

  return {};
}

Graphs GraphLoader::loadCSV(const std::string &filename) {
  Graphs out;

  std::ifstream file(filename);
  std::string line;

  bool readingValues = false;
  while (std::getline(file, line)) {
    std::istringstream tokStream(line);

    if (line.rfind("TIME", 0) == 0) {
      std::string token;
      tokStream >> token;

      while (tokStream >> token) {
        out.push_back({token, {}, {}, std::string(token.begin(), token.begin() + 1), "t"});
      }
      readingValues = true;
      continue;
    }

    if (!readingValues) continue;

    auto numSamples = out.size();

    double time = 0;
    tokStream >> time;

    for (auto idx = 0; idx < numSamples; idx++) {
      double val = 0;
      tokStream >> val;
      out[idx].x.push_back(time);
      out[idx].y.push_back(val);
    }
  }

  return out;
}
