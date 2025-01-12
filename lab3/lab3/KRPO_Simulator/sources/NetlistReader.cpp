#include "netlistreader.hpp"

#include <iostream>
#include <ctime>

void printTime2(clock_t _time) {
  if (_time < 1000)
    printf("Netlist reading finished in %ld ms.\n\n", _time);
  else {
    _time /= 1000;
    if (_time < 60)
      printf("Netlist reading finished in %ld sec.\n\n", _time);
    else {
      _time /= 60;
      if (_time < 60)
        printf("Netlist reading finished in %ld min.\n\n", _time);
      else {
        clock_t mins = _time % 60;
        _time /= 60;
        printf("Netlist reading finished in %ld hr %ld min.\n\n", _time, mins);
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
        ret_val = atof(_value.substr(0, last - 2).c_str())*(1.0e+6);
      else
        ret_val = atof(_value.substr(0, last).c_str())*(1.0e+9);
      break;
    case 'k': 
      ret_val = atof(_value.substr(0, last).c_str())*(1.0e+3);
      break;
    case 'm': 
      ret_val = atof(_value.substr(0, last).c_str())*(1.0e-3);
      break;
    case 'u': 
      ret_val = atof(_value.substr(0, last).c_str())*(1.0e-6);
      break;
    case 'n': 
      ret_val = atof(_value.substr(0, last).c_str())*(1.0e-9);
      break;
    case 'p': 
      ret_val = atof(_value.substr(0, last).c_str())*(1.0e-12);
      break;
  };
  return ret_val;
}


bool Netlistreader::readNetlist(std::string _fileName, Netlist *_p_netlist) {
  p_netlist = _p_netlist;
  p_netlist->fileName = _fileName;

  clock_t A = std::clock();

  if(!tokenizeFile(_fileName))
    return false;
  if (!updateTokens())
    return false;
  if(!parseTokens())
    return false;
  
  if(!p_netlist->Postprocess())
    return false;

  clock_t B = std::clock();

  printTime2(B - A);

  p_netlist->PrintStatistics();

  return true;
}

bool Netlistreader::tokenizeFile(std::string _file_name) {
  FILE *p_file = fopen(_file_name.c_str(), "rt");
  if (!p_file) {
    std::cout << "__error__ : Can't open specified file '" << _file_name << "'." << std::endl << std::endl;
    return false;
  }
  std::cout << "Parsing netlist '" << _file_name << "'... ";

  size_t npos = std::string::npos, line = 1, pos = 0;
  char buf[512];
  std::string s;
  Token t;

  while (!feof(p_file)) {
    pos = 1;
    buf[0] = '\0';
    fgets(buf, 510, p_file);
    s = buf;
    // Начинаем парсить строку

    // Чистим строку в начале
    npos = s.find_first_not_of(" \t\n");
    if (npos == std::string::npos) {
      ++line;
      continue;
    }
    if (npos)
      s.erase(0, npos);

    // Чистим строку в конце
    npos = s.find_last_not_of(" \t\n");
    if (npos != std::string::npos)
      if (npos)
        s.erase(npos + 1);

    // Ловим токены
    t.line = line;
    t.line_orig = line;
    while (!s.empty()) {
      t.pos = pos;
      t.pos_orig = pos;
      npos = s.find_first_of(" \t()=:");
      if (npos == std::string::npos) {
        t.token = s;
        tokens.push_back(t);
        s.erase();
      }
      else
        if (npos == 0) {
          t.token = s.substr(0, 1);
          tokens.push_back(t);
          s.erase(0, 1);
          npos = s.find_first_not_of(" \t\n");
          s.erase(0, npos);
        }
        else {
          t.token = s.substr(0, npos);
          tokens.push_back(t);
          s.erase(0, npos);
          npos = s.find_first_not_of(" \t\n");
          s.erase(0, npos);
        }
        ++pos;
    }
    // Закончили парсить
    ++line;
  }

  fclose(p_file);

  // Сохраняем оригинальное положение в файле
  for (size_t i = 0; i < tokens.size(); ++i) {
    t.line_orig = t.line;
    t.pos_orig = t.pos;
  }

  std::cout << "done." << std::endl;
  return true;
}

bool Netlistreader::updateTokens() {
  size_t line;
  // Убрать комментарии SPICE (*) и HSPICE ($)
  for (size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i].token[0] == '*' && tokens[i].pos == 1) {
      line = tokens[i].line;
      while (tokens[i].line == line)
        tokens.erase(tokens.begin() + i);
      --i;
    }
    else
      if (tokens[i].token[0] == '$') {
        line = tokens[i].line;
        while (tokens[i].line == line)
          tokens.erase(tokens.begin() + i);
        --i;
      }
  }

  for (size_t i = 0; i < tokens.size(); ++i)
    for (size_t j = 0; j < tokens[i].token.length(); ++j)
      tokens[i].token[j] = (char)tolower(tokens[i].token[j]);
  return true;
}

