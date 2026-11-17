
#ifndef SERIALIZING_H
#define SERIALIZING_H

#include <QObject>
#include <QMap>
#include "pallet.h"

class Serializing : public QObject
{
    Q_OBJECT
public:
    explicit Serializing(const QMap<int, Pallet*>& pallets, QObject* parent = nullptr);

public slots:
    void run(); // slot to run in worker thread

signals:
    // Emitted when XML is generated (or empty on failure)
    void finished(const QString& xml);

private:
    QString generateXML() const;

    QMap<int, Pallet*> pallets;
};

#endif // SERIALIZING_H

