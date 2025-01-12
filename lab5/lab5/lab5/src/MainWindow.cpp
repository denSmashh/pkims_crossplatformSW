#include <QMenu>
#include <QString>
#include <QMenuBar>
#include <QTextEdit>
#include <QTextStream>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>

#include "MainWindow.hpp"

const QString netlistFilePath = "D:/PKIMS/lab_1sem/crossplatform_sw/lab5/lab5/lab5/simulator/KRPO_Simulator/bin/netlists";
const QString simRunPath      = "D:/PKIMS/lab_1sem/crossplatform_sw/lab5/lab5/lab5/simulator/KRPO_Simulator/build/msvs2017ce/x64/Debug/KRPO_Simulator.exe";
const QString simGraphPath    = "D:/PKIMS/lab_1sem/crossplatform_sw/lab5/lab5/SimGraph/x64/Debug/SimGraph.exe";

MainWindow::MainWindow() : QMainWindow(nullptr) {
    resize(800, 600);
    setWindowTitle("PKIMS SPICE Simulator *");

    registerActions();
    configMenu();
    initStatusBar();
    initCodeEditor();
    initSimProcess();
    initDockWidgets();
}

void MainWindow::registerActions() {
    actionFileNew = new QAction("&New", this);
    actionFileNew->setShortcut(QKeySequence("Ctrl+N"));
    connect(actionFileNew, &QAction::triggered, this, &MainWindow::callbackFileNew);

    actionFileOpen = new QAction("&Open", this);
    actionFileOpen->setShortcut(QKeySequence("Ctrl+O"));
    connect(actionFileOpen, &QAction::triggered, this, &MainWindow::callbackFileOpen);

    actionFileSave = new QAction("&Save", this);
    actionFileSave->setShortcut(QKeySequence("Ctrl+S"));
    connect(actionFileSave, &QAction::triggered, this, &MainWindow::callbackFileSave);
    
    actionFileSaveAs = new QAction("&Save As", this);
    connect(actionFileSaveAs, &QAction::triggered, this, &MainWindow::callbackFileSaveAs);

    actionFileExit = new QAction("&Exit", this);
    actionFileExit->setShortcut(QKeySequence("Ctrl+E"));
    connect(actionFileExit, &QAction::triggered, this, &MainWindow::callbackFileExit);

    actionRunNetlist = new QAction("&Netlist", this);
    actionRunNetlist->setShortcut(QKeySequence("Ctrl+R"));
    connect(actionRunNetlist, &QAction::triggered, this, &MainWindow::callbackRunNetlist);

    actionRunPostprocessor = new QAction("&Postprocessor", this);
    actionRunPostprocessor->setShortcut(QKeySequence("Ctrl+P"));
    connect(actionRunPostprocessor, &QAction::triggered, this, &MainWindow::callbackRunPostprocessor);

    actionHelpAbout = new QAction("&About", this);
    actionHelpAbout->setShortcut(QKeySequence("Ctrl+H"));
    connect(actionHelpAbout, &QAction::triggered, this, &MainWindow::callbackHelpAbout);
}

void MainWindow::configMenu() {
    QMenu* menuFile = new QMenu("&File");
    QMenu* menuRun = new QMenu("&Run");
    QMenu* menuHelp = new QMenu("&Help");

    menuFile->addAction(actionFileNew);
    menuFile->addAction(actionFileOpen);
    menuFile->addAction(actionFileSave);
    menuFile->addAction(actionFileSaveAs);
    menuFile->addAction(actionFileExit);
    menuRun->addAction(actionRunNetlist);
    menuRun->addAction(actionRunPostprocessor);
    menuHelp->addAction(actionHelpAbout);

    menuBar()->addMenu(menuFile);
    menuBar()->addMenu(menuRun);
    menuBar()->addMenu(menuHelp);
}

void MainWindow::initStatusBar() {
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    cursorLabel = new QLabel("0:0");
    statusBar->addPermanentWidget(cursorLabel);
}

void MainWindow::initSimProcess() {
    simProcess = new QProcess(this);
    graphProcess = new QProcess(this);
    connect(simProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::callbackReadSimStandardOutput);
    connect(simProcess, &QProcess::readyReadStandardError, this, &MainWindow::callbackReadSimStandardError);
    connect(simProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::simProcessFinished);

}

void MainWindow::initCodeEditor() {
    spiceEditorWidget = new SpiceEditor(this);
    setCentralWidget(spiceEditorWidget);
    connect(spiceEditorWidget, &SpiceEditor::cursorPositionChanged, this, &MainWindow::callbackOnCursorPositionChanged);
    connect(spiceEditorWidget, &SpiceEditor::textChanged, this, &MainWindow::callbackTextChanged);
}

void MainWindow::initDockWidgets() {
    //loggerWidget = new Console(this);
    loggerWidget = new QTextEdit(this);
    loggerWidget->setReadOnly(true);
    QDockWidget* consoleDockWidget = new QDockWidget("Console", this);
    consoleDockWidget->setWidget(loggerWidget);
    addDockWidget(Qt::BottomDockWidgetArea, consoleDockWidget);
    consoleDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);

    spiceTreeWidget = new SpiceTree(this);
    QDockWidget* spiceTreeDockWidget = new QDockWidget("Spice Tree", this);
    spiceTreeDockWidget->setWidget(spiceTreeWidget);
    addDockWidget(Qt::LeftDockWidgetArea, spiceTreeDockWidget);
    spiceTreeDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
}

