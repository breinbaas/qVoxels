// project.cpp
#include "project.h"

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QDirIterator>

#include "soilprofile.h"

Project::Project(QObject *parent)
    : QObject{parent}
    , m_isDirty{false}
{
    // add api
    m_apiService = new Api(this);
    connect(m_apiService, &Api::errorOccurred, this, [](const QString &errorMsg){
        qWarning() << "Network Engine Error:" << errorMsg;
    });
    connect(m_apiService, &Api::interpretationReceived, this, &Project::onApiCptInterpretationReceived);
    connect(m_apiService, &Api::voxelModelReceived, this, &Project::onApiVoxelModelReceived);
}

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
    project->loadExistingModels();

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

    // now load any soilprofiles and connect them to the cpts
    QString soilProfilesDirPath = QDir(m_path).filePath("interpretations");
    QDir soilProfilesDir(soilProfilesDirPath);

    if (!soilProfilesDir.exists()) {
        qWarning() << "Interpretations directory does not exist for project:" << soilProfilesDirPath;
        return;
    }

    QDirIterator itInterpretations(soilProfilesDirPath, QStringList() << "*.json", QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
    while (itInterpretations.hasNext()) {
        QString filePath = itInterpretations.next();
        SoilProfile* sp = SoilProfile::fromJsonFile(filePath);

        if(!sp){
             qWarning() << "Failed to load existing interpretation from project directory:" << filePath;
        }else{
            QString interpretationName = QFileInfo(filePath).baseName();
            for(Cpt* cpt : m_cpts){
                if(cpt->name() == interpretationName){
                    cpt->setSoilProfile(sp);
                    break;
                }
            }
        }
    }
}

void Project::loadExistingModels()
{
    QString modelDirPath = QDir(m_path).filePath("models");
    QDir modelsDir(modelDirPath);

    if (!modelsDir.exists()) {
        qWarning() << "Voxel models directory does not exist for project:" << modelDirPath;
        return;
    }

    QDirIterator it(modelDirPath, QStringList() << "*.glb", QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        VoxelModel* vm = new VoxelModel();
        vm->setFilePath(filePath);
        m_voxelModels.append(vm);
    }
}


/* API INTERACTION */
void Project::getApiCptInterpretation(Cpt *cpt)
{
    if (!cpt) return;
    m_apiService->getInterpretationFromCpt(cpt->name(), cpt->filePath(), 2, 0.5, 6.0);
}

void Project::getApiVoxelModel()
{
    float xmin = 1e9;
    float xmax = -1e9;
    float ymin= 1e9;
    float ymax = -1e9;
    float zmin = 1e9;
    float zmax = -1e9;

    QList<SoilProfile*> soilProfiles;
    for(Cpt* cpt:m_cpts){
        if(cpt->soilProfile()){
            soilProfiles.append(cpt->soilProfile());
            xmin = qMin(xmin, cpt->x());
            xmax = qMax(xmax, cpt->x());
            ymin = qMin(ymin, cpt->y());
            ymax = qMax(ymax, cpt->y());
            zmin = qMin(zmin, cpt->bottom());
            zmax = qMax(zmax, cpt->top());
        }
    }

    if(soilProfiles.isEmpty()){
        qWarning("No soilprofiles found to create voxelmodel from");
        return;
    }

    ApiVoxelModelRequestParameters parameters;
    parameters.xmin = xmin;
    parameters.xmax = xmax;
    parameters.ymin = ymin;
    parameters.ymax = ymax;
    parameters.zmin = zmin;
    parameters.zmax = zmax;
    parameters.dx = 5;
    parameters.dy = 5;
    parameters.dz = 1;
    parameters.anisotropy_ratio = 50;
    parameters.step_size = 0.5;

    QString modelsDirPath = QDir(m_path).filePath("models");
    m_apiService->getVoxelModel(soilProfiles, parameters, modelsDirPath);
}

/* API SLOTS */
void Project::onApiCptInterpretationReceived(QString cptName, SoilProfile *soilProfile)
{
    for(Cpt* cpt: m_cpts)
    {
        if (cpt->name() ==cptName){
            cpt->setSoilProfile(soilProfile);
            QString soilProfileDirPath = QDir(m_path).filePath(QString("interpretations/%1.json").arg(cptName));
            soilProfile->toJsonFile(soilProfileDirPath);
            m_isDirty = true;
            break;
        }
    }
}

void Project::onApiVoxelModelReceived(const QString filePath)
{
    VoxelModel *voxelModel = new VoxelModel();
    voxelModel->setFilePath(filePath);
    m_voxelModels.append(voxelModel);
    m_isDirty = true;
}

