#pragma once

#include <QFrame>

class QQuickWidget;
class CptMapManager; // Forward declaration
class Project;       // Forward declaration for your Project class

class CptMap : public QFrame {
    Q_OBJECT

public:
    explicit CptMap(QWidget *parent = nullptr);
    ~CptMap() override;

    // Pass your Project pointer directly to update the map canvas
    void setProject(Project *project);

signals:
    // Forwarded from QML area selection bounding box bounding rectangles
    void areaSelected(double minLat, double minLng, double maxLat, double maxLng);
    void cptListChanged();

private:
    QQuickWidget *m_quickWidget;
    CptMapManager *m_mapManager;
    void initUi();
};