void MainWindow::updateSpiceTree(std::string spiceNetlist) {
    delete p_netlist;
    p_netlist = new Netlist;

    QFileInfo openFileInfo(currentFileName);
    p_netlist->fileName = openFileInfo.fileName().toStdString();

    Netlistreader netlistReader;
    netlistReader.readNetlist(spiceNetlist, p_netlist, false);

    spiceTreeWidget->updateTree({p_netlist});
}

void MainWindow::callbackOnCursorPositionChanged() {
    cursorLabel->setText(
        QString("%1:%2").arg(spiceEditorWidget->textCursor().blockNumber())
        .arg(spiceEditorWidget->textCursor().positionInBlock()));
}

void MainWindow::callbackTextChanged() {
    updateSpiceTree(spiceEditorWidget->toPlainText().toStdString());
    setWindowTitle("PKIMS SPICE Simulator *");
}

void MainWindow::callbackFileNew() {
    QString content = spiceEditorWidget->toPlainText();
    if (!content.isEmpty() && QMessageBox::question(this, "New File", "Do you want to save the current file ?") == QMessageBox::Yes) {
        callbackFileSave();
    }
    spiceEditorWidget->clear();
    currentFileName.clear();
    setWindowTitle("PKIMS SPICE Simulator *");
    loggerWidget->insertHtml(QString("Creating blank file!<br>"));
}

void MainWindow::callbackFileOpen() {
    QString openFileName = QFileDialog::getOpenFileName(
        this,
        "Open code",
        netlistFilePath,
        "SPICE Files (*.sp);;All files (*.*)");
    
    if (openFileName.isEmpty())
        return;

    QFile openFile(openFileName);

    if (!openFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        loggerWidget->insertHtml(QString("Error: Don't open file: %1<br>").arg(openFileName));
        return;
    }

    loggerWidget->insertHtml(QString("Open file: %1<br>").arg(openFileName));

    currentFileName = openFile.fileName();

    spiceEditorWidget->setPlainText(openFile.readAll());
    openFile.close();

    setWindowTitle("PKIMS SPICE Simulator");
}

void MainWindow::callbackFileSave() {
    if (currentFileName.isEmpty()) {
        callbackFileSaveAs();
    }
    else {
        QFile file (currentFileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << spiceEditorWidget->toPlainText();
            file.close();
            setWindowTitle("PKIMS SPICE Simulator");
            loggerWidget->insertHtml(QString("Save file: %1<br>").arg(currentFileName));
        }
        else {
            loggerWidget->insertHtml(QString("Could not save file: %1<br>").arg(currentFileName));
        }
    }
}

void MainWindow::callbackFileSaveAs() {
    QString saveAsFilepath = QFileDialog::getSaveFileName(
        this, 
        "Save File As", 
        netlistFilePath,
        "SPICE Files (*.sp);;All files (*.*)");
    if (!saveAsFilepath.isEmpty()) {
        currentFileName = saveAsFilepath;
        callbackFileSave();
        setWindowTitle("PKIMS SPICE Simulator");
    }
}

void MainWindow::callbackFileExit() {
    close();
}

void MainWindow::callbackReadSimStandardOutput() {
    QByteArray output = simProcess->readAllStandardOutput();
    QString text = QString::fromLocal8Bit(output);
    loggerWidget->append(text);
}

void MainWindow::callbackReadSimStandardError() {
    loggerWidget->append(simProcess->readAllStandardError());
}

void MainWindow::simProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    //qDebug() << "exitCode = " << exitCode << "  exitStatus = " << exitStatus;
    if (exitCode == EXIT_SUCCESS) {
        loggerWidget ->append("Simulation process is done!<br>");
        callbackRunPostprocessor(true);
    }
    else {
        loggerWidget->append("Error: Simulation process have error! Fix them and run again simulator!<br>");
        //loggerWidget->append("<b>Simulation process have error! Fix them and run again simulator!</b><br>");
        //loggerWidget->append("<span style = 'color: red; font-weight: bold;'>Simulation process have error! Fix them and run again simulator!< / span>");
    }
}

void MainWindow::callbackRunNetlist() {
    if (simProcess->state() != QProcess::NotRunning) {
        simProcess->terminate();
        simProcess->waitForFinished();
    }
  
    QStringList simArgs; 
    simArgs << "--input" << currentFileName;
    simProcess->start(simRunPath, simArgs);
}

void MainWindow::callbackRunPostprocessor(bool afterSim) {
    simProcess->waitForFinished();
    if (graphProcess->state() != QProcess::NotRunning) {
        graphProcess->terminate();
        graphProcess->waitForFinished();
    }

    if (afterSim) {
        QStringList simGraphArgs;
        QFileInfo fileInfo(currentFileName);
        QString logPath = "./simulator/KRPO_Simulator/sim_log/" + fileInfo.fileName().chopped(3) + ".csv";
        //QString logPath = "./simulator/KRPO_Simulator/sim_log/rc.csv";

        if (!QFileInfo::exists(logPath)) {
            loggerWidget->append("<br>Error: log file " + logPath + " does not exist. Fixed error!<br>");
            return;
        }

        simGraphArgs << logPath;
        graphProcess->start(simGraphPath, simGraphArgs);
    }
    else {
        graphProcess->start(simGraphPath);
    }

    loggerWidget->append("<br>Start PostProcessor Tool - SimGraph!<br>");
}

void MainWindow::callbackHelpAbout() {
    QMessageBox::information(
        nullptr, 
        "About", 
        "Developed by D. Smashnoy\n \
        Inspired and support by the PKIMS Department :)"
    );
}


