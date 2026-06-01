#ifndef PROJECTTREEVIEW_H
#define PROJECTTREEVIEW_H

#include <QTreeWidget>

class Project;
class Cpt;
class VoxelModel;

class ProjectTreeView : public QTreeWidget
{
    Q_OBJECT
public:
    explicit ProjectTreeView(QWidget *parent = nullptr);
    void setProject(Project *project);


private:
    Project* m_project;

signals:
    void cptSelected(Cpt *cpt);
    void voxelModelSelected(VoxelModel *voxelModel);
    void getCptInterpretationSelected(Cpt *cpt);

private slots:
    // Slot to handle the item click
    void handleItemClicked(QTreeWidgetItem *item, int column);
    void showContextMenu(const QPoint &pos);

};

#endif // PROJECTTREEVIEW_H
