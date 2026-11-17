#include "containerserver.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ContainerServer w;
    w.show();
    return a.exec();
}
