#pragma once

class Solver {
public:
  virtual void Solve(double **Y, double *x, double *I, int sleSize) = 0;
};