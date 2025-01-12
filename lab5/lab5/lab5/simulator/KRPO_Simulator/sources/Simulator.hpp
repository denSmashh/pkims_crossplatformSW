#pragma once

#include "Netlist.hpp"
#include "Dumper.hpp"
#include "Solver.hpp"

class Simulator {
  Netlist *p_netlist;
public:
  Simulator(Netlist *_p_netlist);
public:
  bool runTran(AnalysisTran *_p_tran, Solver *p_solver, Dumper *p_dumper);
};