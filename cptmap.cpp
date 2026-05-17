#include "cptmap.h"

#include <QQuickWidget>
#include <QQmlContext>
#include <QVBoxLayout>
#include <QVariantMap>

#include "project.h" // Include your project header to parse m_cpts
#include "cptmapmanager.h"

CptMap::CptMap(QWidget *parent)
    : QFrame(parent),
    m_quickWidget(new QQuickWidget(this)),
    m_mapManager(new CptMapManager(this))
{
    initUi();
}

CptMap::~CptMap() = default;

void CptMap::initUi() {
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_quickWidget);

    // Context variable remains named "cptManager" so you don't have to change your QML file
    QQmlContext *context = m_quickWidget->rootContext();
    context->setContextProperty("cptManager", m_mapManager);

    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSource(QUrl(QStringLiteral("qrc:/map.qml")));

    connect(m_mapManager, &CptMapManager::qmlAreaSelected, this, &CptMap::areaSelected);
    connect(m_mapManager, &CptMapManager::cptListChanged, this, &CptMap::cptListChanged);
}

void CptMap::setProject(Project *project) {
    m_mapManager->refreshFromProject(project);
}

