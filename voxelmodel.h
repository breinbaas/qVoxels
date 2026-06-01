#ifndef VOXELMODEL_H
#define VOXELMODEL_H

#include <QObject>

class VoxelModel
{
public:
    VoxelModel();

    // getters
    QString filePath() const { return m_filePath; }
    QString name() const;

    // setters
    void setFilePath(QString filePath){m_filePath=filePath;}

private:
    QString m_filePath;


};

#endif // VOXELMODEL_H
