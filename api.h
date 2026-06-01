#ifndef API_H
#define API_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>

#include "soilprofile.h"
#include "soilcolorpalette.h"

class ApiVoxelModelRequestParameters{
public:
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float zmax;
    float zmin;
    float dx;
    float dy;
    float dz;
    float anisotropy_ratio = 0.5;
    float step_size = 0.5;
    SoilColorPalette palette;
 };

class Api : public QObject
{
    Q_OBJECT
public:
    explicit Api(QObject *parent = nullptr);
    ~Api();

    // Main execution endpoint
    void getInterpretationFromCpt(const QString &cptName, const QString &filePath, int method, double minLayerHeight, double peatFrictionRatio);
    void getVoxelModel(QList<SoilProfile*> soilProfiles, ApiVoxelModelRequestParameters &parameters, const QString &modelsDirPath);

signals:
    void interpretationReceived(const QString &cptName, SoilProfile *soilProfile);
    void voxelModelReceived(const QString filePath);
    void errorOccurred(const QString &errorString);

private slots:
    void onReplyInterpretation(QNetworkReply *reply);
    void onReplyVoxelModel(QNetworkReply *reply, const QString &modelsDirPath);

private:
    QNetworkAccessManager *m_manager;
    const QString m_baseUrl = "http://127.0.0.1:8000";
    QString m_apiKey;

};

#endif // API_H