#include "glbviewerwidget.h"
#include <QSurfaceFormat>
#include <QQuick3D>
#include <QQuickItem>
#include <QFileInfo>
#include <QDebug>

GlbViewerWidget::GlbViewerWidget(QWidget *parent)
    : QQuickWidget(parent)
{
    setFormat(QQuick3D::idealSurfaceFormat());
    setResizeMode(QQuickWidget::SizeRootObjectToView);

    connect(this, &QQuickWidget::statusChanged, this, &GlbViewerWidget::onWidgetStatusChanged);

    setSource(QUrl(QStringLiteral("qrc:/glbviewer.qml")));

    if (status() == QQuickWidget::Error) {
        qDebug() << "Immediate QML Errors:";
        for (const QQmlError &err : errors()) {
            qDebug() << err.toString();
        }
    }
}

void GlbViewerWidget::loadModel(const QString &localFilePath)
{
    if (status() != QQuickWidget::Ready) {
        qDebug() << "GlbViewerWidget: QML not ready yet. Caching path:" << localFilePath;
        m_pendingModelPath = localFilePath;
        return;
    }

    QFileInfo checkFile(localFilePath);
    if (!checkFile.exists() || !checkFile.isFile()) {
        qWarning() << "GlbViewerWidget: File does not exist ->" << localFilePath;
        return;
    }

    QObject *rootObj = rootObject();
    if (rootObj) {
        QUrl modelUrl = QUrl::fromLocalFile(localFilePath);
        rootObj->setProperty("modelSource", modelUrl);
        m_pendingModelPath.clear(); // Clear cache on success
    }
}

void GlbViewerWidget::onWidgetStatusChanged(QQuickWidget::Status status)
{
    if (status == QQuickWidget::Ready) {
        qDebug() << "GlbViewerWidget: QML engine is Ready!";
        if (!m_pendingModelPath.isEmpty()) {
            loadModel(m_pendingModelPath);
        }
    } else if (status == QQuickWidget::Error) {
        qCritical() << "GlbViewerWidget: Critical QML Loading Errors:";
        for (const QQmlError &error : errors()) {
            qCritical() << "  Line" << error.line() << ":" << error.toString();
        }
    }
}

void GlbViewerWidget::clearModel()
{
    m_pendingModelPath.clear();
    QObject *rootObj = rootObject();
    if (rootObj) {
        rootObj->setProperty("modelSource", QUrl(""));
    }
}