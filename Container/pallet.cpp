#include "pallet.h"
#include <QtGlobal>

Pallet::Pallet(int number) : number(number) {}

Pallet::~Pallet() {
    qDeleteAll(containers);
    containers.clear();
}

void Pallet::addContainer(Container* container) {
    if (container) containers.append(container);
}

void Pallet::removeContainer(Container* container) {
    containers.removeOne(container);
}

int Pallet::getTotalWeight() const {
    int total = 0;
    for (const Container* c : containers) total += c->getWeight();
    return total;
}

int Pallet::getTotalVolume() const {
    int total = 0;
    for (const Container* c : containers) total += c->getVolume();
    return total;
}