bool Netlistreader::parseTokens() {
  size_t line = 0;
  Element *p_e = nullptr;
  VSource *p_v = nullptr;
  // У нас могут быть всего две ситуации - или элемент, или директива
  for (size_t i = 0; i < tokens.size(); ++i) {
    switch (tokens[i].token[0]) {
      case 'r': 
        p_e = p_netlist->AddElement(tokens[i++].token);
        if (!p_e)
          return false;
        p_e->pins[0] = p_netlist->AddNet(tokens[i++].token);
        p_e->pins[1] = p_netlist->AddNet(tokens[i++].token);
        static_cast<Resistor *>(p_e)->value = smart_atof(tokens[i].token.c_str());
        break;
      case 'c': 
        p_e = p_netlist->AddElement(tokens[i++].token);
        if (!p_e)
          return false;
        p_e->pins[0] = p_netlist->AddNet(tokens[i++].token);
        p_e->pins[1] = p_netlist->AddNet(tokens[i++].token);
        static_cast<Capacitor *>(p_e)->value = smart_atof(tokens[i].token.c_str());
        break;
      case 'd':
        p_e = p_netlist->AddElement(tokens[i++].token);
        if (!p_e)
          return false;
        p_e->pins[0] = p_netlist->AddNet(tokens[i++].token);
        p_e->pins[1] = p_netlist->AddNet(tokens[i++].token);
        --i;
        //static_cast<Diode *>(p_e)->Io_value = smart_atof(tokens[i].token.c_str());
        break;
      case 'v': 
        p_v = p_netlist->AddVSource(tokens[i].token, tokens[i + 3].token);
        if (!p_v)
          return false;
        p_v->pins[0] = p_netlist->AddNet(tokens[i + 1].token);
        p_v->pins[1] = p_netlist->AddNet(tokens[i + 2].token);
        switch (p_v->type) {
          case SourceType::Pulse: 
            static_cast<VPulse *>(p_v)->v0 = smart_atof(tokens[i + 4].token);
            static_cast<VPulse *>(p_v)->v1 = smart_atof(tokens[i + 5].token);
            static_cast<VPulse *>(p_v)->td = smart_atof(tokens[i + 6].token);
            static_cast<VPulse *>(p_v)->tr = smart_atof(tokens[i + 7].token);
            static_cast<VPulse *>(p_v)->tf = smart_atof(tokens[i + 8].token);
            static_cast<VPulse *>(p_v)->pw = smart_atof(tokens[i + 9].token);
            static_cast<VPulse *>(p_v)->per = smart_atof(tokens[i + 10].token);
            i += 10;
            break;
          case SourceType::DC:    
            if (tokens[i + 3].token == "dc") {
              ((VDC *)p_v)->dc = smart_atof(tokens[i + 4].token);
              i += 4;
            } else {
              ((VDC *)p_v)->dc = smart_atof(tokens[i + 3].token);
              i += 3;
            }
            break;
          case SourceType::Sine:
            static_cast<VSine *>(p_v)->v0 = smart_atof(tokens[i + 4].token);
            static_cast<VSine *>(p_v)->va = smart_atof(tokens[i + 5].token);
            static_cast<VSine *>(p_v)->freq = smart_atof(tokens[i + 6].token);
            static_cast<VSine *>(p_v)->td = smart_atof(tokens[i + 7].token);
            static_cast<VSine *>(p_v)->df = smart_atof(tokens[i + 8].token);
            static_cast<VSine *>(p_v)->phase = smart_atof(tokens[i + 9].token);
            i += 9;
            break;
        }
        break;
      case '.': 
        if (!parseDirective(i))
          return false;
        break;
      default: printf("Ошибка. Неподдерживаемый элемент '%s' встретился на позиции (%3d, %3d).\n", tokens[i].token.c_str(), tokens[i].line_orig, tokens[i].pos_orig);
    }
  }
  return true;
}

bool Netlistreader::parseDirective(size_t &_index) {
  if (tokens[_index].token == std::string(".tran")) {
    AnalysisTran *p_tran = new AnalysisTran(tokens[_index].token);
    p_netlist->analyses.push_back(p_tran);
    p_tran->step = smart_atof(tokens[++_index].token);
    p_tran->stop = smart_atof(tokens[++_index].token);
  }
  return true;
}
