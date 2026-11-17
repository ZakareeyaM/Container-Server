#include "containerfactory.h"
#include <QDate>

int ContainerFactory::boxSerial = 1;
int ContainerFactory::cylinderSerial = 1;

Container* ContainerFactory::createContainer(ContainerType type, const QVariantList& params) {
    Container* container = nullptr;

    if (type == BOX && params.size() >= 4) {
        // params: length, breadth, height, weight
        container = new Box(params[0].toInt(), params[1].toInt(), params[2].toInt(), params[3].toInt());
    } else if (type == CYLINDER && params.size() >= 3) {
        // params: diameter, height, weight
        container = new Cylinder(params[0].toInt(), params[1].toInt(), params[2].toInt());
    }

    if (container) {
        container->setCode(generateCode(type));
    }

    return container;
}

QString ContainerFactory::generateCode(ContainerType type) {
    QDate currentDate = QDate::currentDate();
    QString code = QString("%1/%2/%3%4")
                       .arg(currentDate.year())
                       .arg(currentDate.month(), 2, 10, QChar('0'))
                       .arg(type == BOX ? 'B' : 'C')
                       .arg(type == BOX ? boxSerial++ : cylinderSerial++);
    return code;
}
