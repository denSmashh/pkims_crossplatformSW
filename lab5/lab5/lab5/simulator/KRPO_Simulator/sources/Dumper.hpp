#pragma once

#include <vector>
#include <string>
#include <fstream>

enum class ViewpointType {
  Unknown = 0,
  Voltage,
  Current,
};

struct Viewpoint {
  std::string   name;
  ViewpointType type;
  double       *valuePtr;
};

class Dumper {
protected:
  std::string             fileName;
  std::ofstream			  sim_log;
  std::vector<Viewpoint>  viewpoints;
public:
  Dumper();
public:
  void AddViewpoint(std::string &_name, ViewpointType _vpt, double *_val) {
    Viewpoint vp;
    vp.name = _name;
    vp.type = _vpt;
    vp.valuePtr = _val;
    viewpoints.push_back(vp);
  };
public:
  virtual void BeginDump(std::string &_fileName) = 0;
  virtual void WriteHeader() = 0;
  virtual void WriteValuesAtTime(double _t) = 0;
  virtual void WriteValues(std::vector<double>& currValuesPtr) = 0;
  virtual void EndDump() = 0;
};
