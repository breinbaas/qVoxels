#ifndef SOILPROFILE_H
#define SOILPROFILE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include "SoilLayer.h"

class SoilProfile : public QObject {
    Q_OBJECT
    Q_PROPERTY(int c READ c WRITE setC NOTIFY cChanged)
    Q_PROPERTY(QString location READ location WRITE setLocation NOTIFY locationChanged)
    Q_PROPERTY(double x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(double y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(QList<QObject*> soilLayers READ soilLayers NOTIFY soilLayersChanged)

public:
    explicit SoilProfile(QObject *parent = nullptr) : QObject(parent), m_c(0), m_x(0.0), m_y(0.0) {}

    // Main Deserializer Method
    static SoilProfile* fromJson(const QJsonObject &json, QObject *parent = nullptr) {
        auto *profile = new SoilProfile(parent);

        profile->setC(json["c"].toInt());
        profile->setLocation(json["location"].toString());
        profile->setX(json["x"].toDouble());
        profile->setY(json["y"].toDouble());

        // Parse nested array
        QJsonArray layersArray = json["soil_layers"].toArray();
        for (const QJsonValue &value : layersArray) {
            if (value.isObject()) {
                // Pass 'profile' as parent for automatic memory management
                SoilLayer *layer = SoilLayer::fromJson(value.toObject(), profile);
                profile->m_soilLayers.append(layer);
            }
        }
        emit profile->soilLayersChanged();
        return profile;
    }

    // Getters and Setters
    int c() const { return m_c; }
    QString location() const { return m_location; }
    double x() const { return m_x; }
    double y() const { return m_y; }
    QList<QObject*> soilLayers() const { return m_soilLayers; }

    void setC(int c) { if (m_c != c) { m_c = c; emit cChanged(); } }
    void setLocation(const QString &loc) { if (m_location != loc) { m_location = loc; emit locationChanged(); } }
    void setX(double x) { if (m_x != x) { m_x = x; emit xChanged(); } }
    void setY(double y) { if (m_y != y) { m_y = y; emit yChanged(); } }

signals:
    void cChanged();
    void locationChanged();
    void xChanged();
    void yChanged();
    void soilLayersChanged();

private:
    int m_c;
    QString m_location;
    double m_x;
    double m_y;
    QList<QObject*> m_soilLayers;
};

#endif // SOILPROFILE_H