#pragma once

#include <QFrame>

class QQuickWidget;
class CptMapManager;
class Project;

class CptMap : public QFrame {
    Q_OBJECT

public:
    explicit CptMap(QWidget *parent = nullptr);
    ~CptMap() override;

    void setProject(Project *project);

signals:    
    void areaSelected(double minLat, double minLng, double maxLat, double maxLng);

private:
    QQuickWidget *m_quickWidget;
    CptMapManager *m_mapManager;
    void initUi();
};