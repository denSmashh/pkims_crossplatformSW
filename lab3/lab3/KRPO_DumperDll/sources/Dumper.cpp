#include "Dumper.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

//const std::string simlog_path = "D:\\PKIMS\\lab_1sem\\crossplatform_sw\\lab3\\lab3\\KRPO_Simulator\\sim_log";
const std::string simlog_path = L".\\simulator\\KRPO_Simulator\\sim_log"

Dumper::Dumper() : fileName("") {
}

void Dumper::AddViewpoint(std::string &_name, ViewpointType _vpt, double *_val) {
  Viewpoint vp;
  vp.name = _name;
  vp.type = _vpt;
  vp.valuePtr = _val;
  viewpoints.push_back(vp);
}

void Dumper_CSV::BeginDump(std::string &_fileName) {
    if (!std::filesystem::exists(simlog_path)) {
        if (!std::filesystem::create_directory(simlog_path)) {
            std::cerr << "Не удалось создать директорию для сохранения результатов симуляции" << std::endl;
        }
    }

    std::filesystem::path filePath(_fileName);
    std::string cut_filename = filePath.stem().string();

    fileName = cut_filename + ".csv";

    sim_log.open(simlog_path + "\\" + fileName);
    if (!sim_log.is_open())
    {
        std::cerr << "Не удалось открыть файл для сохранения результатов симуляции" << std::endl;
    }
}


void Dumper_CSV::WriteHeader() {
    sim_log << "; << CSV SPICE Simulation Log >>" << std::endl;
    sim_log << "; Additional parameters :" << std::endl;
    sim_log << "; Temperature(TEMP) = 2.50000000e+001" << std::endl;
    sim_log << "; Temperature(TNOM) = 2.50000000e+001" << std::endl;
    sim_log << "; Local inaccuracy = 9.99999700e-003" << std::endl;
    sim_log << "; reltol = 1.00000000e-003" << std::endl;
    sim_log << "; Acceleration level = without acceleration\n" << std::endl;
    sim_log << "TIME;";
    for (int i = 0; i < viewpoints.size(); ++i) {
        if (viewpoints[i].type == ViewpointType::Voltage && viewpoints[i].name != "0")
            sim_log << "'v(" << viewpoints[i].name << ")';";
        else if (viewpoints[i].type == ViewpointType::Current)
            sim_log << "'i(" << viewpoints[i].name << ")';";
    }
    sim_log << "\n";
}


void Dumper_CSV::WriteValuesAtTime(double _t) {
    sim_log << _t << ";";
}

void Dumper_CSV::WriteValues(std::vector<double>& currValuesPtr) {
    for (size_t i = 0; i < currValuesPtr.size(); ++i)
    {
        sim_log << currValuesPtr[i] << ";";
    }
    sim_log << "\n";
}


void Dumper_CSV::EndDump() {
    if (sim_log.is_open()) {
        sim_log.close();
        std::cerr << "\nРезультаты симуляции: " << simlog_path + "/" + fileName << std::endl;
    }
    else
    {
        std::cerr << "\nТ.к. не удалось создать директорию или открыть файл - результаты симуляции не сохранились :(" << std::endl;
    }
}

PluginType GetType() {
  return PluginType::dumper;
}

void GetStringID(std::string &_id) {
  _id = "0xDumper_CSV_ID";
}

Dumper *GetDumper() {
  return new Dumper_CSV;
}

void FreeDumper(Dumper *_p_dumper) {
  if (_p_dumper) {
    delete _p_dumper;
    _p_dumper = nullptr;
  }
}