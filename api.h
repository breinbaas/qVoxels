#ifndef API_H
#define API_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>

class Api : public QObject
{
    Q_OBJECT
public:
    explicit Api(QObject *parent = nullptr);
    ~Api();

    // Main execution endpoint
    void uploadCptGef(const QString &filePath, int method, double minLayerHeight, double peatFrictionRatio);

signals:
    void interpretationReady(const QJsonObject &jsonResult);
    void errorOccurred(const QString &errorString);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
    const QString m_baseUrl = "http://127.0.0.1:8000";
    QString m_apiKey;
};

#endif // API_H