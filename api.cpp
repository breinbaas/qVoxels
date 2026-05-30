#include "api.h"
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QCoreApplication>

#include "soilprofile.h"

Api::Api(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    // Connect the manager's global finished signal to our handler
    connect(m_manager, &QNetworkAccessManager::finished, this, &Api::onReplyFinished);

    QString envPath = QCoreApplication::applicationDirPath() + "/.env";
    QFile file(envPath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();

            // Skip comments or empty lines
            if (line.isEmpty() || line.startsWith('#')) {
                continue;
            }

            // Split by the first '=' character
            int separatorIdx = line.indexOf('=');
            if (separatorIdx != -1) {
                QString key = line.left(separatorIdx).trimmed();
                QString value = line.mid(separatorIdx + 1).trimmed();

                if (key == "BREINBAAS_API_KEY") {
                    // Strip quotes if they exist in the .env file
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
        qDebug() << "No API key found!";
    }
}

Api::~Api()
{
    // QNetworkAccessManager child cleanup handled automatically by Qt parent tree
}

void Api::uploadCptGef(const QString &cptName, const QString &filePath, int method, double minLayerHeight, double peatFrictionRatio)
{
    QFile *file = new QFile(filePath, this);
    if (!file->open(QIODevice::ReadOnly)) {
        emit errorOccurred(QString("Failed to open file: %1").arg(filePath));
        file->deleteLater();
        return;
    }

    // 1. Prepare Target Endpoint & Security Headers
    QUrl url(m_baseUrl + "/api/slim/cpt_interpretation/from_gef");
    QNetworkRequest request(url);
    request.setRawHeader("accept", "application/json");
    request.setRawHeader("X-API-Key", m_apiKey.toUtf8());

    // 2. Initialize Multipart Form Wrapper
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);

    // 3. Build File Part
    QHttpPart filePart;
    QFileInfo fileInfo(filePath);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileInfo.fileName()));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    filePart.setBodyDevice(file);
    file->setParent(multiPart); // Let multiPart track device lifespan
    multiPart->append(filePart);

    // 4. Build Configuration Metadata JSON Part
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

    // 5. Fire Request
    QNetworkReply *reply = m_manager->post(request, multiPart);
    multiPart->setParent(reply); // Automatically delete multipart metadata structures when raw transaction ends
}

void Api::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        return;
    }

    // Read payload payload
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