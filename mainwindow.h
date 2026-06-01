#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "project.h"
#include "cptmap.h"
#include "projecttreeview.h"
#include "cptchartwidget.h"
#include "glbviewerwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;


private slots:
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionCPTs_triggered();
    void onCptSelected(Cpt *cpt);
    void onVoxelModelSelected(VoxelModel *voxelModel);
    void onCptInterpretationSelected(Cpt *cpt);
    //void onApiCptInterpretationReceived(Cpt *cpt, SoilProfile *soilProfile);

    void on_actionCpt_Interpretations_triggered();

    void on_actionVoxel_Model_triggered();

private:
    Ui::MainWindow *ui;
    Project *m_currentProject = nullptr; // Member to hold the current project

    // widgets
    // the map
    CptMap *m_mapWidget = nullptr;
    GlbViewerWidget *m_glbViewer = nullptr;

    // the cpt chart
    CptChartWidget* m_cptChart = nullptr;    
    ProjectTreeView *m_projectTreeView = nullptr;
    QTreeWidgetItem* m_cptsBranch = nullptr;
    QTreeWidgetItem* m_boreholesBranch = nullptr;
    QTreeWidgetItem* m_interpretationsBranch = nullptr;
    QTreeWidgetItem* m_modelsBranch = nullptr;

    void setupMap();
    void updateMap();
    void updateUI();
    void updateInterpretation(const QString cptName);
    void updateVoxelModel(const QString& filePath);
    void setupTreeView();

};
#endif // MAINWINDOW_H