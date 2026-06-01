#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QDir>
#include <QMessageBox>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_currentProject(nullptr)
{
    ui->setupUi(this);
    setWindowTitle(tr("qVoxels - No Project"));

    // add map widget
    m_mapWidget = new CptMap(this);
    setupMap();

    // add treeview
    m_projectTreeView = new ProjectTreeView(this);
    setupTreeView();

    // add cpt chart
    m_cptChart = new CptChartWidget(this);
    ui->interpretationLayout->addWidget(m_cptChart);

    // add glb viewer
    m_glbViewer = new GlbViewerWidget(this);
    ui->glbViewerLayout->addWidget(m_glbViewer);

    // connect some signals and slots
    connect(m_projectTreeView, &ProjectTreeView::cptSelected,
            this, &MainWindow::onCptSelected);

    connect(m_projectTreeView, &ProjectTreeView::voxelModelSelected,
            this, &MainWindow::onVoxelModelSelected);

    connect(m_projectTreeView, &ProjectTreeView::getCptInterpretationSelected,
            this, &MainWindow::onCptInterpretationSelected);

}

MainWindow::~MainWindow()
{
    m_currentProject = nullptr;
    delete ui;    
}

void MainWindow::setupMap(){
    QLayout *frameLayout = ui->frmMain->layout();
    if (!frameLayout) {
        frameLayout = new QVBoxLayout(ui->frmMain);
        frameLayout->setContentsMargins(0, 0, 0, 0);
    }

    ui->frmMain->layout()->addWidget(m_mapWidget);
    connect(m_mapWidget, &CptMap::areaSelected, this, [](double minLat, double minLng, double maxLat, double maxLng) {
        qDebug() << "Selected Bound Area coordinates passed to main logic:"
                 << "\nMin Lat/Lng:" << minLat << "," << minLng
                 << "\nMax Lat/Lng:" << maxLat << "," << maxLng;
    });
}

void MainWindow::setupTreeView(){
    ui->verticalLayout->addWidget(m_projectTreeView);
    m_projectTreeView->setColumnCount(1);
    m_projectTreeView->setHeaderLabels(QStringList() << "Project");
    m_projectTreeView->setHeaderHidden(false);
    m_boreholesBranch= new QTreeWidgetItem(m_projectTreeView);
    m_boreholesBranch->setText(0, "Boreholes");
    m_boreholesBranch->setExpanded(true);
    m_cptsBranch = new QTreeWidgetItem(m_projectTreeView);
    m_cptsBranch->setText(0, "CPTs");
    m_cptsBranch->setExpanded(true);
    m_interpretationsBranch= new QTreeWidgetItem(m_projectTreeView);
    m_interpretationsBranch->setText(0, "Interpretations");
    m_interpretationsBranch->setExpanded(true);
    m_modelsBranch= new QTreeWidgetItem(m_projectTreeView);
    m_modelsBranch->setText(0, "Voxel models");
    m_modelsBranch->setExpanded(true);


    // signal slots
    connect(m_projectTreeView, &ProjectTreeView::cptSelected,
            this, &MainWindow::onCptSelected);
}



void MainWindow::updateMap()
{
    m_mapWidget->setProject(m_currentProject);
}

void MainWindow::updateUI()
{
    qDebug () << "updateUI";
    if (!m_currentProject) {
        return;
    }

    if (m_cptsBranch)
        m_cptsBranch->takeChildren();

    // update treeview
    for(Cpt* cpt : m_currentProject->cpts()) {
        QTreeWidgetItem* child = new QTreeWidgetItem(m_cptsBranch);
        child->setText(0, cpt->name());
        child->setData(0, Qt::UserRole, QVariant::fromValue<Cpt*>(cpt));

        if(cpt->soilProfile()){
            QTreeWidgetItem* child = new QTreeWidgetItem(m_interpretationsBranch);
            child->setText(0, cpt->name());
            child->setData(0, Qt::UserRole, QVariant::fromValue<SoilProfile*>(cpt->soilProfile()));
        }
    }

    for(VoxelModel* vm : m_currentProject->voxelModels()){
        QTreeWidgetItem* child = new QTreeWidgetItem(m_modelsBranch);
        child->setText(0, vm->name());
        child->setData(0, Qt::UserRole, QVariant::fromValue<VoxelModel*>(vm));
    }
}

