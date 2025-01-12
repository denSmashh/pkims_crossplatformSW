#pragma once

#include <string>

enum class PluginType {
  solver = 1,
  dumper,
};

class Solver {
public:
  virtual void Solve(double **Y, double *x, double *I, int sleSize) = 0;
};


class Solver_Jacobi : public Solver {
public:
  void Solve(double **Y, double *x, double *I, int sleSize);
};


__declspec(dllexport) PluginType GetType();
__declspec(dllexport) void GetStringID(std::string &_id);
__declspec(dllexport) Solver *GetSolver();
__declspec(dllexport) void FreeSolver(Solver *_p_solver);
