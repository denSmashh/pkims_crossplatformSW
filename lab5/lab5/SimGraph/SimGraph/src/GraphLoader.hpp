#pragma once

#include "Graph.hpp"

class GraphLoader {
public:
  static Graphs loadFromFile(const std::string& filename);

private:
  static Graphs loadCSV(const std::string& filename);
};
