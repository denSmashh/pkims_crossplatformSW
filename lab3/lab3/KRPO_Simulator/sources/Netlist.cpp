#include "Netlist.hpp"

Net::Net(std::string &_name) : name(_name), index(NODEINDEX_UNDEFINED), fi(0.0), fi_1(0.0) {}

Element::Element(ElementType _type, std::string &_name) : name(_name), type(_type), I(0.0), dI(0.0), value(0.0) {
  pins.resize(2);
}

void Element::fillI(double *_I) {
  if (pins[PIN_PLUS]->index > NODEINDEX_GROUND)
    _I[pins[PIN_PLUS]->index] += I;
  if (pins[PIN_MINUS]->index > NODEINDEX_GROUND)
    _I[pins[PIN_MINUS]->index] -= I;
}

void Element::fillY(double **_Y) {
  int i = pins[PIN_PLUS]->index,
      j = pins[PIN_MINUS]->index;

  if (i > NODEINDEX_GROUND) {
    _Y[i][i] -= dI;
    if (j > NODEINDEX_GROUND)
      _Y[i][j] += dI;
  }
  if (j > NODEINDEX_GROUND) {
    _Y[j][j] -= dI;
    if (i > NODEINDEX_GROUND)
      _Y[j][i] += dI;
  }
}

double Element::getI(Net *_p_net) {
  if (!_p_net)
    return 0.0;
  if (pins[PIN_PLUS] == _p_net)
    return I;
  if (pins[PIN_MINUS] == _p_net)
    return -I;
  return 0.0;
}

Resistor::Resistor(std::string &_name) : Element(ElementType::Resistor, _name) {
  value = 1e+3;
}

void Resistor::initTran(double _time, double _step) {
  I = (pins[PIN_PLUS]->fi - pins[PIN_MINUS]->fi) / value;
  dI = 1.0 / value;
}

Capacitor::Capacitor(std::string &_name) : Element(ElementType::Capacitor, _name) {
  value = 1e-12;
}

void Capacitor::initTran(double _time, double _step) {
  I = value / _step * ((pins[PIN_PLUS]->fi - pins[PIN_MINUS]->fi) - (pins[PIN_PLUS]->fi_1 - pins[PIN_MINUS]->fi_1));
  dI = value / _step;
}

Diode::Diode(std::string &_name) : Element(ElementType::Diode, _name) {
  value = 1e-12;
}

void Diode::initTran(double _time, double _step) {
  I = value * (exp((pins[PIN_PLUS]->fi - pins[PIN_MINUS]->fi) / 0.025) - 1);
  dI = value / 0.025 * exp((pins[PIN_PLUS]->fi - pins[PIN_MINUS]->fi) / 0.025);
}

VSource::VSource(SourceType _type, std::string _name) : value(0.0), type(_type), name(_name) {
  pins.resize(2);
}

double VSource::getV() {
  return value;
}

VDC::VDC(std::string _name) : VSource(SourceType::DC, _name), dc(0.0) {
}

void VDC::initTran(double _time) {
  value = dc;

  if (pins[0]->index != NODEINDEX_GROUND)
    pins[0]->fi = value;
  if (pins[1]->index != NODEINDEX_GROUND)
    pins[0]->fi = -value;
}

VPulse::VPulse(std::string _name) : VSource(SourceType::Pulse, _name), v0(0.0), v1(0.0), td(0.0), tr(0.0), tf(0.0), pw(0.0), per(0.0) {
}

void VPulse::initTran(double _time) {
  if (_time <= td) {
    value = v0;
    goto RET_POINT;
  }
  _time -= td;
  if (_time > per)
    while (_time > per)
      _time -= per;
  if (_time <= tr) {
    value = v0 + _time * (v1 - v0) / tr;
    goto RET_POINT;
  }
  _time -= tr;
  if (_time <= pw) {
    value = v1;
    goto RET_POINT;
  }
  _time -= pw;
  if (_time < tf) {
    value = v1 + _time * (v0 - v1) / tf;
    goto RET_POINT;
  }
  value = v0;
RET_POINT:
  if (pins[0]->index != NODEINDEX_GROUND)
    pins[0]->fi = value;
  if (pins[1]->index != NODEINDEX_GROUND)
    pins[1]->fi = -value;
}

VSine::VSine(std::string _name) : VSource(SourceType::Sine, _name), v0(0.0), va(0.0), freq(0.0), td(0.0), df(0.0), phase(0.0) {
}

