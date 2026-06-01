#include "voxelmodel.h"
#include <QFileInfo>

VoxelModel::VoxelModel() {}

QString VoxelModel::name() const
{
    QFileInfo fileInfo(m_filePath);
    return fileInfo.fileName();
}
