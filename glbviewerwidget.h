#ifndef GLBVIEWERWIDGET_H
#define GLBVIEWERWIDGET_H

#include <QQuickWidget>
#include <QUrl>

class GlbViewerWidget : public QQuickWidget
{
    Q_OBJECT
public:
    explicit GlbViewerWidget(QWidget *parent = nullptr);
    void loadModel(const QString &localFilePath);
    void clearModel();

private slots:
    void onWidgetStatusChanged(QQuickWidget::Status status);

private:
    QString m_pendingModelPath; // Stores the path if QML isn't loaded yet
};

#endif // GLBVIEWERWIDGET_H