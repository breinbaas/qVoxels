#include "projecttreeview.h"

#include "project.h"
#include "cpt.h"

ProjectTreeView::ProjectTreeView(QWidget *parent)
    : QTreeWidget{parent}
    , m_project(nullptr)
{
    setHeaderHidden(true);
    setColumnCount(1);
}

void ProjectTreeView::setProject(Project *project)
{
    if (m_project == project) {
        return; // No change
    }

    m_project = project;
    clear();

    if (!m_project) {
        return; // No project to display
    }

    // 1. Create the root item for the project itself
    QTreeWidgetItem *projectRootItem = new QTreeWidgetItem(this); // Add to the top-level of the QTreeWidget
    // projectRootItem->setText(0, m_project->name());
    projectRootItem->setText(0, "Project");
    // Store the Project* in a custom user role for later retrieval
    projectRootItem->setData(0, Qt::UserRole, QVariant::fromValue(m_project));

    // 2. Add the "CPTs" branch as a child of the project root
    QTreeWidgetItem *cptsBranchItem = new QTreeWidgetItem(projectRootItem);
    cptsBranchItem->setText(0, "CPTs");
    // Optionally store a type identifier if you need to distinguish this branch later
    // cptsBranchItem->setData(0, Qt::UserRole + 1, "CPTsBranch");

    // 3. Populate CPT children under the "CPTs" branch
    for (Cpt *cpt : m_project->cpts()) {
        QTreeWidgetItem *cptItem = new QTreeWidgetItem(cptsBranchItem);
        cptItem->setText(0, cpt->name());
        // Store the Cpt* in a custom user role for later retrieval
        cptItem->setData(0, Qt::UserRole, QVariant::fromValue(cpt));
    }

    projectRootItem->setExpanded(true); // Expand the project root by default
    cptsBranchItem->setExpanded(true);  // Expand the CPTs branch by default

    // Ensure the project root item is visible and selected if it's the only one
    if (topLevelItemCount() > 0) {
        setCurrentItem(topLevelItem(0));
    }

}