void MainWindow::updateInterpretation(const QString cptName)
{
    for(Cpt* cpt: m_currentProject->cpts()) {
        if (cpt->name().compare(cptName)==0) {
            m_cptChart->setCpt(cpt);
            break;
        }
    }
}

void MainWindow::updateVoxelModel(const QString &filePath)
{
    m_glbViewer->loadModel(filePath);
}


void MainWindow::on_actionNew_triggered()
{
    QString parentDirPath = QFileDialog::getExistingDirectory(this,
                                                              tr("Select Directory for New Project"),
                                                              QDir::homePath(),
                                                              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (parentDirPath.isEmpty()) {
        return;
    }

    bool ok;
    QString projectName = QInputDialog::getText(this,
                                                tr("New Project Name"),
                                                tr("Enter project name:"),
                                                QLineEdit::Normal,
                                                QString(),
                                                &ok);

    if (!ok || projectName.isEmpty()) {
        return;
    }

    projectName.replace(QRegularExpression("[^a-zA-Z0-9_\\-]+"), "_");
    if (projectName.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Project Name"), tr("The project name contains only invalid characters. Please choose a different name."));
        return;
    }

    QDir projectDir(parentDirPath);
    QString fullProjectPath = projectDir.filePath(projectName);

    QDir newProjectBaseDir(fullProjectPath);
    if (!newProjectBaseDir.mkpath(".")) { // Create the base project directory itself
        QMessageBox::critical(this, tr("Error Creating Project"),
                              tr("Failed to create project directory: %1").arg(fullProjectPath));
        return;
    }

    QStringList subDirs = {"cpts", "boreholes", "interpretations", "maps", "models"};
    bool allSubDirsCreated = true;

    for (const QString& subDir : subDirs) {
        if (!newProjectBaseDir.mkpath(subDir)) {
            QMessageBox::warning(this, tr("Warning"),
                                 tr("Failed to create subdirectory: %1/%2").arg(fullProjectPath, subDir));
            allSubDirsCreated = false;
        }
    }

    if (allSubDirsCreated) {
        QMessageBox::information(this, tr("Project Created"),
                                 tr("New project '%1' and its subdirectories created successfully at:\n%2")
                                     .arg(projectName, fullProjectPath));
    } else {
        QMessageBox::warning(this, tr("Project Created with Warnings"),
                             tr("New project '%1' created at:\n%2\nSome subdirectories could not be created.")
                                 .arg(projectName, fullProjectPath));
    }

    if (m_currentProject) {
        delete m_currentProject;
        m_currentProject = nullptr;
    }

    // Create a new Project object and set it as the current project
    m_currentProject = new Project(this);
    m_currentProject->setName(projectName);
    m_currentProject->setPath(fullProjectPath);
    m_currentProject->setDirty(true);

    setWindowTitle(tr("qVoxels - %1%2").arg(m_currentProject->name(), m_currentProject->isDirty() ? "*" : ""));

    updateMap();
    updateUI();
}
void MainWindow::on_actionOpen_triggered()
{
    QString projectPath = QFileDialog::getExistingDirectory(this,
                                                            tr("Open Project"),
                                                            QDir::homePath(),
                                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (projectPath.isEmpty()) {
        return; // User cancelled
    }

    Project *openedProject = Project::fromPath(projectPath, this);

    if (!openedProject) {
        QMessageBox::warning(this, tr("Error Opening Project"),
                             tr("The selected directory is not a valid qVoxels project or could not be opened."));
        return;
    }

    // If an existing project is open, delete it before replacing
    if (m_currentProject) {
        delete m_currentProject;
    }

    m_currentProject = openedProject;

    setWindowTitle(tr("qVoxels - %1%2").arg(m_currentProject->name(), m_currentProject->isDirty() ? "*" : ""));    

    updateMap();
    updateUI();
}


void MainWindow::on_actionCPTs_triggered()
{
    if (!m_currentProject) {
        QMessageBox::warning(this, tr("No Project Open"), tr("Please open or create a project first."));
        return;
    }

    QString cptsDirPath = QDir(m_currentProject->path()).filePath("cpts");
    QDir cptsDir(cptsDirPath);

    QStringList filePaths = QFileDialog::getOpenFileNames(this,
                                                          tr("Select CPT Files to Import"),
                                                          QDir::homePath(),
                                                          tr("CPT Files (*.gef);;All Files (*)"));

    if (filePaths.isEmpty()) {
        return;
    }

    QStringList successfulImports;
    QStringList failedImports;
    bool projectWasDirty = m_currentProject->isDirty();

    for (const QString &filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();
        QString destinationPath = cptsDir.filePath(fileName);

        bool proceedWithImport = true;
        if (QFile::exists(destinationPath)) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("File Exists"),
                                          tr("A file named '%1' already exists in the project's CPT folder. Overwrite?").arg(fileName),
                                          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            if (reply == QMessageBox::Cancel) {
                failedImports.append(QString("%1 (Import cancelled by user)").arg(fileName));
                proceedWithImport = false;
            }
            if (reply == QMessageBox::No) {
                failedImports.append(QString("%1 (Skipped - file already exists)").arg(fileName));
                proceedWithImport = false;
            }            
        }

        if (proceedWithImport) {
            QPair<bool, QString> result = m_currentProject->importCptFile(filePath, true);
            if (result.first) {
                successfulImports.append(fileName);
            } else {
                failedImports.append(QString("%1 (%2)").arg(fileName, result.second));
            }
        }
    }

    if (m_currentProject->isDirty() && !projectWasDirty) {
        setWindowTitle(tr("qVoxels - %1%2").arg(m_currentProject->name(), "*"));
    }

    QString summaryMessage;
    QTextStream stream(&summaryMessage);
    stream << tr("CPT Import Results:\n\n");

    if (!successfulImports.isEmpty()) {
        stream << tr("Successfully imported (%1):\n").arg(successfulImports.size());
        for (const QString &s : successfulImports) {
            stream << "  - " << s << "\n";
        }
        stream << "\n";
    }

    if (!failedImports.isEmpty()) {
        stream << tr("Failed to import (%1):\n").arg(failedImports.size());
        for (const QString &f : failedImports) {
            stream << "  - " << f << "\n";
        }
        stream << "\n";
    }

    if (successfulImports.isEmpty() && failedImports.isEmpty()) {
        stream << tr("No CPT files were selected or processed.");
    }

    QMessageBox::information(this, tr("CPT Import Summary"), summaryMessage);

    updateMap();
    updateUI();
}

void MainWindow::onCptSelected(Cpt *cpt){

    updateInterpretation(cpt->name());
}

void MainWindow::onVoxelModelSelected(VoxelModel *voxelModel)
{
    qDebug () << "Voxel model selected:" << voxelModel->name();
    updateVoxelModel(voxelModel->filePath());
}

void MainWindow::onCptInterpretationSelected(Cpt *cpt)
{
    if(cpt){
        m_currentProject->getApiCptInterpretation(cpt);
    }
}

void MainWindow::on_actionCpt_Interpretations_triggered()
{
    // TODO -> progressbar
    if(m_currentProject){
        for(Cpt* cpt:m_currentProject->cpts()){
            m_currentProject->getApiCptInterpretation(cpt);
        }
    }
}


void MainWindow::on_actionVoxel_Model_triggered()
{
    if(m_currentProject){
        m_currentProject->getApiVoxelModel();
    }
}

