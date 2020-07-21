QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Developer mode
DEFINES += DEVELMODE

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cio.cpp \
    data.cpp \
    dataset.cpp \
    dialogenterlabel.cpp \
    dlinereader.cpp \
    dterminal.cpp \
    helper.cpp \
    helpwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    parsetextmatrix.cpp \
    render/NPlot/GfxLib.cpp \
    render/NPlot/NPlotWidgetBase.cpp \
    render/NPlot/NPlotWidgetLabel.cpp \
    render/NPlot/NPlotWidgetScope.cpp \
    render/NPlot/NPlotWidgetScopeExt.cpp \
    render/NPlot/Scope.cpp \
    render/NPlot/ScopeBase.cpp \
    render/NPlot/ScopeType.cpp \
    render/SSView/SSViewAbstract.cpp \
    render/SSView/SSViewLabel.cpp \
    render/SSView/SSViewScope.cpp \
    render/guihelper.cpp

HEADERS += \
    cio.h \
    data.h \
    dataset.h \
    dialogenterlabel.h \
    dlinereader.h \
    dterminal.h \
    helper.h \
    helpwindow.h \
    mainwindow.h \
    parsetextmatrix.h \
    render/NPlot/GfxLib.h \
    render/NPlot/NPlotWidgetBase.h \
    render/NPlot/NPlotWidgetLabel.h \
    render/NPlot/NPlotWidgetScope.h \
    render/NPlot/NPlotWidgetScopeExt.h \
    render/NPlot/SDL_gfxPrimitives_font.h \
    render/NPlot/Scope.h \
    render/NPlot/ScopeBase.h \
    render/NPlot/ScopeType.h \
    render/SSView/SSViewAbstract.h \
    render/SSView/SSViewLabel.h \
    render/SSView/SSViewScope.h \
    render/guihelper.h

FORMS += \
    dialogenterlabel.ui \
    helpwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
   resources.qrc


# icon
win32: RC_FILE = labeller.rc


#RC_ICONS = sine512.ico


