#include "soilprofile.h"
#include "soillayer.h"

#include <QFile>

SoilProfile* SoilProfile::fromJson(const QJsonObject &json, QObject *parent) {
    auto *profile = new SoilProfile(parent);

    profile->setC(json["c"].toInt());
    profile->setLocation(json["location"].toString());
    profile->setX(json["x"].toDouble());
    profile->setY(json["y"].toDouble());

    QJsonArray layersArray = json["soil_layers"].toArray();
    for (const QJsonValue &value : layersArray) {
        if (value.isObject()) {
            SoilLayer *layer = SoilLayer::fromJson(value.toObject(), profile);
            profile->m_soilLayers.append(layer);
        }
    }
    emit profile->soilLayersChanged();
    return profile;
}

SoilProfile *SoilProfile::fromJsonFile(const QString &filePath, QObject *parent)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for reading:" << filePath << file.errorString();
        return nullptr;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return nullptr;
    }

    if (!doc.isObject()) {
        qWarning() << "JSON document is not a valid object.";
        return nullptr;
    }

    return SoilProfile::fromJson(doc.object(), parent);
}

bool SoilProfile::toJsonFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath << file.errorString();
        return false;
    }

    QJsonObject jsonObject = toJson();
    QJsonDocument doc(jsonObject);
    file.write(doc.toJson(QJsonDocument::Indented));

    file.close();
    return true;
}

QJsonObject SoilProfile::toJson() const {
    QJsonObject json;
    json["c"] = m_c;
    json["location"] = m_location;
    json["x"] = m_x;
    json["y"] = m_y;

    QJsonArray layersArray;
    for (QObject* obj : m_soilLayers) {
        if (auto *layer = qobject_cast<SoilLayer*>(obj)) {
            layersArray.append(layer->toJson());
        }
    }
    json["soil_layers"] = layersArray;

    return json;
}