#ifndef PROJECTTREEVIEW_H
#define PROJECTTREEVIEW_H

#include <QTreeWidget>

class Project;
class Cpt;

class ProjectTreeView : public QTreeWidget
{
    Q_OBJECT
public:
    explicit ProjectTreeView(QWidget *parent = nullptr);
    void setProject(Project *project);


private:
    Project* m_project;

signals:
};

#endif // PROJECTTREEVIEW_H
