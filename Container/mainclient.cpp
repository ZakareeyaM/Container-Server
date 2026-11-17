#include "mainclient.h"
#include "containerwidget.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QApplication>
#include <QIcon>

MainClient::MainClient(QWidget *parent)
    : QMainWindow(parent),
    containerWidget(new ContainerWidget(this))
{
    setupActions();
    setupMenus();

    connect(actAbout, &QAction::triggered, this, &MainClient::showAbout);
    connect(actHelp, &QAction::triggered, this, &MainClient::showHelp);
    connect(actExit, &QAction::triggered, this, &QWidget::close);

    connect(actBackup, &QAction::triggered, containerWidget, &ContainerWidget::handleBackup);
    connect(actRestore, &QAction::triggered, containerWidget, &ContainerWidget::handleRestore);
    connect(actPostXml, &QAction::triggered, containerWidget, &ContainerWidget::handlePostXML);

    setCentralWidget(containerWidget);
    setWindowTitle("Container");
    setWindowIcon(QIcon(":/icons/container.png"));
    statusBar()->showMessage("Ready - Add containers and assign them to pallets");
}

MainClient::~MainClient() {
    // Qt parent-child handles cleanup
}

void MainClient::showAbout()
{
    QMessageBox aboutBox(this);
    QPixmap icon(":/icons/aboutIcon.png");

    // Scale to something reasonable, e.g. 64x64
    aboutBox.setIconPixmap(icon.scaled(84, 84, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    aboutBox.setWindowTitle("About Container Application");


    QString message;
    message += "╔═══════════════════════════╗\n";
    message += "             📦 COS3711 Container Application\n";
    message += "╠═══════════════════════════╣\n";
    message += " 👨‍💻 Developer : Zakareeya Moodley\n";
    message += " 🆔 Student No : 21097356\n";
    message += " ⚙️ Built in : Qt 6.9.0 + CMake\n";

    aboutBox.setText(message);
    aboutBox.exec();
}

void MainClient::showHelp()
{
    QMessageBox::information(this, "Help \n",
                             "💡How to use this application:\n\n"
                             "You have 2 products to pick from: \n"
                             "(1) Box\n"
                             "(2) Cylinder\n\n"
                             "You will be required to enter their dimensions and weight.\n\n"
                             "When you finish register them, you will then be tasked to move them to a specific pallet.\n"
                             "To do this, click on the box/cylinder from the display and select a pallet number.\n\n"
                             "Backup - upon completion click on the backup button to save your work.\n"
                             "Shortcut = 'Ctrl+B'\n\n"
                             "Restore Pallet - this will restore whatever was previously saved.\n"
                             "Shortcut = 'Ctrl+R'\n\n"
                             "To post xml to network -> click on the 'Post XML' tab , then the button 'Post XML to network'.\n"
                             "Shortcut = 'Ctrl+P'\n\n"
                             "\u2705 Tip: Open the Container Server app before posting the XML network to view the XML live.");
}

void MainClient::setupActions()
{
    actAbout = new QAction(QIcon(":/icons/info.png"), "About", this);
    actAbout->setShortcut(QKeySequence("Ctrl+A"));

    actHelp = new QAction(QIcon(":/icons/help.png"),"Help",this);
    actHelp->setShortcut(QKeySequence("Ctrl+H"));

    actExit = new QAction(QIcon(":/icons/exit.png"),"Exit",this);
    actExit->setShortcut(QKeySequence("Ctrl+E"));

    actBackup = new QAction (QIcon(":/icons/backup.png"),"Backup Pallets",this);
    actBackup->setShortcut(QKeySequence("Ctrl+B"));

    actRestore = new QAction (QIcon(":icons/restore.png"),"Restore Pallets",this);
    actRestore->setShortcut(QKeySequence("Ctrl+R"));

    actPostXml = new QAction (QIcon(":/icons/xml.png"),"Post XML",this);
    actPostXml->setShortcut(QKeySequence("Ctrl+P"));
}

void MainClient::setupMenus()
{
    mnuFile = menuBar()->addMenu("&File");
    mnuFile->addAction(actExit);

    mnuBackup = menuBar()->addMenu("&Backup");
    mnuBackup->addAction(actBackup);
    mnuBackup->addAction(actRestore);

    mnuPost = menuBar()->addMenu("&Network");
    mnuPost->addAction(actPostXml);

    mnuHelp = menuBar()->addMenu("&Help");
    mnuHelp->addAction(actHelp);
    mnuHelp->addSeparator();
    mnuHelp->addAction(actAbout);
}

void MainClient::closeEvent(QCloseEvent *event)
{
    auto response = QMessageBox::question(this, "Exit Application",
                                          "Are you sure you want to exit the Container Manager?",
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No);
    if (response == QMessageBox::Yes) event->accept();
    else event->ignore();
}
