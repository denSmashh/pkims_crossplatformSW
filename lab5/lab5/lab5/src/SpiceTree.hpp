#pragma once

#include <QString>
#include <QWidget>
#include <QTreeWidget>

#include "../simulator/KRPO_Simulator/sources/Simulator.hpp"

class SpiceTree : public QWidget {
private:
    QTreeWidget* tree;

    const char* typeToString(ElementType);
    const char* typeToString(SourceType);

public:
    SpiceTree(QWidget* parent);

    void updateTree(const std::vector<Netlist*>& netlists);
};