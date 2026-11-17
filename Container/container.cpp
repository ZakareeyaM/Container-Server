#include "container.h"
#include <QtMath>  // for qRound, M_PI

// ---------------- Container ----------------
Container::Container(int weight) : weight(weight) {}
Container::~Container() = default;

QString Container::getCode() const { return code; }
void Container::setCode(const QString& c) { code = c; }

int Container::getWeight() const { return weight; }
void Container::setWeight(int w) { weight = w; }

// ---------------- Box ----------------
Box::Box(int length, int breadth, int height, int weight)
    : Container(weight), length(length), breadth(breadth), height(height) {}

int Box::getLength() const { return length; }
void Box::setLength(int v) { length = v; }

int Box::getBreadth() const { return breadth; }
void Box::setBreadth(int v) { breadth = v; }

int Box::getHeight() const { return height; }
void Box::setHeight(int v) { height = v; }

int Box::getVolume() const { return length * breadth * height; }
QString Box::getType() const { return "Box"; }
Container* Box::clone() const { return new Box(*this); }

// ---------------- Cylinder ----------------
Cylinder::Cylinder(int diameter, int height, int weight)
    : Container(weight), diameter(diameter), height(height) {}

int Cylinder::getDiameter() const { return diameter; }
void Cylinder::setDiameter(int d) { diameter = d; }

int Cylinder::getHeight() const { return height; }
void Cylinder::setHeight(int h) { height = h; }

int Cylinder::getVolume() const {
    double r = diameter / 2.0;
    double vol = M_PI * r * r * height;
    return static_cast<int>(qRound(vol));
}

QString Cylinder::getType() const { return "Cylinder"; }
Container* Cylinder::clone() const { return new Cylinder(*this); }
