#pragma once

#include <string>
#include <vector>

#include "Netlist.hpp"

struct Token {
  std::string token;
  size_t      line,
              pos;
  size_t      line_orig,
              pos_orig;
};

class Netlistreader {
  Netlist            *p_netlist;
  std::vector<Token>  tokens;
public:
  bool readNetlist(std::string _file_name, Netlist *_p_netlist);
private:
  bool tokenizeFile(std::string _file_name);
  bool updateTokens();
  bool parseTokens();

  bool parseDirective(size_t &_index);
};
