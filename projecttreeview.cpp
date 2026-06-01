#include "projecttreeview.h"

#include <QMouseEvent>
#include <QApplication>
#include <QMenu>

#include "project.h"
#include "cpt.h"

ProjectTreeView::ProjectTreeView(QWidget *parent)
    : QTreeWidget{parent}
    , m_project(nullptr)
{
    setHeaderHidden(true);
    setColumnCount(1);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QTreeWidget::itemClicked, this, &ProjectTreeView::handleItemClicked);
    connect(this, &QWidget::customContextMenuRequested, this, &ProjectTreeView::showContextMenu);
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

    QTreeWidgetItem *interpretationsBranchItem = new QTreeWidgetItem(projectRootItem);
    interpretationsBranchItem->setText(0, "Interpretations");
    for (Cpt *cpt : m_project->cpts()) {
        if(cpt->soilProfile()){
            QTreeWidgetItem *spItem = new QTreeWidgetItem(interpretationsBranchItem);
            spItem->setText(0, cpt->name());
            // Store the Cpt* in a custom user role for later retrieval
            spItem->setData(0, Qt::UserRole, QVariant::fromValue(cpt->soilProfile()));
        }
    }


    projectRootItem->setExpanded(true); // Expand the project root by default
    cptsBranchItem->setExpanded(true);  // Expand the CPTs branch by default
    interpretationsBranchItem->setExpanded(true);


    // Ensure the project root item is visible and selected if it's the only one
    if (topLevelItemCount() > 0) {
        setCurrentItem(topLevelItem(0));
    }

}

void ProjectTreeView::handleItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (!item) {
        return;
    }

    // Since this signal fires perfectly for standard select/click actions,
    // we don't need to manually check QApplication::mouseButtons().
    QVariant data = item->data(0, Qt::UserRole);
    if (data.canConvert<Cpt*>()) {
        Cpt *clickedCpt = data.value<Cpt*>();
        if (clickedCpt) {
            emit cptSelected(clickedCpt);
        }
    // }else if (data.canConvert<SoilProfile*>()) {
    //     SoilProfile *clickedSoilProfile = data.value<SoilProfile*>();
    //     if (clickedSoilProfile) {
    //         emit soilProfileSelected(clickedSoilProfile);
    //     }
    } else if (data.canConvert<VoxelModel*>()) {
        VoxelModel *clickedVoxelModel = data.value<VoxelModel*>();
        if (clickedVoxelModel) {
            emit voxelModelSelected(clickedVoxelModel);
        }
    }
}

void ProjectTreeView::showContextMenu(const QPoint &pos)
{
    // 1. Get the item at the right-clicked position
    QTreeWidgetItem *item = itemAt(pos);
    if (!item) {
        return;
    }

    // 2. Extract and verify data type
    QVariant data = item->data(0, Qt::UserRole);
    if (!data.canConvert<Cpt*>()) {
        return; // Only show menu for actual CPT items
    }

    Cpt *clickedCpt = data.value<Cpt*>();
    if (!clickedCpt) {
        return;
    }

    // 3. Create and populate the context menu
    QMenu menu(this);
    QAction *getInterpretationAction = menu.addAction("Get Interpretation");
    QAction *removeAction = menu.addAction("Remove");

    // 4. Map the local widget position to global screen position for the popup
    QPoint globalPos = mapToGlobal(pos);
    QAction *selectedAction = menu.exec(globalPos);

    // 5. Respond to the chosen action
    if (selectedAction == getInterpretationAction) {
        emit getCptInterpretationSelected(clickedCpt);

    } /*else if (selectedAction == removeAction) {
        // Handle "Remove" data logic safely:
        if (m_project) {
            // e.g., m_project->removeCpt(clickedCpt);
        }

        // Remove visually from the QTreeWidget
        delete item;
    }*/
}
