#ifndef CONTAINER_H
#define CONTAINER_H

#include <QString>
#include <QMetaObject>

// Base Container class + Box + Cylinder using Q_GADGET for meta-properties
class Container
{
public:
    Container(int weight = 0);
    virtual ~Container();

    virtual QString getType() const = 0;
    virtual int getVolume() const = 0;
    virtual Container* clone() const = 0;

    QString getCode() const;
    void setCode(const QString& c);

    int getWeight() const;
    void setWeight(int w);

protected:
    QString code;
    int weight;
};

// Box - uses Q_GADGET to expose properties
class Box : public Container
{
    Q_GADGET
    Q_PROPERTY(QString code READ getCode WRITE setCode)
    Q_PROPERTY(int weight READ getWeight WRITE setWeight)
    Q_PROPERTY(int length READ getLength WRITE setLength)
    Q_PROPERTY(int breadth READ getBreadth WRITE setBreadth)
    Q_PROPERTY(int height READ getHeight WRITE setHeight)

public:
    Box(int length = 0, int breadth = 0, int height = 0, int weight = 0);

    int getLength() const;
    void setLength(int v);

    int getBreadth() const;
    void setBreadth(int v);

    int getHeight() const;
    void setHeight(int v);

    int getVolume() const override;
    QString getType() const override;
    Container* clone() const override;

private:
    int length, breadth, height;
};

// Cylinder - uses Q_GADGET to expose properties
class Cylinder : public Container
{
    Q_GADGET
    Q_PROPERTY(QString code READ getCode WRITE setCode)
    Q_PROPERTY(int weight READ getWeight WRITE setWeight)
    Q_PROPERTY(int diameter READ getDiameter WRITE setDiameter)
    Q_PROPERTY(int height READ getHeight WRITE setHeight)

public:
    Cylinder(int diameter = 0, int height = 0, int weight = 0);

    int getDiameter() const;
    void setDiameter(int d);

    int getHeight() const;
    void setHeight(int h);

    int getVolume() const override;
    QString getType() const override;
    Container* clone() const override;

private:
    int diameter, height;
};

#endif // CONTAINER_H
