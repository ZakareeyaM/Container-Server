#include "containerserver.h"
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QDebug>
#include <QTimer>

ContainerServer::ContainerServer(QWidget *parent)
    : QMainWindow(parent),
    tcpServer(new QTcpServer(this)),
    clientSocket(nullptr),
    model(new QStandardItemModel(this)),
    isListening(false)
{
    // Create central widget and layout
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);

    // Create button layout
    auto *buttonLayout = new QHBoxLayout();

    setWindowIcon(QIcon(":/icon/server_icon.ico"));
    // Create Start/Stop Listening button
    startListeningButton = new QPushButton("Start Listening", this);
    startListeningButton->setFixedSize(150, 30);
    connect(startListeningButton, &QPushButton::clicked, this, &ContainerServer::toggleListening);

    // Create status label
    statusLabel = new QLabel("Ready - Not listening", this);
    statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; margin-left: 10px; }");

    buttonLayout->addWidget(startListeningButton);
    buttonLayout->addWidget(statusLabel);
    buttonLayout->addStretch(); // Push elements to the left

    // Set up the table view
    auto *tableView = new QTableView(this);
    tableView->setModel(model);
    model->setHorizontalHeaderLabels({"Pallet", "Container", "Code", "Height", "Breadth/Diameter", "Length", "Weight"});

    // Add widgets to main layout
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(tableView);

    setWindowTitle("Container Server");
    resize(800, 450);

    qDebug() << "Container Server initialized - Click 'Start Listening' to begin";
}

ContainerServer::~ContainerServer()
{
    // Clean up client socket
    if (clientSocket) {
        clientSocket->blockSignals(true); // Prevent further signals
        clientSocket->disconnectFromHost();
        if (clientSocket->state() != QAbstractSocket::UnconnectedState) {
            clientSocket->waitForDisconnected(1000);
        }
        clientSocket->deleteLater();
        clientSocket = nullptr;
    }

    // Stop server
    if (tcpServer && tcpServer->isListening()) {
        tcpServer->close();
    }
}

void ContainerServer::toggleListening()
{
    if (!isListening) {
        // Start listening
        if (!tcpServer->listen(QHostAddress::Any, 6164)) {
            QMessageBox::critical(this, "Error", "Unable to start server: " + tcpServer->errorString());
            return;
        }

        connect(tcpServer, &QTcpServer::newConnection, this, &ContainerServer::newConnection, Qt::UniqueConnection);

        isListening = true;
        startListeningButton->setText("Stop Listening");
        statusLabel->setText("Listening on port 6164");
        statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; margin-left: 10px; }");

        // Clear any existing data when starting to listen
        model->removeRows(0, model->rowCount());

        qDebug() << "Server started listening on port 6164";
        QMessageBox::information(this, "Server Status", "Server is now listening on port 6164.\nReady to receive container data.");
    } else {
        // Stop listening
        if (clientSocket) {
            clientSocket->blockSignals(true);
            clientSocket->disconnectFromHost();
            if (clientSocket->state() != QAbstractSocket::UnconnectedState) {
                clientSocket->waitForDisconnected(1000);
            }
            clientSocket->deleteLater();
            clientSocket = nullptr;
        }

        tcpServer->disconnect();
        tcpServer->close();

        isListening = false;
        startListeningButton->setText("Start Listening");
        statusLabel->setText("Ready - Not listening");
        statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; margin-left: 10px; }");

        qDebug() << "Server stopped listening";
        QMessageBox::information(this, "Server Status", "Server has stopped listening.");
    }
}

void ContainerServer::newConnection()
{
    if (!isListening) {
        qDebug() << "Connection received but server is not supposed to be listening";
        return;
    }

    // Clean up any existing connection safely
    if (clientSocket) {
        clientSocket->blockSignals(true);
        clientSocket->disconnectFromHost();
        clientSocket->deleteLater();
        clientSocket = nullptr;
    }

    clientSocket = tcpServer->nextPendingConnection();

    if (!clientSocket) {
        qDebug() << "Failed to get pending connection";
        return;
    }

    // Use Qt::UniqueConnection to prevent multiple connections
    connect(clientSocket, &QTcpSocket::readyRead, this, &ContainerServer::readClientData, Qt::UniqueConnection);
    connect(clientSocket, &QTcpSocket::disconnected, this, &ContainerServer::clientDisconnected, Qt::UniqueConnection);
    connect(clientSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &ContainerServer::socketError, Qt::UniqueConnection);

    qDebug() << "New client connected from:" << clientSocket->peerAddress().toString();
}

void ContainerServer::clientDisconnected()
{
    qDebug() << "Client disconnected";
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        socket->deleteLater();
        if (socket == clientSocket) {
            clientSocket = nullptr;
        }
    }
}

void ContainerServer::socketError(QAbstractSocket::SocketError error)
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        qDebug() << "Socket error occurred:" << error << socket->errorString();
        socket->deleteLater();
        if (socket == clientSocket) {
            clientSocket = nullptr;
        }
    }
}

