#ifndef CONTAINERWIDGET_H
#define CONTAINERWIDGET_H
#include <QWidget>
#include <QMap>
#include <QList>
#include "container.h"
#include "pallet.h"

class QSpinBox;
class QPushButton;
class QListWidget;
class QTextEdit;
class QTabWidget;
class ContainerMemento;

class ContainerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContainerWidget(QWidget *parent = nullptr);
    ~ContainerWidget();

    // Public slots used by main window actions
public slots:
    void handleBackup();
    void handleRestore();
    void handlePostXML();

private slots:
    void addBox();
    void addCylinder();
    void moveToPallet();
    void onSelectionChanged();

private:
    void setupUI();
    void updateUnallocatedList();
    void clearInputs();
    void enableDisableButtons();
    void sendXMLToServer(const QString& xml);

    // New methods for file-based backup/restore
    bool saveContainersToFile(const QString& filename);
    bool loadContainersFromFile(const QString& filename);

    // UI
    QTabWidget *tabWidget;
    QSpinBox *box_breadth, *box_length, *box_height, *box_weight;
    QSpinBox *cylinder_diameter, *cylinder_height, *cylinder_weight;
    QPushButton *buttonAddBox, *buttonAddCylinder;
    QListWidget *displayUnallocated;
    QSpinBox *palletNumber;
    QPushButton *buttonMoveToPallet, *buttonBackupRestore;
    QPushButton *buttonPostXML;
    QTextEdit *displayPalletXML;

    // Data
    QList<Container*> unallocatedContainers;
    QMap<int, Pallet*> pallets;
    ContainerMemento* memento;
};

class ContainerMemento
{
public:
    void saveState(const QList<Container*>& containers);
    QList<Container*> restoreState();

private:
    QList<Container*> savedContainers;
};

#endif // CONTAINERWIDGET_H
