#ifndef CPTMAPMANAGER_H
#define CPTMAPMANAGER_H

#include <QObject>
#include <QVariant>
#include <QVariantList>

class Project;

class CptMapManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList cptList READ cptList NOTIFY cptListChanged)
public:
    CptMapManager();

    explicit CptMapManager(QObject *parent = nullptr) : QObject(parent) {}
    QVariantList cptList() const { return m_cptList; }
    void refreshFromProject(Project *project);


signals:
    void cptListChanged();
    void qmlAreaSelected(double minLat, double minLng, double maxLat, double maxLng);

public slots:
    void reportSelectedArea(double minLat, double minLng, double maxLat, double maxLng) {
        emit qmlAreaSelected(minLat, minLng, maxLat, maxLng);
    }

private:
    QVariantList m_cptList;
};

#endif // CPTMAPMANAGER_H
