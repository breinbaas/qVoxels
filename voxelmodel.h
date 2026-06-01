#ifndef VOXELMODEL_H
#define VOXELMODEL_H

#include <QObject>

class VoxelModel
{
public:
    VoxelModel();

    QString filePath() const { return m_filePath; }
    void setFilePath(QString filePath){m_filePath=filePath;}

    QString name() const;

private:
    QString m_filePath;


};

#endif // VOXELMODEL_H
