// containerwidget.cpp
#include "containerwidget.h"
#include "containerfactory.h"
#include "serializing.h"

#include <QMessageBox>
#include <QSpinBox>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QTcpSocket>
#include <QThread>
#include <QMetaProperty>
#include <QXmlStreamWriter>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStringList>

ContainerWidget::ContainerWidget(QWidget *parent)
    : QWidget(parent), memento(new ContainerMemento)
{
    setupUI();

    connect(buttonAddBox, &QPushButton::clicked, this, &ContainerWidget::addBox);
    connect(buttonAddCylinder, &QPushButton::clicked, this, &ContainerWidget::addCylinder);
    connect(buttonMoveToPallet, &QPushButton::clicked, this, &ContainerWidget::moveToPallet);
    connect(displayUnallocated, &QListWidget::itemSelectionChanged, this, &ContainerWidget::onSelectionChanged);

    connect(buttonBackupRestore, &QPushButton::clicked, this, &ContainerWidget::handleBackup);
    connect(buttonPostXML, &QPushButton::clicked, this, &ContainerWidget::handlePostXML);

    enableDisableButtons();
}

ContainerWidget::~ContainerWidget() {
    qDeleteAll(unallocatedContainers);
    unallocatedContainers.clear();

    qDeleteAll(pallets);
    pallets.clear();

    delete memento;
}

void ContainerWidget::setupUI() {
    tabWidget = new QTabWidget(this);
    QWidget *containerTab = new QWidget;

    // Box group
    QGroupBox *boxGroup = new QGroupBox("Box");
    QGridLayout *boxLayout = new QGridLayout(boxGroup);

    box_breadth = new QSpinBox;
    box_breadth->setRange(1, 1000);
    box_breadth->setValue(1);

    box_length = new QSpinBox;
    box_length->setRange(1, 1000);
    box_length->setValue(1);

    box_height = new QSpinBox;
    box_height->setRange(1, 1000);
    box_height->setValue(1);

    box_weight = new QSpinBox;
    box_weight->setRange(1, 10000);
    box_weight->setValue(1);

    buttonAddBox = new QPushButton("Add box");

    boxLayout->addWidget(new QLabel("Breadth:"), 0, 0);
    boxLayout->addWidget(box_breadth, 0, 1);
    boxLayout->addWidget(new QLabel("Length:"), 1, 0);
    boxLayout->addWidget(box_length, 1, 1);
    boxLayout->addWidget(new QLabel("Height:"), 2, 0);
    boxLayout->addWidget(box_height, 2, 1);
    boxLayout->addWidget(new QLabel("Weight:"), 3, 0);
    boxLayout->addWidget(box_weight, 3, 1);
    boxLayout->addWidget(buttonAddBox, 4, 0, 1, 2);

    // Cylinder group
    QGroupBox *cylinderGroup = new QGroupBox("Cylinder");
    QGridLayout *cylinderLayout = new QGridLayout(cylinderGroup);

    cylinder_diameter = new QSpinBox;
    cylinder_diameter->setRange(1, 1000);
    cylinder_diameter->setValue(1);

    cylinder_height = new QSpinBox;
    cylinder_height->setRange(1, 1000);
    cylinder_height->setValue(1);

    cylinder_weight = new QSpinBox;
    cylinder_weight->setRange(1, 10000);
    cylinder_weight->setValue(1);

    buttonAddCylinder = new QPushButton("Add cylinder");

    cylinderLayout->addWidget(new QLabel("Diameter:"), 0, 0);
    cylinderLayout->addWidget(cylinder_diameter, 0, 1);
    cylinderLayout->addWidget(new QLabel("Height:"), 1, 0);
    cylinderLayout->addWidget(cylinder_height, 1, 1);
    cylinderLayout->addWidget(new QLabel("Weight:"), 2, 0);
    cylinderLayout->addWidget(cylinder_weight, 2, 1);
    cylinderLayout->addWidget(buttonAddCylinder, 4, 0, 1, 2);

    // Bottom layout - unallocated list and pallet controls
    displayUnallocated = new QListWidget;
    palletNumber = new QSpinBox;
    palletNumber->setRange(1, 9999);
    palletNumber->setValue(1);

    buttonMoveToPallet = new QPushButton("Move to pallet");
    buttonMoveToPallet->setEnabled(false);

    buttonBackupRestore = new QPushButton("Backup");

    QGridLayout *bottomLayout = new QGridLayout;
    bottomLayout->addWidget(new QLabel("List of unallocated containers:"), 0, 0);
    bottomLayout->addWidget(displayUnallocated, 1, 0, 5, 1);
    bottomLayout->addWidget(new QLabel("Click on item in list, choose a pallet number, and move the container to that pallet"), 1, 1);
    bottomLayout->addWidget(new QLabel("Pallet number:"), 2, 1);
    bottomLayout->addWidget(palletNumber, 3, 1);
    bottomLayout->addWidget(buttonMoveToPallet, 4, 1);
    bottomLayout->addWidget(buttonBackupRestore, 5, 1);

    QGridLayout *containerLayout = new QGridLayout(containerTab);
    containerLayout->addWidget(boxGroup, 0, 0);
    containerLayout->addWidget(cylinderGroup, 0, 1);
    containerLayout->addLayout(bottomLayout, 1, 0, 1, 2);

    // Post XML tab
    QWidget *postTab = new QWidget;
    QVBoxLayout *postLayout = new QVBoxLayout(postTab);

    buttonPostXML = new QPushButton("Post XML to network");
    displayPalletXML = new QTextEdit;
    displayPalletXML->setReadOnly(true);

    postLayout->addWidget(buttonPostXML);
    postLayout->addWidget(new QLabel("XML Output:"));
    postLayout->addWidget(displayPalletXML);

    tabWidget->addTab(containerTab, "Containers");
    tabWidget->addTab(postTab, "Post XML");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);
}

