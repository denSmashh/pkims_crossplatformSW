#pragma once

#include <vector>
#include <string>

struct Graph {
  std::string name;

  std::vector<double> x;
  std::vector<double> y;

  std::string xName = "y";
  std::string yName = "x";

  explicit operator bool() const {
    return !x.empty() && !y.empty() && x.size() == y.size();
  }
};

using Graphs = std::vector<Graph>;