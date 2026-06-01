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
        return;
    }

    m_project = project;
    clear();

    if (!m_project) {
        return;
    }

    QTreeWidgetItem *projectRootItem = new QTreeWidgetItem(this); // Add to the top-level of the QTreeWidget   
    projectRootItem->setText(0, "Project");    
    projectRootItem->setData(0, Qt::UserRole, QVariant::fromValue(m_project));

    // cpts
    QTreeWidgetItem *cptsBranchItem = new QTreeWidgetItem(projectRootItem);
    cptsBranchItem->setText(0, "CPTs");
    for (Cpt *cpt : m_project->cpts()) {
        QTreeWidgetItem *cptItem = new QTreeWidgetItem(cptsBranchItem);
        cptItem->setText(0, cpt->name());
        // Store the Cpt* in a custom user role for later retrieval
        cptItem->setData(0, Qt::UserRole, QVariant::fromValue(cpt));
    }

    // interpretations
    QTreeWidgetItem *interpretationsBranchItem = new QTreeWidgetItem(projectRootItem);
    interpretationsBranchItem->setText(0, "Interpretations");
    for (Cpt *cpt : m_project->cpts()) {
        if(cpt->soilProfile()){
            QTreeWidgetItem *spItem = new QTreeWidgetItem(interpretationsBranchItem);
            spItem->setText(0, cpt->name());            
            spItem->setData(0, Qt::UserRole, QVariant::fromValue(cpt->soilProfile()));
        }
    }

    projectRootItem->setExpanded(true);
    cptsBranchItem->setExpanded(true);
    interpretationsBranchItem->setExpanded(true);

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
    QTreeWidgetItem *item = itemAt(pos);
    if (!item) {
        return;
    }

    QVariant data = item->data(0, Qt::UserRole);
    if (!data.canConvert<Cpt*>()) {
        return; // Only show menu for actual CPT items
    }

    Cpt *clickedCpt = data.value<Cpt*>();
    if (!clickedCpt) {
        return;
    }

    QMenu menu(this);
    QAction *getInterpretationAction = menu.addAction("Get Interpretation");
    QAction *removeAction = menu.addAction("Remove");

    QPoint globalPos = mapToGlobal(pos);
    QAction *selectedAction = menu.exec(globalPos);

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
