// project.cpp
#include "project.h"
#include <QDir>     // For directory checks
#include <QFileInfo> // For getting directory name

Project::Project(QObject *parent)
    : QObject{parent}
    , m_isDirty{false}
{}

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

    // TODO: In a real application, you would load project-specific data here (e.g., CPTs, boreholes list)

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
        // Optionally emit a signal like nameChanged(QString) if needed
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
        // Optionally emit a signal like pathChanged(QString) if needed
    }
}