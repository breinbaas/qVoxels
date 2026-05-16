#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog> // For selecting directory
#include <QInputDialog> // For getting project name
#include <QDir>         // For directory operations
#include <QMessageBox>  // For error/info messages
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;

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

    // Emit a signal or call a method to update the UI (e.g., enable/disable actions)
    //updateProjectActions();
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

    QMessageBox::information(this, tr("Project Opened"),
                             tr("Project '%1' opened successfully from:\n%2")
                                 .arg(m_currentProject->name(), m_currentProject->path()));

    // Emit a signal or call a method to update the UI (e.g., enable/disable actions)
    // updateProjectActions();
}


void MainWindow::on_actionCPTs_triggered()
{

}

