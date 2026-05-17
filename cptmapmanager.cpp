#include <QVariantMap>

#include "cptmapmanager.h"
#include "cpt.h"
#include "project.h"


CptMapManager::CptMapManager() {}

void CptMapManager::refreshFromProject(Project *project) {
    if (!project) return;

    m_cptList.clear();

    // Iterate through your QList<Cpt*> map collection
    const QList<Cpt*>& projectCpts = project->cpts();
    for (const Cpt* cpt : projectCpts) {
        if (!cpt) continue;

        QVariantMap mapItem;
        // Adjust these method calls (.id(), .latitude()) to match your actual Cpt.h signatures
        mapItem["name"] = cpt->name();
        mapItem["latitude"] = cpt->latitude();
        mapItem["longitude"] = cpt->longitude();

        m_cptList.append(mapItem);
    }
    emit cptListChanged();

}