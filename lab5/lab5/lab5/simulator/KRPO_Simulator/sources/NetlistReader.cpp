#include "NetlistReader.hpp"

#include <iostream>
#include <ctime>
#include <sstream>
#include <set>

void printTime2(clock_t _time) {
    if (_time < 1000)
        std::cout << "Netlist reading finished in " << _time << " ms.\n\n";
    else {
        _time /= 1000;
        if (_time < 60)
            std::cout << "Netlist reading finished in " << _time << " sec.\n\n";
        else {
            _time /= 60;
            if (_time < 60)
                std::cout << "Netlist reading finished in " << _time << " min.\n\n";
            else {
                clock_t mins = _time % 60;
                _time /= 60;
                std::cout << "Netlist reading finished in " << _time << " hr " << mins << " min.\n\n";
            }
        }
    }
}

double smart_atof(std::string _value) {
    size_t last = _value.length() - 1;
    if (isdigit(_value[last]))
        return atof(_value.c_str());
    double ret_val = -1.0;
    switch (_value[last]) {
    case 'g':
        if (_value[last - 1] == 'e' && _value[last - 2] == 'm')
            ret_val = atof(_value.substr(0, last - 2).c_str()) * (1.0e+6);
        else
            ret_val = atof(_value.substr(0, last).c_str()) * (1.0e+9);
        break;
    case 'k':
        ret_val = atof(_value.substr(0, last).c_str()) * (1.0e+3);
        break;
    case 'm':
        ret_val = atof(_value.substr(0, last).c_str()) * (1.0e-3);
        break;
    case 'u':
        ret_val = atof(_value.substr(0, last).c_str()) * (1.0e-6);
        break;
    case 'n':
        ret_val = atof(_value.substr(0, last).c_str()) * (1.0e-9);
        break;
    case 'p':
        ret_val = atof(_value.substr(0, last).c_str()) * (1.0e-12);
        break;
    };
    return ret_val;
}


bool Netlistreader::readNetlist(std::string content, Netlist* _p_netlist, bool fromFile) {
    errors.clear();

    p_netlist = _p_netlist;
    if (fromFile) p_netlist->fileName = content;

    clock_t A = std::clock();

    bool tokenized = fromFile ? tokenizeFile(content) : tokenizeFileContent(content);
    if (!tokenized)
        return false;
    if (!updateTokens())
        return false;
    if (!parseTokens())
        return false;

    if (!p_netlist->Postprocess())
        return false;

    clock_t B = std::clock();

    // printTime2(B - A);

    // p_netlist->PrintStatistics();

    return true;
}

bool Netlistreader::tokenizeFileContent(const std::string& fileContent) {
    size_t npos = std::string::npos, line = 1, pos = 0;
    char buf[512];
    std::string s;
    Token t;

    std::istringstream fileStream(fileContent);

    while (fileStream.getline(buf, sizeof(buf))) {
        pos = 1;
        s = buf;

        // Trim leading whitespace
        npos = s.find_first_not_of(" \t\n\r");
        if (npos == std::string::npos) {
            ++line;
            continue;
        }
        if (npos)
            s.erase(0, npos);

        // Trim trailing whitespace
        npos = s.find_last_not_of(" \t\n\r");
        if (npos != std::string::npos)
            if (npos)
                s.erase(npos + 1);

        // Tokenize the line
        t.line = line;
        t.line_orig = line;
        int line_col_idx = 0;
        while (!s.empty()) {
            t.pos = pos;
            t.pos_orig = line_col_idx;
            npos = s.find_first_of(" \t()=:");
            if (npos == std::string::npos) {
                t.token = s;
                tokens.push_back(t);
                s.erase();
            }
            else if (npos == 0) {
                t.token = s.substr(0, 1);
                tokens.push_back(t);

                s.erase(0, 1);
                line_col_idx++;

                npos = s.find_first_not_of(" \t\n\r");

                s.erase(0, npos);
                line_col_idx += npos;
            }
            else {
                t.token = s.substr(0, npos);
                tokens.push_back(t);

                s.erase(0, npos);
                line_col_idx += npos;

                npos = s.find_first_not_of(" \t\n\r");

                s.erase(0, npos);
                line_col_idx += npos;
            }
            ++pos;
        }

        ++line;
    }

    return true;
}

bool Netlistreader::tokenizeFile(const std::string& fileName) {
    //FILE* p_file = fopen(fileName.c_str(), "rt");
    FILE* p_file = nullptr;
    errno_t err = fopen_s(&p_file, fileName.c_str(), "rt");
    if (!p_file) {
        std::cout << "__error__ : Can't open specified file '" << fileName << "'." << std::endl << std::endl;
        return false;
    }
    std::cout << "Parsing netlist '" << fileName << "'... ";

    std::string fileContent;
    char buf[512];

    while (fgets(buf, sizeof(buf), p_file)) {
        fileContent += buf;
    }

    fclose(p_file);

    bool result = tokenizeFileContent(fileContent);

    std::cout << "done." << std::endl;
    return result;
}

bool Netlistreader::updateTokens() {
    std::set<int> comments;

    for (auto& tok : tokens) {
        if ((tok.token[0] == '*' && tok.pos == 1) || (tok.token[0] == '$')) {
            comments.insert(tok.line);
        }
    }

    tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [&](Token& token) {
        return comments.count(token.line);
        }), tokens.end());

    for (size_t i = 0; i < tokens.size(); ++i)
        for (size_t j = 0; j < tokens[i].token.length(); ++j)
            tokens[i].token[j] = (char)tolower(tokens[i].token[j]);

    return true;
}

