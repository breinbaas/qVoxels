QT += widgets quick quickwidgets location positioning

CONFIG += c++17
RESOURCES += resources.qrc
# LIBS += -lproj

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    borehole.cpp \
    cpt.cpp \
    cptmap.cpp \
    cptmapmanager.cpp \
    main.cpp \
    mainwindow.cpp \
    project.cpp

HEADERS += \
    borehole.h \
    cpt.h \
    cptmap.h \
    cptmapmanager.h \
    mainwindow.h \
    project.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    CODING.md \
    map.qml

RESOURCES += \
    resources.qrc