void ContainerWidget::addBox() {
    QVariantList params;
    params << box_length->value() << box_breadth->value() << box_height->value() << box_weight->value();

    Container* box = ContainerFactory::createContainer(ContainerFactory::BOX, params);
    if (box) {
        unallocatedContainers.append(box);
        updateUnallocatedList();
        clearInputs();
        enableDisableButtons();

        QMessageBox::information(this, "Success",
                                 QString("Box %1 added successfully!").arg(box->getCode()));
    }
}

void ContainerWidget::addCylinder() {
    QVariantList params;
    params << cylinder_diameter->value() << cylinder_height->value() << cylinder_weight->value();

    Container* cylinder = ContainerFactory::createContainer(ContainerFactory::CYLINDER, params);
    if (cylinder) {
        unallocatedContainers.append(cylinder);
        updateUnallocatedList();
        clearInputs();
        enableDisableButtons();

        QMessageBox::information(this, "Success",
                                 QString("Cylinder %1 added successfully!").arg(cylinder->getCode()));
    }
}

void ContainerWidget::moveToPallet() {
    int currentRow = displayUnallocated->currentRow();
    if (currentRow < 0 || currentRow >= unallocatedContainers.size()) {
        QMessageBox::warning(this, "Warning", "Please select a container to move.");
        return;
    }

    Container* container = unallocatedContainers.takeAt(currentRow);
    int palletNum = palletNumber->value();

    if (!pallets.contains(palletNum)) {
        pallets[palletNum] = new Pallet(palletNum);
    }

    pallets[palletNum]->addContainer(container);
    updateUnallocatedList();
    enableDisableButtons();

    QMessageBox::information(this, "Success",
                             QString("Container %1 moved to pallet %2").arg(container->getCode()).arg(palletNum));
}

void ContainerWidget::onSelectionChanged() {
    enableDisableButtons();
}

void ContainerWidget::enableDisableButtons() {
    buttonMoveToPallet->setEnabled(displayUnallocated->currentRow() >= 0);
}