void VSine::initTran(double _time) {
  if (_time <= td)
    value = v0;
  else
    value = v0 + va * sin(2 * 3.14 * (freq * (_time - td) + phase / 360.0)) * exp(-(_time - td) * df);

  if (pins[0]->index != NODEINDEX_GROUND)
    pins[0]->fi = value;
  if (pins[1]->index != NODEINDEX_GROUND)
    pins[1]->fi = -value;
}

AnalysisTran::AnalysisTran(std::string &_name) : start(0.0), stop(0.0), step(0.0) {
  name = _name;
  type = AnalysisType::Tran;
}

Netlist::Netlist() {

}

Netlist::~Netlist() {
  for (size_t i = 0; i < nets.size(); ++i) {
    delete nets[i];
    nets[i] = nullptr;
  }
  nets.clear();
  for (size_t i = 0; i < elements.size(); ++i) {
    delete elements[i];
    elements[i] = nullptr;
  }
  elements.clear();
  for (size_t i = 0; i < vsources.size(); ++i) {
    delete vsources[i];
    vsources[i] = nullptr;
  }
  vsources.clear();
  for (size_t i = 0; i < analyses.size(); ++i) {
    delete analyses[i];
    analyses[i] = nullptr;
  }
  analyses.clear();
}

Net *Netlist::AddNet(std::string &_name) {
  Net *p_net = GetNetByName(_name);
  if(p_net)
    return p_net;

  p_net = new Net(_name);
  nets.push_back(p_net);

  return p_net;
}

Net *Netlist::GetNetByName(std::string &_name) {
  for(size_t i = 0; i < nets.size(); ++i)
    if(nets[i]->name == _name)
      return nets[i];
  return nullptr;
}

Element *Netlist::AddElement(std::string &_name) {
  Element *p_element = GetElementByName(_name);
  if (p_element)
    return p_element;

  switch (_name[0]) {
    case 'R':
    case 'r':
      p_element = new Resistor(_name);
      break;
    case 'C':
    case 'c':
      p_element = new Capacitor(_name);
      break;
    case 'D':
    case 'd':
      p_element = new Diode(_name);
      break;
    default:
      return nullptr;
  }
  elements.push_back(p_element);

  return p_element;
}

Element *Netlist::GetElementByName(std::string &_name) {
  for (size_t i = 0; i < elements.size(); ++i)
    if (elements[i]->name == _name)
      return elements[i];
  return nullptr;
}

VSource *Netlist::AddVSource(std::string _name, std::string _type) {
  for (size_t i = 0; i < vsources.size(); ++i) {
    if (vsources[i]->name == _name) {
      printf("Ошибка. Множественное определение источников. Источник с именем '%s' уже определён.\n", _name.c_str());
      return nullptr;
    }
  }

  VSource *p_vsource = nullptr;
  if (_type == std::string("pulse"))
    p_vsource = new VPulse(_name);
  else
    if (_type == std::string("sin"))
      p_vsource = new VSine(_name);
    else
      if (_type == std::string("dc"))
        p_vsource = new VDC(_name);
      else
        p_vsource = new VDC(_name);

  if (p_vsource)
    vsources.push_back(p_vsource);

  return p_vsource;
}

bool Netlist::Postprocess() {
  for (size_t i = 0; i < vsources.size(); ++i) {
    vsources[i]->pins[0]->index = NODEINDEX_VSOURCE;
    vsources[i]->pins[1]->index = NODEINDEX_GROUND;
  }
  for (size_t i = 0; i < nets.size(); ++i) {
    if (nets[i]->name == std::string("0"))
      nets[i]->index = NODEINDEX_GROUND;
    if (nets[i]->name == std::string("gnd"))
      nets[i]->index = NODEINDEX_GROUND;
  }
  int net_index = 0;
  for (size_t i = 0; i < nets.size(); ++i) {
    if (nets[i]->index == NODEINDEX_UNDEFINED)
      nets[i]->index = net_index++;
  }

  return true;
}

void Netlist::PrintStatistics() {
  size_t  r_count = 0,
    c_count = 0,
    d_count = 0,
    v_count = vsources.size();

  for (size_t i = 0; i < elements.size(); ++i)
    switch (elements[i]->type) {
    case ElementType::Resistor:
        ++r_count;
        break;
    case ElementType::Capacitor:
        ++c_count;
        break;
    case ElementType::Diode:
        ++d_count;
        break;
    }

  printf("Scheme statistics:\n");
  if (v_count)
    printf("  Voltage sources : %zd\n", v_count);
  if (r_count)
    printf("  Resistors       : %zd\n", r_count);
  if (c_count)
    printf("  Capacitors      : %zd\n", c_count);
  if (d_count)
    printf("  Diodes          : %zd\n", d_count);
  printf("\n");
}
