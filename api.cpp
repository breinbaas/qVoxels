#include "api.h"
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QStandardPaths>

#include "soilprofile.h"
#include "soillayer.h"

Api::Api(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    // reads the .env file if any to get the api key that should be used for API requests.
    QString envPath = QCoreApplication::applicationDirPath() + "/.env";
    QFile file(envPath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();

            if (line.isEmpty() || line.startsWith('#')) {
                continue;
            }

            int separatorIdx = line.indexOf('=');
            if (separatorIdx != -1) {
                QString key = line.left(separatorIdx).trimmed();
                QString value = line.mid(separatorIdx + 1).trimmed();

                if (key == "BREINBAAS_API_KEY") {
                    if (value.startsWith('"') && value.endsWith('"')) {
                        value = value.mid(1, value.length() - 2);
                    }
                    m_apiKey = value;
                    break;
                }
            }
        }
        file.close();
    }

    if(m_apiKey.isEmpty()) {
        qWarning() << "No API key found!";
    }
}

Api::~Api()
{

}

void Api::getInterpretationFromCpt(const QString &cptName, const QString &filePath, int method, double minLayerHeight, double peatFrictionRatio)
{
    // this function will use the Python API to get an interpretation of a CPT
    QFile *file = new QFile(filePath, this);
    if (!file->open(QIODevice::ReadOnly)) {
        emit errorOccurred(QString("Failed to open file: %1").arg(filePath));
        file->deleteLater();
        return;
    }

    QUrl url(m_baseUrl + "/api/slim/cpt_interpretation/from_gef");
    QNetworkRequest request(url);
    request.setRawHeader("accept", "application/json");
    request.setRawHeader("X-API-Key", m_apiKey.toUtf8());

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);

    QHttpPart filePart;
    QFileInfo fileInfo(filePath);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileInfo.fileName()));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    filePart.setBodyDevice(file);
    file->setParent(multiPart); // Let multiPart track device lifespan
    multiPart->append(filePart);

    QJsonObject configJson;
    configJson["method"] = method;
    configJson["minimum_layerheight"] = minLayerHeight;
    configJson["peat_friction_ratio"] = peatFrictionRatio;
    configJson["cpt_name"] = cptName;

    QJsonDocument doc(configJson);
    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"request\"");
    textPart.setBody(doc.toJson(QJsonDocument::Compact));
    multiPart->append(textPart);

    QNetworkReply *reply = m_manager->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        this->onReplyInterpretation(reply);
    });
}

void Api::getVoxelModel(QList<SoilProfile*> soilProfiles, ApiVoxelModelRequestParameters &parameters, const QString &modelsDirPath)
{
    // TODO check if there is anything to create (soilProfiles > 0)
    // use the API to generate a voxel model and save it to the given path using the current date and time
    QJsonArray profilesArray;
    for (SoilProfile* profile : soilProfiles) {
        if (!profile) continue;

        QJsonObject profileObj;
        profileObj["x"] = profile->x();
        profileObj["y"] = profile->y();

        QJsonArray layersArray;
        for (QObject* obj : profile->soilLayers()) {
            SoilLayer* layer = qobject_cast<SoilLayer*>(obj);
            if (layer) {
                layersArray.append(layer->toJson());
            }
        }
        profileObj["soil_layers"] = layersArray;
        profilesArray.append(profileObj);
    }

    // TODO > own colors
    QJsonObject colorsObj;
    QStringList colorKeys = {"preexcavated", "organic_clay", "clay", "silty_clay", "silty_sand", "sand", "dense_sand", "peat"};
    for (const QString& key : colorKeys) {
        colorsObj[key] = parameters.palette.getColor(key).name(); // Extract CSS hex strings e.g. #fff000
    }

    // TODO -> parameters to payload function
    QJsonObject mainPayload;
    mainPayload["soil_profiles"] = profilesArray;
    mainPayload["x_min"] = parameters.xmin;
    mainPayload["x_max"] = parameters.xmax;
    mainPayload["dx"] = parameters.dx;
    mainPayload["y_min"] = parameters.ymin;
    mainPayload["y_max"] = parameters.ymax;
    mainPayload["dy"] = parameters.dy;
    mainPayload["z_min"] = parameters.zmin;
    mainPayload["z_max"] = parameters.zmax;
    mainPayload["dz"] = parameters.dz;
    mainPayload["anisotropy_ratio"] = parameters.anisotropy_ratio;
    mainPayload["step_size"] = parameters.step_size;
    mainPayload["soil_colors"] = colorsObj;
    mainPayload["center"] = true;

    QJsonDocument doc(mainPayload);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    QUrl url(m_baseUrl + "/api/voxels/export/glb/3d");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("accept", "application/json");
    request.setRawHeader("X-API-Key", m_apiKey.toUtf8());

    QNetworkReply *reply = m_manager->post(request, jsonData);

    connect(reply, &QNetworkReply::finished, this, [this, reply, modelsDirPath]() {
        this->onReplyVoxelModel(reply, modelsDirPath);
    });
}

void Api::onReplyInterpretation(QNetworkReply *reply)
{
    // called if the API returns an interpretation
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred(QString("JSON Parsing Error: %1").arg(parseError.errorString()));
        return;
    }

    QJsonObject rootObject = doc.object();
    QString cptName = rootObject["cpt_name"].toString();

    if (rootObject.contains("soil_profile") && rootObject["soil_profile"].isObject()) {
        SoilProfile *profile = SoilProfile::fromJson(rootObject["soil_profile"].toObject(), this);
        emit interpretationReceived(cptName, profile);
    } else {
        emit errorOccurred("Response was valid JSON, but could not be converted to a SoilProfile.");
    }
}

void Api::onReplyVoxelModel(QNetworkReply *reply, const QString &modelsDirPath){
    // called if a voxel model is returned
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Voxel API Error: %1").arg(reply->errorString()));
        return;
    }

    QString systemDownloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timestamp = currentDateTime.toString("yyyyMMddhhmmss");
    QString fileName = timestamp + ".glb";
    QString savePath = modelsDirPath  + "/" +  fileName;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred(QString("Could not open destination path for writing file: %1").arg(savePath));
        return;
    }

    QByteArray rawBinaryGlb = reply->readAll();
    qint64 writtenBytes = file.write(rawBinaryGlb);
    file.close();

    if (writtenBytes > 0) {
        qDebug() << "Successfully downloaded and saved 3D Voxel Engine assets to:" << savePath;        
        emit voxelModelReceived(savePath);
    } else {
        emit errorOccurred("The server response file was returned completely empty.");
    }
}