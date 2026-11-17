
#include "serializing.h"
#include "container.h"

#include <QXmlStreamWriter>
#include <QMetaProperty>
#include <QVariant>

Serializing::Serializing(const QMap<int, Pallet*>& pallets, QObject* parent)
    : QObject(parent), pallets(pallets)
{
}

void Serializing::run()
{
    QString xml = generateXML();
    emit finished(xml);
}

QString Serializing::generateXML() const
{
    QString xml;
    QXmlStreamWriter writer(&xml);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("pallets");
    writer.writeAttribute("NumberOfPallets", QString::number(pallets.size()));

    for (auto it = pallets.constBegin(); it != pallets.constEnd(); ++it) {
        Pallet* pallet = it.value();
        writer.writeStartElement("pallet");
        writer.writeAttribute("weight", QString::number(pallet->getTotalWeight()));
        writer.writeAttribute("volume", QString::number(pallet->getTotalVolume()));
        writer.writeAttribute("number", QString::number(pallet->getNumber()));

        for (const Container* container : pallet->getContainers()) {
            writer.writeStartElement(container->getType());

            // Use meta-object information for properties
            const QMetaObject* metaObj = (container->getType() == "Box") ? &Box::staticMetaObject : &Cylinder::staticMetaObject;

            for (int i = 0; i < metaObj->propertyCount(); ++i) {
                QMetaProperty prop = metaObj->property(i);
                QString propName = prop.name();

                // Use readOnGadget - works because we used Q_GADGET for Box/Cylinder
                QVariant value = prop.readOnGadget(const_cast<void*>(reinterpret_cast<const void*>(container)));

                // skip volume property if present
                if (propName != "volume") {
                    writer.writeTextElement(propName, value.toString());
                }
            }

            writer.writeEndElement(); // Box / Cylinder
        }

        writer.writeEndElement(); // pallet
    }

    writer.writeEndElement(); // pallets
    writer.writeEndDocument();

    return xml;
}
