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

struct ParseError {
    size_t line = 0;
    size_t col = 0;
    size_t size = 0;
    std::string tok;
};

class Netlistreader {
    Netlist* p_netlist;
    std::vector<Token>  tokens;
public:
    bool readNetlist(std::string content, Netlist* _p_netlist, bool fromFile = true);

    std::vector<ParseError> errors;

private:
    bool tokenizeFileContent(const std::string& fileContent);
    bool tokenizeFile(const std::string& _file_name);
    bool updateTokens();
    bool parseTokens();

    bool parseDirective(size_t& _index);
};
