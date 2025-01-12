#pragma once

#include <string>
#include <vector>

#define NODEINDEX_UNDEFINED -3
#define NODEINDEX_VSOURCE   -2
#define NODEINDEX_GROUND    -1

#define PIN_PLUS      0
#define PIN_MINUS     1
#define PIN_ANODE     PIN_PLUS
#define PIN_CATHODE   PIN_MINUS

struct Net {
  std::string name;
  int         index;
  double      fi,
              fi_1;
public:
  Net(std::string &_name);
};

enum class ElementType {
  Unknown = 0,
  Resistor,
  Capacitor,
  Diode,
  VSource,
};

enum class SourceType {
  Unknown = 0,
  DC,
  Pulse, 
  Sine,
};

enum class AnalysisType {
  Unknown = 0,
  Tran,
};

class Element {
public:
  std::string         name;
  ElementType         type;
  std::vector<Net *>  pins;

  double              I, dI, value;
public:
  Element(ElementType _type, std::string &_name);
public:
  virtual void  initTran(double _time, double _step) = 0;
public:
  double  getI(Net *_p_net);
  void    fillI(double *_I);
  void    fillY(double **_Y);
};

class Resistor : public Element {
public:
  Resistor(std::string &_name);
public:
  void  initTran(double _time, double _step);
};

class Capacitor : public Element {
public:
  Capacitor(std::string &_name);
public:
  void  initTran(double _time, double _step);
};

class Diode : public Element {
public:
  Diode(std::string &_name);
public:
  void  initTran(double _time, double _step);
};

class VSource {
protected:
  double              value;
public:
  SourceType          type;
  std::string         name;
  std::vector<Net *>  pins;
public:
  VSource(SourceType _type, std::string _name);
public:
  virtual void initTran(double _time) = 0;
  double getV();
};

class VDC : public VSource {
public:
  double              dc;
public:
  VDC(std::string _name);
public:
  void    initTran(double _time);
};

class VPulse : public VSource {
public:
  double v0, v1, td, tr, tf, pw, per;
public:
  VPulse(std::string _name);
public:
  void    initTran(double _time);
};

class VSine : public VSource {
public:
  double v0, va, freq, td, df, phase;
public:
  VSine(std::string _name);
public:
  void    initTran(double _time);
};

struct Analysis {
  std::string   name;
  AnalysisType  type;
};

struct AnalysisTran : public Analysis {
  double  start,
          stop,
          step;
public:
  AnalysisTran(std::string &_name);
};

class Netlist {
public:
  std::string             fileName;
  std::vector<Net*>       nets;
  std::vector<Element*>   elements;
  std::vector<VSource*>   vsources;
  std::vector<Analysis*>  analyses;
public:
  Netlist();
 ~Netlist();
public:
  Net *AddNet(std::string &_name);
  Element *AddElement(std::string &_name);
  Net *GetNetByName(std::string &_name);
  Element *GetElementByName(std::string &_name);
  VSource *AddVSource(std::string _name, std::string _type);
  bool Postprocess();
  void PrintStatistics();
};

