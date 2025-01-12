#include "simulator.hpp"

#include <iostream>
#include <ctime>

Simulator::Simulator(Netlist *_p_netlist) : p_netlist(_p_netlist) {
}

void printSimulationTime(clock_t _time) {
  if (_time < 1000)
    printf("100%% done\nSimulation finished in %ld ms.\n\n", _time);
  else {
    _time /= 1000;
    if (_time < 60)
      printf("100%% done\nSimulation finished in %ld sec.\n\n", _time);
    else {
      _time /= 60;
      if (_time < 60)
        printf("100%% done\nSimulation finished in %ld min.\n\n", _time);
      else {
        clock_t mins = _time % 60;
        _time /= 60;
        printf("100%% done\nSimulation finished in %ld hr %ld min.\n\n", _time, mins);
      }
    }
  }
}
   
bool Simulator::runTran(AnalysisTran *_p_tran, Solver *p_solver, Dumper *p_dumper) {
  size_t sleSize = 0;
  for (size_t i = 0; i < p_netlist->nets.size(); ++i)
    if (p_netlist->nets[i]->index >= 0)
      ++sleSize;

  double *x = new double[sleSize];
  double *I = new double[sleSize];
  double **Y = new double *[sleSize];
  for (size_t i = 0; i < sleSize; ++i)
    Y[i] = new double[sleSize];

  double  step_delta = (_p_tran->stop - _p_tran->start) / 10.0,
          delta_max = 0.0,
          tran_step = _p_tran->step;
  size_t  step_no = 0,
          iter_no = 0, 
          iter_tot = 0;
  int     step_id = 0;

  for (size_t i = 0; i < p_netlist->nets.size(); ++i) {
      p_dumper->AddViewpoint(p_netlist->nets[i]->name, ViewpointType::Voltage, &p_netlist->nets[i]->fi);
  }
  std::cout << "Simulation started\n" << std::endl;
  p_dumper->BeginDump(p_netlist->fileName);
  p_dumper->WriteHeader();

  std::cout << "Running analysis '" << _p_tran->name << "'" << std::endl;
  clock_t A = clock();
  for (double time_curr = _p_tran->start; time_curr <= _p_tran->stop; time_curr += tran_step) {
    iter_no = 0;

    // Выведем информацию на экран
    if (time_curr > 1 * step_delta && step_id == 0) { std::cout << " 10% done" << std::endl; ++step_id; }
    if (time_curr > 2 * step_delta && step_id == 1) { std::cout << " 20% done" << std::endl; ++step_id; }
    if (time_curr > 3 * step_delta && step_id == 2) { std::cout << " 30% done" << std::endl; ++step_id; }
    if (time_curr > 4 * step_delta && step_id == 3) { std::cout << " 40% done" << std::endl; ++step_id; }
    if (time_curr > 5 * step_delta && step_id == 4) { std::cout << " 50% done" << std::endl; ++step_id; }
    if (time_curr > 6 * step_delta && step_id == 5) { std::cout << " 60% done" << std::endl; ++step_id; }
    if (time_curr > 7 * step_delta && step_id == 6) { std::cout << " 70% done" << std::endl; ++step_id; }
    if (time_curr > 8 * step_delta && step_id == 7) { std::cout << " 80% done" << std::endl; ++step_id; }
    if (time_curr > 9 * step_delta && step_id == 8) { std::cout << " 90% done" << std::endl; ++step_id; }

    do {
      // Инициализация источников и элементов
      for (size_t i = 0; i < p_netlist->vsources.size(); ++i)
        p_netlist->vsources[i]->initTran(time_curr);
      for (size_t i = 0; i < p_netlist->elements.size(); ++i)
        p_netlist->elements[i]->initTran(time_curr, tran_step);

      // Обнуление вектора источников тока и матрицы проводимостей
      for (size_t i = 0; i < sleSize; ++i)
        I[i] = 0.0;
      for (size_t i = 0; i < sleSize; ++i)
        for (size_t j = 0; j < sleSize; ++j)
          Y[i][j] = 0.0;

      // Заполнение вектора источников тока и матрицы проводимостей
      for (size_t i = 0; i < p_netlist->elements.size(); ++i) {
        p_netlist->elements[i]->fillI(I);
        p_netlist->elements[i]->fillY(Y);
      }
      // На данном этапе у нас сформирована ММ с учётом НЯМЭ и Ньютона

      // Решаем ММ
      p_solver->Solve(Y, x, I, sleSize);

      // Вносим поправку в потенциалы схемы
      for (size_t i = 0, j = 0; i < p_netlist->nets.size(); ++i) {
        if (p_netlist->nets[i]->index < 0)
          continue;
        p_netlist->nets[i]->fi += x[j++];
      }

      delta_max = 0.0;
      for (size_t i = 0; i < sleSize; ++i)
        if (fabs(x[i]) > delta_max)
          delta_max = fabs(x[i]);

      iter_no++;
      iter_tot++;
      if (iter_no > 20) {
        printf("Convergence problem at step %d!\nNumber of iterations exceeded %d.\n", step_no, 20);
        break;
      }
    } while (delta_max > 0.001);

    if (iter_no > 20)
      break;

    std::vector<double> currValues;
    for (size_t i = 0; i < p_netlist->nets.size(); ++i) {
        if (p_netlist->nets[i]->index != NODEINDEX_GROUND)
            currValues.push_back(p_netlist->nets[i]->fi);
        //std::cout << p_netlist->nets[i]->fi << "(" << p_netlist->nets[i]->index << ")" << " ";
    }
    //std::cout << "\n";
    
    p_dumper->WriteValuesAtTime(time_curr);
    p_dumper->WriteValues(currValues);
   
      step_no++;
  }
  clock_t B = clock();

  printSimulationTime(B - A);
  std::cout << "Simulation done." << std::endl;
  std::cout << "Newton iterations total : " << iter_tot << std::endl;
  std::cout << "Euler steps total       : " << step_no << std::endl;

  p_dumper->EndDump();

  delete [] x;
  x = nullptr;
  delete [] I;
  I = nullptr;
  for (size_t i = 0; i < sleSize; ++i)
    delete [] Y[i];
  delete [] Y;
  Y = nullptr;

  return true;
}