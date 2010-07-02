# -------------------------------------------------
# Project created by QtCreator 2010-06-13T04:26:52
# -------------------------------------------------
TARGET = QSanguosha
QT += phonon \
    opengl \
    script \
    network
TEMPLATE = app
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/gamescene.cpp \
    src/button.cpp \
    src/settings.cpp \
    src/startscene.cpp \
    src/roomscene.cpp \
    src/photo.cpp \
    src/dashboard.cpp \
    src/card.cpp \
    src/pixmap.cpp \
    src/general.cpp \
    src/server.cpp \
    src/engine.cpp \
    src/servingthread.cpp \
    src/connectiondialog.cpp \
    src/client.cpp \
    src/cardclass.cpp \
    src/carditem.cpp \
    src/roomthread.cpp \
    src/generaloverview.cpp
HEADERS += src/gamescene.h \
    src/mainwindow.h \
    src/button.h \
    src/settings.h \
    src/startscene.h \
    src/roomscene.h \
    src/photo.h \
    src/dashboard.h \
    src/card.h \
    src/pixmap.h \
    src/general.h \
    src/server.h \
    src/engine.h \
    src/servingthread.h \
    src/connectiondialog.h \
    src/client.h \
    src/cardclass.h \
    src/carditem.h \
    src/roomthread.h \
    src/generaloverview.h
FORMS += src/mainwindow.ui \
    src/connectiondialog.ui \
    src/generaloverview.ui
OTHER_FILES += scripts/generals.js \
    scripts/cards.js \
    scripts/init.js
RESOURCES += resource/sanguosha.qrc
RC_FILE += resource/icon.rc
TRANSLATIONS += sanguosha.ts