void ContainerServer::readClientData()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        qDebug() << "readClientData: sender is not a QTcpSocket";
        return;
    }

    // Check if socket is still connected
    if (socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Socket is not in connected state:" << socket->state();
        return;
    }

    // Read all available data
    QByteArray data;
    while (socket->bytesAvailable() > 0) {
        data.append(socket->readAll());
        // Small delay to ensure we get all data
        socket->waitForReadyRead(100);
    }

    if (data.isEmpty()) {
        qDebug() << "No data received";
        return;
    }

    QString xmlData = QString::fromUtf8(data).trimmed();
    qDebug() << "Received XML data length:" << data.size();
    qDebug() << "XML Content:" << xmlData;

    // Parse XML in a try-catch equivalent (using validation)
    if (xmlData.startsWith("<?xml") || xmlData.startsWith("<pallets")) {
        parseXML(xmlData);
    } else {
        qDebug() << "Received data doesn't appear to be valid XML";
        QMessageBox::warning(this, "Invalid Data", "Received data is not valid XML format");
    }

    // Close the connection after processing with additional safety check
    QTimer::singleShot(100, socket, [socket]() {
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            socket->disconnectFromHost();
        }
    });
}
void ContainerServer::parseXML(const QString &xmlData)
{
    if (xmlData.isEmpty()) {
        qDebug() << "Empty XML data received";
        return;
    }

    // Clear previous data only when we receive new data
    model->removeRows(0, model->rowCount());

    QXmlStreamReader xml(xmlData);

    // Use static regex to avoid repeated construction (Clazy warning fix)
    static const QRegularExpression codeRegex("^\\d{4}/\\d{2}/[BC]\\d+$");

    QString currentPallet;
    QString type;
    QString code;
    QString height;
    QString breadthOrDiameter;
    QString length;
    QString weight;

    bool hasError = false;
    QString errorMessage;

    try {
        while (!xml.atEnd() && !xml.hasError()) {
            QXmlStreamReader::TokenType token = xml.readNext();

            if (token == QXmlStreamReader::StartElement) {
                QString elementName = xml.name().toString();

                if (elementName == "pallet") {
                    currentPallet = xml.attributes().value("number").toString();
                    qDebug() << "Processing pallet:" << currentPallet;
                }
                else if (elementName == "Box" || elementName == "Cylinder") {
                    type = elementName;
                    code.clear();
                    height.clear();
                    breadthOrDiameter.clear();
                    length.clear();
                    weight.clear();
                    qDebug() << "Found container type:" << type;
                }
                else if (elementName == "code") {
                    code = xml.readElementText().trimmed();
                    qDebug() << "Raw code from XML:" << code;

                    // Use static regex for validation
                    if (!codeRegex.match(code).hasMatch()) {
                        qDebug() << "Code validation failed for:" << code << "- marking as ****";
                        code = "****";
                    } else {
                        qDebug() << "Code validation passed for:" << code;
                    }
                }
                else if (elementName == "height") {
                    height = xml.readElementText().trimmed();
                    qDebug() << "Found height:" << height;
                }
                else if (elementName == "breadth") {
                    breadthOrDiameter = xml.readElementText().trimmed();
                    qDebug() << "Found breadth:" << breadthOrDiameter;
                }
                else if (elementName == "diameter") {
                    breadthOrDiameter = xml.readElementText().trimmed();
                    qDebug() << "Found diameter:" << breadthOrDiameter;
                }
                else if (elementName == "length") {
                    length = xml.readElementText().trimmed();
                    qDebug() << "Found length:" << length;
                }
                else if (elementName == "weight") {
                    weight = xml.readElementText().trimmed();
                    qDebug() << "Found weight:" << weight;
                }
            }
            else if (token == QXmlStreamReader::EndElement) {
                QString elementName = xml.name().toString();
                if (elementName == "Box" || elementName == "Cylinder") {
                    if (currentPallet.isEmpty() || type.isEmpty() || height.isEmpty() || weight.isEmpty()) {
                        qDebug() << "Incomplete container data - skipping";
                        continue;
                    }

                    if (type == "Cylinder") {
                        length = "";
                    }

                    QList<QStandardItem *> row;
                    row << new QStandardItem(currentPallet)
                        << new QStandardItem(type)
                        << new QStandardItem(code)
                        << new QStandardItem(height)
                        << new QStandardItem(breadthOrDiameter)
                        << new QStandardItem(length)
                        << new QStandardItem(weight);

                    model->appendRow(row);

                    qDebug() << "Added row - Pallet:" << currentPallet << "Code:" << code
                             << "Type:" << type << "Height:" << height
                             << "Breadth/Diameter:" << breadthOrDiameter
                             << "Length:" << length << "Weight:" << weight;
                }
            }
        }
    } catch (...) {
        hasError = true;
        errorMessage = "Exception occurred during XML parsing";
    }

    if (xml.hasError()) {
        errorMessage = "Failed to parse XML: " + xml.errorString();
        hasError = true;
    }

    if (hasError) {
        qDebug() << "XML Parse Error:" << errorMessage;
        QMessageBox::warning(this, "XML Error", errorMessage);
    } else {
        qDebug() << "XML parsed successfully. Total rows:" << model->rowCount();
        if (model->rowCount() > 0) {
            QMessageBox::information(this, "Data Received",
                                     QString("Successfully received and parsed data for %1 containers")
                                         .arg(model->rowCount()));
        } else {
            QMessageBox::information(this, "Data Received", "XML received but no container data found.");
        }
    }
}
