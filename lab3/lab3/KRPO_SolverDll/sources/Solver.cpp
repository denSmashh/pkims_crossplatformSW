#include "Solver.hpp"


void Solver_Jacobi::Solve(double **Y, double *x, double *I, int sleSize) {
    double* x_new = new double[sleSize];
    std::memcpy(x_new, x, sleSize * sizeof(double));

    for (int i = 0; i < sleSize; ++i) {
        double sum = 0.0;
        for (int j = 0; j < sleSize; ++j) {
            if (i != j) {
                sum += Y[i][j] * x[j];
            }
        }
        x_new[i] = (I[i] - sum) / Y[i][i];
    }

    std::memcpy(x, x_new, sleSize * sizeof(double));

    delete[] x_new;
}


PluginType GetType() {
  return PluginType::solver;
}

void GetStringID(std::string &_id) {
  _id = "0xSolver_Jacobi_ID";
}

Solver *GetSolver() {
  return new Solver_Jacobi;
}

void FreeSolver(Solver *_p_solver) {
  if (_p_solver) {
    delete _p_solver;
    _p_solver = nullptr;
  }
}