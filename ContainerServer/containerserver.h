#ifndef CONTAINERSERVER_H
#define CONTAINERSERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QStandardItemModel>
#include <QAbstractSocket>

QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
QT_END_NAMESPACE

class ContainerServer : public QMainWindow
{
    Q_OBJECT

public:
    explicit ContainerServer(QWidget *parent = nullptr);
    ~ContainerServer();

private slots:
    void newConnection();
    void readClientData();
    void clientDisconnected();
    void toggleListening();
    void socketError(QAbstractSocket::SocketError error); // New slot for error handling

private:
    void parseXML(const QString &xmlData);

    QTcpServer *tcpServer;
    QTcpSocket *clientSocket;
    QStandardItemModel *model;
    QPushButton *startListeningButton;
    QLabel *statusLabel;
    bool isListening;
};

#endif // CONTAINERSERVER_H