void ContainerWidget::handleBackup() {
    if (unallocatedContainers.isEmpty()) {
        QMessageBox::information(this, "Backup", "No containers to backup.");
        return;
    }

    QFile file("backup.txt");

    // Check if backup file already exists
    if (file.exists()) {
        // File exists, ask user what they want to do
        auto response = QMessageBox::question(this, "Backup File Exists",
                                              "A backup file already exists. Do you want to:\n"
                                              "- Yes: Overwrite the existing backup\n"
                                              "- No: Keep the existing backup (no action taken)",
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::No);

        if (response == QMessageBox::No) {
            QMessageBox::information(this, "Backup", "Backup operation cancelled. Existing backup file preserved.");
            return;
        }
        // If Yes, continue with overwriting
    }

    // Either file doesn't exist or user chose to overwrite
    if (saveContainersToFile("backup.txt")) {
        QString message;
        if (file.exists()) {
            message = QString("Successfully updated backup with %1 containers in backup.txt")
            .arg(unallocatedContainers.size());
        } else {
            message = QString("Successfully created new backup with %1 containers in backup.txt")
            .arg(unallocatedContainers.size());
        }
        QMessageBox::information(this, "Backup", message);
    } else {
        QMessageBox::warning(this, "Backup Error",
                             "Failed to save containers to backup.txt");
    }
}

void ContainerWidget::handleRestore() {
    QFile file("backup.txt");
    if (!file.exists()) {
        QMessageBox::information(this, "Restore", "No backup file found (backup.txt).");
        return;
    }

    auto response = QMessageBox::question(this, "Restore Containers",
                                          "This will replace all current unallocated containers with the backed up ones. Continue?",
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No);

    if (response == QMessageBox::Yes) {
        if (loadContainersFromFile("backup.txt")) {
            updateUnallocatedList();
            enableDisableButtons();
            QMessageBox::information(this, "Restore",
                                     QString("Successfully restored %1 containers from backup.txt")
                                         .arg(unallocatedContainers.size()));
        } else {
            QMessageBox::warning(this, "Restore Error",
                                 "Failed to load containers from backup.txt");
        }
    }
}

void ContainerWidget::handlePostXML() {
    if (pallets.isEmpty()) {
        QMessageBox::information(this, "No Data", "No pallets with containers to send. Please move some containers to pallets first.");
        return;
    }

    // Create worker + thread
    QThread* thread = new QThread;
    Serializing* worker = new Serializing(pallets);
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &Serializing::run);
    connect(worker, &Serializing::finished, this, [this, thread, worker](const QString& xml) {
        // Display generated XML
        displayPalletXML->setPlainText(xml);

        // Send to server
        sendXMLToServer(xml);

        // cleanup
        thread->quit();
    });

    connect(worker, &Serializing::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
}

void ContainerWidget::sendXMLToServer(const QString& xml) {
    QTcpSocket* socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, [this, socket, xml]() {
        QByteArray data = xml.toUtf8();
        qint64 written = socket->write(data);
        socket->flush();
        socket->disconnectFromHost();
    });

    connect(socket,
            QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this,   // context object
            [this, socket](QAbstractSocket::SocketError) {
                QString warningMessage = QString("Failed to connect/send: %1\n\n"
                                                 "\u2705 Remember to have Container Server \n      listening when posting the network")
                                             .arg(socket->errorString());

                QMessageBox::warning(this, "Network Error", warningMessage);

                socket->deleteLater();
            });

    connect(socket, &QTcpSocket::disconnected,
            this,   // context object
            [this, socket]() {
                QMessageBox::information(this, "Success", "XML sent to server successfully!");
                socket->deleteLater();
            });


    socket->connectToHost("127.0.0.1", 6164);
}

