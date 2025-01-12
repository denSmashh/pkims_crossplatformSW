#pragma once

#include <string>
#include <vector>
#include <Windows.h>

#include "Solver.hpp"
#include "Dumper.hpp"

enum class PluginType {
  solver = 1,
  dumper,
};

typedef PluginType (* type_func_ptr)();
typedef void (* idstr_func_ptr)(std::string &id);

typedef Solver *(* get_solver_func_ptr)();
typedef Dumper *(* get_dumper_func_ptr)();
typedef void (* free_solver_func_ptr)(Solver *);
typedef void (* free_dumper_func_ptr)(Dumper *);

struct PluginInfo {
  std::string     fileName;
  std::string     idString;
  PluginType      type;
};

typedef std::vector<PluginInfo> Plugins;

class PluginManager {
  Plugins               plugins;
  bool                  ok;
  HMODULE               hSolverDll,
                        hDumperDll;
  get_solver_func_ptr   p_getSolverFunc;
  get_dumper_func_ptr   p_getDumperFunc;
  free_solver_func_ptr  p_freeSolverFunc;
  free_dumper_func_ptr  p_freeDumperFunc;
public:
  PluginManager();
 ~PluginManager();
public:
  bool IsOk();
  void List();
  bool SelectSolver(std::string &_solverName);
  bool SelectDumper(std::string &_dumperName);
  std::string GetDefaultSolverName();
  std::string GetDefaultDumperName();
  Solver *GetSolver();
  Dumper *GetDumper();
  void FreeSolver(Solver *);
  void FreeDumper(Dumper *);
private:
  bool ListPluginsOnStartup();
};

