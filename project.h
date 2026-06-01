#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QString> // Include QString for m_name and m_path

#include "cpt.h"
#include "borehole.h"
#include "soilprofile.h"
#include "voxelmodel.h"
#include "api.h"

class Project : public QObject
{
    Q_OBJECT

private:
    QString m_name; // Project name
    QString m_path; // Project base path
    QList<Cpt*> m_cpts;
    QList<Borehole> m_boreholes;
    QList<VoxelModel*> m_voxelModels;

    // the api
    Api *m_apiService = nullptr;


    bool m_isDirty;

    void loadExistingCpts();
    void loadExistingModels();

public:
    explicit Project(QObject *parent = nullptr);

    static Project* fromPath(const QString &projectPath, QObject *parent = nullptr);

    // Getters
    bool isDirty() const;
    QString name() const;
    QString path() const;
    const QList<Cpt*>& cpts() const { return m_cpts; }
    const QList<VoxelModel*> voxelModels() const {return m_voxelModels;}

    // Setters
    void setDirty(bool dirty);
    void setName(const QString& name);
    void setPath(const QString& path);

    void addCpt(Cpt *cpt);
    QPair<bool, QString> importCptFile(const QString &sourceFilePath, bool copyFileToProjectDir = true);

    // API interaction
    void getApiCptInterpretation(Cpt *cpt);
    void getApiVoxelModel();


    ~Project();


public slots:
    void onApiCptInterpretationReceived(QString cptName, SoilProfile *soilProfile);
    void onApiVoxelModelReceived(const QString filePath);

signals:
    void dirtyChanged(bool dirty);
};

#endif // PROJECT_H