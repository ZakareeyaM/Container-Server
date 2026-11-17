#ifndef MAINCLIENT_H
#define MAINCLIENT_H

#include <QMainWindow>

class QAction;
class QMenu;
class QToolBar;

class ContainerWidget;

class MainClient : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainClient(QWidget *parent = nullptr);
    ~MainClient();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showAbout();
    void showHelp();

private:
    void setupActions();
    void setupMenus();

    ContainerWidget *containerWidget;

    QAction *actAbout;
    QAction *actHelp;
    QAction *actExit;
    QAction *actBackup;
    QAction *actRestore;
    QAction *actPostXml;

    QMenu *mnuFile;
    QMenu *mnuBackup;
    QMenu *mnuPost;
    QMenu *mnuHelp;

    QToolBar *toolbar;
};

#endif // MAINCLIENT_H
