#pragma once

#include "GraphLoader.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QFileDialog>
#include <QListWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QPainter>
#include <QMenuBar>
#include <QDockWidget>
#include <QMouseEvent>
#include <QToolBar>


class GraphWidget : public QWidget {
public:
    GraphWidget(QWidget* parent);
    void showGraph(const Graph* _graph);
    void fit();

protected:
    void updateGraphData();
    void updateViewData();
    QPointF toLocal(QPointF p);
    QPointF toGlobal(QPointF p);
    QPointF toPlotCoords(QPointF p);
    QPointF toWidgetCoords(QPointF p);
    void drawGrid(float start, float end, bool vertical, bool labels, QPainter& painter, QPen& pen, QPen& penLabel);
    void paintEvent(QPaintEvent* event) override;
    void zoomToRect(QPointF _p1, QPointF _p2);
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    const Graph* graph = nullptr;

    bool zooming = false;
    bool panning = false;

    QPoint lastMousePos;
    QPoint currentMousePos = { 0, 0 };

    QPointF offsetCenterLocal = { 0, 0 };
    QPointF scaleLocal = { 1.5, 1.5 };

    QRect plotArea;
    QRect legendVertical;
    QRect legendHorizontal;

    double xFitScale;
    double yFitScale;

    double xMin = 0;
    double xMax = 0;
    double yMin = 0;
    double yMax = 0;

    int leftMargin = 90;
    int bottomMargin = 40;
    int margin = 20;
    int fontSize = 10;
    int lineThickness = 2;
    int gridX = 10;
    int gridY = 20;
};


class MainWindow : public QMainWindow {
public:
    MainWindow();
    void loadFile(const char* filename);

private:
    QListWidget graphList;
    GraphWidget graphDisplay;
    Graphs graphs;

    void createMenu();

private slots:
    void fitView();
    void openFile();
    void showGraph(const Graph* graph);
    void loadGraphs(const QString& filePath);
    void onGraphSelected(const QString& graphName);
};