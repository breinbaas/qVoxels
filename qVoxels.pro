QT += widgets quick quickwidgets location positioning charts quick3d
CONFIG += c++17
RESOURCES += resources.qrc
INCLUDEPATH += C:/msys64/clang64/include
LIBS += -LC:/msys64/clang64/lib -lproj

# LIBS += -lproj

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    api.cpp \
    borehole.cpp \
    cpt.cpp \
    cptchartwidget.cpp \
    cptmap.cpp \
    cptmapmanager.cpp \
    glbviewerwidget.cpp \
    helpers.cpp \
    main.cpp \
    mainwindow.cpp \
    project.cpp \
    projecttreeview.cpp \
    soilprofile.cpp \
    voxelmodel.cpp

HEADERS += \
    api.h \
    borehole.h \
    cpt.h \
    cptchartwidget.h \
    cptmap.h \
    cptmapmanager.h \
    glbviewerwidget.h \
    helpers.h \
    mainwindow.h \
    project.h \
    projecttreeview.h \
    soilcolorpalette.h \
    soillayer.h \
    soilprofile.h \
    voxelmodel.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    CODING.md \
    glbviewer.qml \
    map.qml

RESOURCES += \
    resources.qrc