bool ContainerWidget::saveContainersToFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << "# Container Backup File\n";
    out << "# Format: TYPE|CODE|WEIGHT|PARAM1|PARAM2|PARAM3\n";
    out << "# Box: TYPE|CODE|WEIGHT|LENGTH|BREADTH|HEIGHT\n";
    out << "# Cylinder: TYPE|CODE|WEIGHT|DIAMETER|HEIGHT\n";
    out << "CONTAINER_COUNT=" << unallocatedContainers.size() << "\n";
    out << "\n";

    for (const Container* container : std::as_const(unallocatedContainers))
    {
        if (container->getType() == "Box") {
            const Box* box = static_cast<const Box*>(container);
            out << "BOX|"
                << box->getCode() << "|"
                << box->getWeight() << "|"
                << box->getLength() << "|"
                << box->getBreadth() << "|"
                << box->getHeight() << "\n";
        } else if (container->getType() == "Cylinder") {
            const Cylinder* cylinder = static_cast<const Cylinder*>(container);
            out << "CYLINDER|"
                << cylinder->getCode() << "|"
                << cylinder->getWeight() << "|"
                << cylinder->getDiameter() << "|"
                << cylinder->getHeight() << "\n";
        }
    }

    file.close();
    return true;
}

bool ContainerWidget::loadContainersFromFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    // Clear current unallocated containers
    qDeleteAll(unallocatedContainers);
    unallocatedContainers.clear();

    QTextStream in(&file);
    int expectedCount = 0;
    int loadedCount = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Skip comments and empty lines
        if (line.startsWith("#") || line.isEmpty()) {
            continue;
        }

        // Parse container count
        if (line.startsWith("CONTAINER_COUNT=")) {
            expectedCount = line.split("=")[1].toInt();
            continue;
        }

        // Parse container data
        QStringList parts = line.split("|");
        if (parts.size() < 4) {
            continue; // Invalid format
        }

        QString type = parts[0];
        QString code = parts[1];
        int weight = parts[2].toInt();

        Container* container = nullptr;

        if (type == "BOX" && parts.size() >= 6) {
            int length = parts[3].toInt();
            int breadth = parts[4].toInt();
            int height = parts[5].toInt();

            container = new Box(length, breadth, height, weight);
            container->setCode(code);
            loadedCount++;
        } else if (type == "CYLINDER" && parts.size() >= 5) {
            int diameter = parts[3].toInt();
            int height = parts[4].toInt();

            container = new Cylinder(diameter, height, weight);
            container->setCode(code);
            loadedCount++;
        }

        if (container) {
            unallocatedContainers.append(container);
        }
    }

    file.close();

    // Verify we loaded the expected number of containers
    if (expectedCount > 0 && loadedCount != expectedCount) {
        QMessageBox::warning(this, "Restore Warning",
                             QString("Expected %1 containers but loaded %2. Some data may be corrupted.")
                                 .arg(expectedCount).arg(loadedCount));
    }

    return true;
}

void ContainerWidget::updateUnallocatedList() {
    displayUnallocated->clear();
    for (const Container* container : std::as_const(unallocatedContainers)) {
        QString itemText = QString("%1 - %2 (Weight: %3, Volume: %4)")
        .arg(container->getCode(),
             container->getType(),
             QString::number(container->getWeight()),
             QString::number(container->getVolume()));
        displayUnallocated->addItem(itemText);
    }
}

void ContainerWidget::clearInputs() {
    box_breadth->setValue(1);
    box_length->setValue(1);
    box_height->setValue(1);
    box_weight->setValue(1);
    cylinder_diameter->setValue(1);
    cylinder_height->setValue(1);
    cylinder_weight->setValue(1);

    box_length->setFocus();
}

// Memento implementation
void ContainerMemento::saveState(const QList<Container*>& containers) {
    qDeleteAll(savedContainers);
    savedContainers.clear();

    for (const Container* c : containers) {
        if (c) savedContainers.append(c->clone());
    }
}

QList<Container*> ContainerMemento::restoreState() {
    QList<Container*> restored;
    for (const Container* c : std::as_const(savedContainers)) {
        restored.append(c->clone());
    }
    return restored;
}
