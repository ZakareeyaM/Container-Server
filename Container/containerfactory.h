#ifndef CONTAINERFACTORY_H
#define CONTAINERFACTORY_H

#include "container.h"
#include <QVariantList>
#include <QString>

class ContainerFactory
{
public:
    enum ContainerType { BOX, CYLINDER };

    static Container* createContainer(ContainerType type, const QVariantList& params);
    static QString generateCode(ContainerType type);

private:
    static int boxSerial;
    static int cylinderSerial;
};

#endif // CONTAINERFACTORY_H
