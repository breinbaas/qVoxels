#include <QVariantMap>

#include "cptmapmanager.h"
#include "cpt.h"
#include "project.h"


CptMapManager::CptMapManager() {}

void CptMapManager::refreshFromProject(Project *project) {
    if (!project) return;

    m_cptList.clear();

    const QList<Cpt*>& projectCpts = project->cpts();
    for (const Cpt* cpt : projectCpts) {
        if (!cpt) continue;

        QVariantMap mapItem;
        mapItem["name"] = cpt->name();
        mapItem["latitude"] = cpt->latitude();
        mapItem["longitude"] = cpt->longitude();

        m_cptList.append(mapItem);
    }
    emit cptListChanged();

}