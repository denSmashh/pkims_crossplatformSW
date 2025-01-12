
#include <QTreeView>
#include <QVBoxLayout>

#include "SpiceTree.hpp"

#include <string>
#include <vector>

SpiceTree::SpiceTree(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    tree = new QTreeWidget(this);
    tree->setColumnCount(1);
    layout->addWidget(tree);
    tree->setHeaderLabel("Spice Tree Viewer");
}

const char* SpiceTree::typeToString(ElementType type) {
    switch (type) {
    case ElementType::Resistor:
        return "Resistor";
    case ElementType::Capacitor:
        return "Capacitor";
    case ElementType::Diode:
        return "Diode";
    case ElementType::VSource:
        return "VSource";
    default:
        return "Unknown";
    }
}

const char* SpiceTree::typeToString(SourceType type) {
    switch (type) {
    case SourceType::DC:
        return "DC";
    case SourceType::Pulse:
        return "Pulse";
    case SourceType::Sine:
        return "Sin";
    default:
        return "Unknown";
    }
}

void SpiceTree::updateTree(const std::vector<Netlist*> &netlists) {
    tree->clear();

    for (const auto* netlist : netlists) {

        auto* netlistItem = new QTreeWidgetItem(tree);
        netlistItem->setText(0, QString::fromStdString(netlist->fileName));

        auto* viewpoints = new QTreeWidgetItem(netlistItem);
        viewpoints->setText(0, "Nets");

        std::map<ElementType, QTreeWidgetItem*> elementTypeGroups;
        std::map<SourceType, QTreeWidgetItem*> sourceTypeGroups;

        for (const auto* element : netlist->elements) {
            if (elementTypeGroups.find(element->type) == elementTypeGroups.end()) {
                auto* typeItem = new QTreeWidgetItem(netlistItem);
                typeItem->setText(0, QString(typeToString(element->type)));
                elementTypeGroups[element->type] = typeItem;
            }

            auto* elementItem = new QTreeWidgetItem(elementTypeGroups[element->type]);
            elementItem->setText(0, QString::fromStdString(element->name));
        }

        for (const auto* element : netlist->vsources) {
            if (sourceTypeGroups.find(element->type) == sourceTypeGroups.end()) {
                auto* typeItem = new QTreeWidgetItem(netlistItem);
                typeItem->setText(0, QString(typeToString(element->type)));
                sourceTypeGroups[element->type] = typeItem;
            }

            auto* elementItem = new QTreeWidgetItem(sourceTypeGroups[element->type]);
            elementItem->setText(0, QString::fromStdString(element->name));
        }

        for (const auto* viewpoint : netlist->nets) {
            auto* typeItem = new QTreeWidgetItem(viewpoints);
            typeItem->setText(0, QString::fromStdString(viewpoint->name));
        }
    }
}