bool Netlistreader::parseTokens() {
    size_t line = 0;
    Element* p_e = nullptr;
    VSource* p_v = nullptr;

    auto genError = [this](Token& tok, size_t size = 0) {
        errors.push_back({
           tok.line_orig,
           tok.pos_orig,
           size == 0 ? tok.token.size() : size,
           tok.token,
            });
        };

    // � ��� ����� ���� ����� ��� �������� - ��� �������, ��� ���������
    for (size_t i = 0; i < tokens.size(); ++i) {
        switch (tokens[i].token[0]) {
        case 'r':
            if (i + 3 >= tokens.size()) {
                genError(tokens[i]);
                return false;
            }
            p_e = p_netlist->AddElement(tokens[i++].token);
            if (!p_e) {
                genError(tokens[i]);
                return false;
            }
            p_e->pins[0] = p_netlist->AddNet(tokens[i++].token);
            p_e->pins[1] = p_netlist->AddNet(tokens[i++].token);
            static_cast<Resistor*>(p_e)->value = smart_atof(tokens[i].token.c_str());
            break;
        case 'c':
            if (i + 3 >= tokens.size()) {
                genError(tokens[i]);
                return false;
            }
            p_e = p_netlist->AddElement(tokens[i++].token);
            if (!p_e) {
                genError(tokens[i]);
                return false;
            }
            p_e->pins[0] = p_netlist->AddNet(tokens[i++].token);
            p_e->pins[1] = p_netlist->AddNet(tokens[i++].token);
            static_cast<Capacitor*>(p_e)->value = smart_atof(tokens[i].token.c_str());
            break;
        case 'd':
            if (i + 3 >= tokens.size()) {
                genError(tokens[i]);
                return false;
            }
            p_e = p_netlist->AddElement(tokens[i++].token);
            if (!p_e) {
                genError(tokens[i]);
                return false;
            }
            p_e->pins[0] = p_netlist->AddNet(tokens[i++].token);
            p_e->pins[1] = p_netlist->AddNet(tokens[i++].token);
            --i;
            //static_cast<Diode *>(p_e)->Io_value = smart_atof(tokens[i].token.c_str());
            break;
        case 'v':
            if (i + 3 >= tokens.size()) {
                genError(tokens[i]);
                return false;
            }
            p_v = p_netlist->AddVSource(tokens[i].token, tokens[i + 3].token);
            if (!p_v) {
                genError(tokens[i]);
                return false;
            }
            p_v->pins[0] = p_netlist->AddNet(tokens[i + 1].token);
            p_v->pins[1] = p_netlist->AddNet(tokens[i + 2].token);
            switch (p_v->type) {
            case SourceType::Pulse:
                if (i + 10 >= tokens.size()) {
                    genError(tokens[i]);
                    return false;
                }
                static_cast<VPulse*>(p_v)->v0 = smart_atof(tokens[i + 4].token);
                static_cast<VPulse*>(p_v)->v1 = smart_atof(tokens[i + 5].token);
                static_cast<VPulse*>(p_v)->td = smart_atof(tokens[i + 6].token);
                static_cast<VPulse*>(p_v)->tr = smart_atof(tokens[i + 7].token);
                static_cast<VPulse*>(p_v)->tf = smart_atof(tokens[i + 8].token);
                static_cast<VPulse*>(p_v)->pw = smart_atof(tokens[i + 9].token);
                static_cast<VPulse*>(p_v)->per = smart_atof(tokens[i + 10].token);
                i += 10;
                break;
            case SourceType::DC:
                if (i + 4 >= tokens.size()) {
                    genError(tokens[i]);
                    return false;
                }
                if (tokens[i + 3].token == "dc") {
                    ((VDC*)p_v)->dc = smart_atof(tokens[i + 4].token);
                    i += 4;
                }
                else {
                    ((VDC*)p_v)->dc = smart_atof(tokens[i + 3].token);
                    i += 3;
                }
                break;
            case SourceType::Sine:
                if (i + 9 >= tokens.size()) {
                    genError(tokens[i]);
                    return false;
                }
                static_cast<VSine*>(p_v)->v0 = smart_atof(tokens[i + 4].token);
                static_cast<VSine*>(p_v)->va = smart_atof(tokens[i + 5].token);
                static_cast<VSine*>(p_v)->freq = smart_atof(tokens[i + 6].token);
                static_cast<VSine*>(p_v)->td = smart_atof(tokens[i + 7].token);
                static_cast<VSine*>(p_v)->df = smart_atof(tokens[i + 8].token);
                static_cast<VSine*>(p_v)->phase = smart_atof(tokens[i + 9].token);
                i += 9;
                break;
            }
            break;
        case '.':
            if (!parseDirective(i))
            {
                genError(tokens[i]);
                return false;
            }
            break;
        default: {
            genError(tokens[i]);
            return false;
        }
        }
    }
    return true;
}

bool Netlistreader::parseDirective(size_t& _index) {
    if (tokens[_index].token == std::string(".tran")) {
        if (_index + 2 >= tokens.size()) {
            return false;
        }

        AnalysisTran* p_tran = new AnalysisTran(tokens[_index].token);
        p_netlist->analyses.push_back(p_tran);
        p_tran->step = smart_atof(tokens[++_index].token);
        p_tran->stop = smart_atof(tokens[++_index].token);

    }

    return true;
}
