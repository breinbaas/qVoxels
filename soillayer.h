#ifndef SOILLAYER_H
#define SOILLAYER_H

#include <QObject>
#include <QString>
#include <QJsonObject>

class SoilLayer : public QObject {
    Q_OBJECT
    Q_PROPERTY(double bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(double top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(QString soilCode READ soilCode WRITE setSoilCode NOTIFY soilCodeChanged)

public:
    explicit SoilLayer(QObject *parent = nullptr) : QObject(parent), m_bottom(0), m_top(0) {}

    // Factory method to create from JSON
    static SoilLayer* fromJson(const QJsonObject &json, QObject *parent = nullptr) {
        auto *layer = new SoilLayer(parent);
        layer->setBottom(json["bottom"].toDouble());
        layer->setTop(json["top"].toDouble());
        layer->setSoilCode(json["soil_code"].toString());
        return layer;
    }

    double bottom() const { return m_bottom; }
    double top() const { return m_top; }
    QString soilCode() const { return m_soilCode; }

    void setBottom(double bottom) { if (m_bottom != bottom) { m_bottom = bottom; emit bottomChanged(); } }
    void setTop(double top) { if (m_top != top) { m_top = top; emit topChanged(); } }
    void setSoilCode(const QString &code) { if (m_soilCode != code) { m_soilCode = code; emit soilCodeChanged(); } }
    QJsonObject toJson() const {
        QJsonObject json;
        json["bottom"] = m_bottom;
        json["top"] = m_top;
        json["soil_code"] = m_soilCode;
        return json;
    }

signals:
    void bottomChanged();
    void topChanged();
    void soilCodeChanged();

private:
    double m_bottom;
    double m_top;
    QString m_soilCode;
};

#endif // SOILLAYER_H