#pragma once

#include <QLabel>
#include <QDebug>
#include <QAction>
#include <QProcess>
#include <QShortcut>
#include <QStatusBar>
#include <QMainWindow>

#include "Console.hpp"
#include "SpiceTree.hpp"
#include "SpiceEditor.hpp"

#include "../simulator/KRPO_Simulator/sources/NetlistReader.hpp"
#include "../simulator/KRPO_Simulator/sources/Simulator.hpp"


class MainWindow : public QMainWindow {

public:
    MainWindow();

private:
    // custom widgets
    //Console* loggerWidget;
    QTextEdit* loggerWidget;
    SpiceTree* spiceTreeWidget;
    SpiceEditor* spiceEditorWidget;

    // main window status bar
    QLabel* cursorLabel;
    QStatusBar* statusBar;

    // process
    QProcess* simProcess;
    QProcess* graphProcess;
    
    // actions
    QAction* actionFileNew;
    QAction* actionFileOpen;
    QAction* actionFileSave;
    QAction* actionFileSaveAs;
    QAction* actionFileExit;
    QAction* actionRunNetlist;
    QAction* actionRunPostprocessor;
    QAction* actionHelpAbout;

    // file processing
    QString currentFileName;
    Netlist* p_netlist = new Netlist;

    // initialization functions
     void configMenu();
     void initStatusBar();
     void initSimProcess();
     void initCodeEditor();
     void initDockWidgets();
     void registerActions();
     void updateSpiceTree(std::string spiceNetlist);

private slots:
    void callbackFileNew();
    void callbackFileOpen();
    void callbackFileSave();
    void callbackFileSaveAs();
    void callbackFileExit();
    void callbackRunNetlist();
    void callbackRunPostprocessor(bool afterSim);
    void callbackHelpAbout();
    void callbackOnCursorPositionChanged();
    void callbackTextChanged();
    void callbackReadSimStandardOutput();
    void callbackReadSimStandardError();
    void simProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
};
