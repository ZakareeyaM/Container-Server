#ifndef PALLET_H
#define PALLET_H

#include "container.h"
#include <QList>

class Pallet
{
public:
    explicit Pallet(int number = 0);
    ~Pallet();


    void addContainer(Container* container);
    void removeContainer(Container* container);

    int getNumber() const { return number; }
    int getTotalWeight() const;
    int getTotalVolume() const;


    const QList<Container*>& getContainers() const { return containers; }

private:
    int number;
    QList<Container*> containers;
};

#endif // PALLET_H
