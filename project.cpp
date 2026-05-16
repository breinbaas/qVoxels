// project.cpp
#include "project.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QDirIterator>


Project::Project(QObject *parent)
    : QObject{parent}
    , m_isDirty{false}
{}

Project::~Project()
{
    // QObjects automatically delete their children.
    // Since Cpt objects are parented to this Project,
    // they will be deleted when the Project is deleted.
}

Project *Project::fromPath(const QString &projectPath, QObject *parent)
{
    QDir projectDir(projectPath);

    // 1. Check if the path exists and is a directory
    if (!projectDir.exists() || !projectDir.isReadable()) {
        qWarning() << "Project path does not exist or is not readable:" << projectPath;
        return nullptr;
    }

    // 2. Basic validation: Check for expected subdirectories to confirm it's a qVoxels project
    // This is a simple check; more robust validation might involve a project file.
    QStringList expectedSubDirs = {"cpts", "boreholes", "interpretations", "maps", "models"};
    bool isValidProjectDir = false;
    for (const QString& subDir : expectedSubDirs) {
        if (projectDir.exists(subDir) && projectDir.cd(subDir)) {
            isValidProjectDir = true;
            projectDir.cdUp(); // Go back to the project root
            break; // Found at least one expected subdirectory
        }
    }

    if (!isValidProjectDir) {
        qWarning() << "Selected directory does not appear to be a qVoxels project:" << projectPath;
        return nullptr;
    }

    // 3. Create a new Project object
    Project *project = new Project(parent);
    project->setPath(projectPath);
    project->setName(QFileInfo(projectPath).fileName()); // Get the directory name as the project name
    project->setDirty(false); // An opened project is not dirty initially

    project->loadExistingCpts();

    return project;
}

void Project::setDirty(bool dirty)
{
    if (m_isDirty != dirty) {
        m_isDirty = dirty;
        emit dirtyChanged(m_isDirty);
    }
}

bool Project::isDirty() const
{
    return m_isDirty;
}

QString Project::name() const
{
    return m_name;
}

void Project::setName(const QString& name)
{
    if (m_name != name) {
        m_name = name;
    }
}

QString Project::path() const
{
    return m_path;
}

void Project::setPath(const QString& path)
{
    if (m_path != path) {
        m_path = path;
    }
}

void Project::addCpt(Cpt *cpt)
{
    if (cpt && !m_cpts.contains(cpt)) {
        m_cpts.append(cpt);
        // If the CPT object's parent is not already 'this', set it.
        // This ensures proper memory management (Cpt will be deleted when Project is).
        if (cpt->parent() != this) {
            cpt->setParent(this);
        }
        setDirty(true); // Adding a CPT makes the project dirty
    }
}

QPair<bool, QString> Project::importCptFile(const QString &sourceFilePath, bool copyFileToProjectDir)
{
    QFileInfo fileInfo(sourceFilePath);
    QString fileName = fileInfo.fileName();
    QString cptsDirPath = QDir(m_path).filePath("cpts");
    QString destinationPath = QDir(cptsDirPath).filePath(fileName);

    // Attempt to parse the CPT file
    Cpt *newCpt = Cpt::fromGef(sourceFilePath, this); // Parent Cpt to this project

    if (!newCpt) {
        return {false, QString("Failed to parse GEF file: %1").arg(fileName)};
    }

    // If parsing is successful, handle file copying if requested
    if (copyFileToProjectDir) {
        // Check if the file already exists at the destination.
        // For import, we typically overwrite or ask the user.
        // Here, we assume the calling UI (MainWindow) handles the overwrite prompt.
        // If it reaches here and copy is requested, we attempt to copy.
        // QFile::copy will overwrite if the destination exists and is writable.
        if (!QFile::copy(sourceFilePath, destinationPath)) {
            // If copy fails, delete the Cpt object created by fromGef
            delete newCpt;
            return {false, QString("Failed to copy file to project folder: %1").arg(fileName)};
        }
    }

    // Add Cpt object to the project's list
    addCpt(newCpt); // This also sets the project dirty

    return {true, QString("Successfully imported: %1").arg(fileName)};
}

void Project::loadExistingCpts()
{
    QString cptsDirPath = QDir(m_path).filePath("cpts");
    QDir cptsDir(cptsDirPath);

    if (!cptsDir.exists()) {
        qWarning() << "CPTs directory does not exist for project:" << cptsDirPath;
        return;
    }

    QDirIterator it(cptsDirPath, QStringList() << "*.gef", QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        // Call importCptFile, but do NOT copy the file as it's already in the project directory.
        // The 'false' for copyFileToProjectDir is crucial here.
        QPair<bool, QString> result = importCptFile(filePath, false);
        if (!result.first) {
            qWarning() << "Failed to load existing CPT from project directory:" << filePath << "-" << result.second;
        }
    }
}