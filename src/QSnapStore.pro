#-------------------------------------------------
#
# Project created by QtCreator 2020-09-26T20:07:27
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Set program version
VERSION = 0.0.1
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

include (libraries/md/qmarkdowntextedit.pri)

TARGET = qsnapstore
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        error.cpp \
        helpers/codetohtmlconverter.cpp \
        main.cpp \
        mainwindow.cpp \
        md2html.cpp \
        remotepixmaplabel.cpp \
        request.cpp \
        rungaurd.cpp \
        screenshots.cpp \
        store.cpp \
        utils.cpp \
        widgets/RangeSlider.cpp \
        widgets/ccheckboxitemdelegate.cpp \
        widgets/controlbutton.cpp \
        widgets/cverticallabel.cpp \
        widgets/elidedlabel.cpp \
        widgets/spoiler.cpp \
        widgets/waitingspinnerwidget.cpp \
        libraries/md4c/md4c/md4c.c \
        libraries/md4c/md2html/render_html.c \
        libraries/md4c/md2html/entity.c

HEADERS += \
        error.h \
        helpers/codetohtmlconverter.h \
        mainwindow.h \
        md2html.h \
        remotepixmaplabel.h \
        request.h \
        rungaurd.h \
        screenshots.h \
        store.h \
        utils.h \
        widgets/RangeSlider.h \
        widgets/ccheckboxitemdelegate.h \
        widgets/controlbutton.h \
        widgets/cverticallabel.h \
        widgets/elidedlabel.h \
        widgets/spoiler.h \
        widgets/waitingspinnerwidget.h \
        libraries/md4c/md4c/md4c.h \
        libraries/md4c/md2html/render_html.h \
        libraries/md4c/md2html/entity.h

FORMS += \
        action_ui.ui \
        error.ui \
        mainwindow.ui \
        screenshots.ui \
        track.ui \
        wallthumb.ui


# Default rules for deployment.
isEmpty(PREFIX){
 PREFIX = /usr
}

BINDIR  = $$PREFIX/bin
DATADIR = $$PREFIX/share

target.path = $$BINDIR

icon.files = icons/qsnapstore.png
icon.path = $$DATADIR/icons/hicolor/512x512/apps/

desktop.files = qsnapstore.desktop
desktop.path = $$DATADIR/applications/

INSTALLS += target icon desktop

RESOURCES += \
    icons.qrc \
    qbreeze.qrc \
    resources.qrc

