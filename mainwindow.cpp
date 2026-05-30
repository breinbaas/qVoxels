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
        frameLayout->setContentsMargins(0, 0, 0, 0); // Make the map fit completely flush
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

    // signal slots
    connect(m_projectTreeView, &QTreeWidget::itemClicked,
            this, &MainWindow::onTreeViewItemClicked);
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
        child->setData(0, Qt::UserRole, cpt->filePath());
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

    // Construct the full path for the new project directory
    QDir projectDir(parentDirPath);
    QString fullProjectPath = projectDir.filePath(projectName);

    // Create the main project directory and its subdirectories
    QDir newProjectBaseDir(fullProjectPath);
    if (!newProjectBaseDir.mkpath(".")) { // Create the base project directory itself
        QMessageBox::critical(this, tr("Error Creating Project"),
                              tr("Failed to create project directory: %1").arg(fullProjectPath));
        return;
    }

    // List of subdirectories to create
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

    // Delete the old project if one exists before creating a new one
    if (m_currentProject) {
        delete m_currentProject;
        m_currentProject = nullptr;
    }

    // Create a new Project object and set it as the current project
    m_currentProject = new Project(this);
    m_currentProject->setName(projectName);
    m_currentProject->setPath(fullProjectPath);
    m_currentProject->setDirty(true); // New project is dirty by default

    // Update the window title to reflect the new project
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

    m_currentProject = openedProject; // Set the newly opened project as current

    // Update the window title to reflect the opened project
    setWindowTitle(tr("qVoxels - %1%2").arg(m_currentProject->name(), m_currentProject->isDirty() ? "*" : ""));

    // QMessageBox::information(this, tr("Project Opened"),
    //                          tr("Project '%1' opened successfully from:\n%2")
    //                              .arg(m_currentProject->name(), m_currentProject->path()));

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
    QDir cptsDir(cptsDirPath); // Create a QDir object for the CPTs directory

    QStringList filePaths = QFileDialog::getOpenFileNames(this,
                                                          tr("Select CPT Files to Import"),
                                                          QDir::homePath(),
                                                          tr("CPT Files (*.gef);;All Files (*)"));

    if (filePaths.isEmpty()) {
        return; // User cancelled file selection
    }

    QStringList successfulImports;
    QStringList failedImports;
    bool projectWasDirty = m_currentProject->isDirty(); // Check dirty state before imports

    for (const QString &filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();
        QString destinationPath = cptsDir.filePath(fileName); // Use the QDir object

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
            // If Yes, proceed with import (importCptFile will handle overwriting)
        }

        if (proceedWithImport) {
            QPair<bool, QString> result = m_currentProject->importCptFile(filePath, true); // Pass true to copy the file
            if (result.first) {
                successfulImports.append(fileName);
            } else {
                failedImports.append(QString("%1 (%2)").arg(fileName, result.second));
            }
        }
    }

    // Update the window title if the project's dirty state changed due to imports
    if (m_currentProject->isDirty() && !projectWasDirty) {
        setWindowTitle(tr("qVoxels - %1%2").arg(m_currentProject->name(), "*"));
    }

    // Display summary of import results
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

void MainWindow::onTreeViewItemClicked(QTreeWidgetItem *item, int column)
{
    if (!item) return;

    if (item->parent() == m_cptsBranch) {
        QString itemName = item->text(0);
        // QString filePath = item->data(0, Qt::UserRole).toString();
        // qDebug() << "Selected CPT Name:" << itemName << " file path: "<<filePath;
        updateInterpretation(itemName);
    }
}

