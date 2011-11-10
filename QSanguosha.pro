# -------------------------------------------------
# Project created by QtCreator 2010-06-13T04:26:52
# -------------------------------------------------
TARGET = OmegaSlash #QSanguosha, OmegaSlash
QT += network sql
TEMPLATE = app
CONFIG += warn_on audio joystick qaxcontainer

macx {
    CONFIG -= joystick # in Mac, we do not support joystick currently
}

SOURCES += src/main.cpp \
	src/client/aux-skills.cpp \
	src/client/client.cpp \
	src/client/clientplayer.cpp \
	src/client/clientstruct.cpp \
	src/core/card.cpp \
	src/core/engine.cpp \
	src/core/general.cpp \
	src/core/lua-wrapper.cpp \
	src/core/player.cpp \
	src/core/settings.cpp \
	src/core/skill.cpp \
	src/dialog/banpairdialog.cpp \
	src/dialog/cardeditor.cpp \
	src/dialog/cardoverview.cpp \
	src/dialog/choosegeneraldialog.cpp \
	src/dialog/configdialog.cpp \
	src/dialog/connectiondialog.cpp \
	src/dialog/distanceviewdialog.cpp \
	src/dialog/generaloverview.cpp \
	src/dialog/generalselector.cpp \
	src/dialog/mainwindow.cpp \
	src/dialog/packagingeditor.cpp \
	src/dialog/playercarddialog.cpp \
	src/dialog/roleassigndialog.cpp \
	src/dialog/scenario-overview.cpp \
        src/dialog/halldialog.cpp \
	src/package/firepackage.cpp \
	src/package/god.cpp \
	src/package/joypackage.cpp \
	src/package/maneuvering.cpp \
	src/package/mountainpackage.cpp \
	src/package/nostalgia.cpp \
	src/package/sp-package.cpp \
	src/package/gold-seinto-viva.cpp \
	src/package/standard-cards.cpp \
	src/package/standard-generals.cpp \
	src/package/standard-skillcards.cpp \
	src/package/standard.cpp \
	src/package/thicket.cpp \
	src/package/wind.cpp \
	src/package/wisdompackage.cpp \
	src/package/yitianpackage.cpp \
	src/package/yjcm-package.cpp \
	src/scenario/boss-mode-scenario.cpp \
	src/scenario/challengemode.cpp \
	src/scenario/couple-scenario.cpp \
	src/scenario/fancheng-scenario.cpp \
	src/scenario/guandu-scenario.cpp \
	src/scenario/hongyan-scenario.cpp \
	src/scenario/legend-mode-scenario.cpp \
	src/scenario/scenario.cpp \
	src/scenario/scenerule.cpp \
	src/scenario/zombie-mode-scenario.cpp \
	src/server/ai.cpp \
	src/server/contestdb.cpp \
	src/server/gamerule.cpp \
	src/server/room.cpp \
	src/server/roomthread.cpp \
	src/server/roomthread1v1.cpp \
	src/server/roomthread3v3.cpp \
	src/server/server.cpp \
	src/server/serverplayer.cpp \
	src/ui/button.cpp \
	src/ui/cardcontainer.cpp \
	src/ui/carditem.cpp \
	src/ui/clientlogbox.cpp \
	src/ui/dashboard.cpp \
	src/ui/indicatoritem.cpp \
	src/ui/photo.cpp \
	src/ui/pixmap.cpp \
	src/ui/rolecombobox.cpp \
	src/ui/roomscene.cpp \
	src/ui/startscene.cpp \
	src/ui/window.cpp \
	src/util/detector.cpp \
	src/util/nativesocket.cpp \
	src/util/recorder.cpp \
	swig/sanguosha_wrap.cxx \
    src/package/OETanA.cpp \
    src/package/OETanB.cpp

HEADERS += src/client/aux-skills.h \
	src/client/client.h \
	src/client/clientplayer.h \
	src/client/clientstruct.h \
	src/core/card.h \
	src/core/engine.h \
	src/core/general.h \
	src/core/lua-wrapper.h \
	src/core/player.h \
	src/core/settings.h \
	src/core/skill.h \
	src/dialog/banpairdialog.h \
	src/dialog/cardeditor.h \
	src/dialog/cardoverview.h \
	src/dialog/choosegeneraldialog.h \
	src/dialog/configdialog.h \
	src/dialog/connectiondialog.h \
	src/dialog/distanceviewdialog.h \
	src/dialog/generaloverview.h \
	src/dialog/generalselector.h \
	src/dialog/halldialog.h \
	src/dialog/mainwindow.h \
	src/dialog/packagingeditor.h \
	src/dialog/playercarddialog.h \
	src/dialog/roleassigndialog.h \ 
	src/dialog/scenario-overview.h \
	src/package/firepackage.h \
	src/package/god.h \
	src/package/joypackage.h \
	src/package/maneuvering.h \
	src/package/mountainpackage.h \
	src/package/nostalgia.h \
	src/package/package.h \
	src/package/gold-seinto-viva.h \
	src/package/sp-package.h \
	src/package/standard-equips.h \
	src/package/standard-skillcards.h \
	src/package/standard.h \
	src/package/thicket.h \
	src/package/wind.h \
	src/package/wisdompackage.h \
	src/package/yitianpackage.h \
	src/package/yjcm-package.h \
	src/scenario/boss-mode-scenario.h \
	src/scenario/challengemode.h \
	src/scenario/couple-scenario.h \
	src/scenario/fancheng-scenario.h \
	src/scenario/guandu-scenario.h \
	src/scenario/hongyan-scenario.h \
	src/scenario/legend-mode-scenario.h \
	src/scenario/scenario.h \
	src/scenario/scenerule.h \
	src/scenario/zombie-mode-scenario.h \
	src/server/ai.h \
	src/server/contestdb.h \
	src/server/gamerule.h \
	src/server/room.h \
	src/server/roomthread.h \
	src/server/roomthread1v1.h \
	src/server/roomthread3v3.h \
	src/server/server.h \
	src/server/serverplayer.h \
	src/server/structs.h \
	src/ui/button.h \
	src/ui/cardcontainer.h \
	src/ui/carditem.h \
	src/ui/clientlogbox.h \
	src/ui/dashboard.h \
	src/ui/indicatoritem.h \
	src/ui/photo.h \
	src/ui/pixmap.h \
	src/ui/rolecombobox.h \
	src/ui/roomscene.h \
	src/ui/startscene.h \
	src/ui/window.h \
	src/util/detector.h \
	src/util/nativesocket.h \
	src/util/recorder.h \
	src/util/socket.h \
    src/package/OETanA.h \
    src/package/OETanB.h \
    OSCS.h
	
FORMS += src/dialog/cardoverview.ui \
	src/dialog/configdialog.ui \
	src/dialog/connectiondialog.ui \
	src/dialog/generaloverview.ui \
        src/dialog/generaloverviewoe.ui \
	src/dialog/mainwindow.ui 
	
INCLUDEPATH += include/lua
INCLUDEPATH += include
INCLUDEPATH += src/client
INCLUDEPATH += src/core
INCLUDEPATH += src/dialog
INCLUDEPATH += src/package
INCLUDEPATH += src/scenario
INCLUDEPATH += src/server
INCLUDEPATH += src/ui
INCLUDEPATH += src/util

win32{
    RC_FILE += resource/iconoe.rc #for OMEGASLASH. When change this, delete debug/icon_res.o
    LIBS += -L. -llua -lm
}

unix:!macx {
    LIBS += -lm -llua
}

macx {
    LIBS += -L. -lm -llua5.1
}

CONFIG(audio){
    DEFINES += AUDIO_SUPPORT
    INCLUDEPATH += include/irrKlang
    win32: LIBS += irrKlang.lib
    unix: QT += phonon
}

CONFIG(joystick){
    DEFINES += JOYSTICK_SUPPORT
    HEADERS += src/ui/joystick.h
    SOURCES += src/ui/joystick.cpp
    win32: LIBS += -lplibjs -lplibul -lwinmm
    unix: LIBS += -lplibjs -lplibul
}

TRANSLATIONS += sanguosha.ts
TRANSLATIONS += sanguoshaoe.ts # for OMEGASLASH

OTHER_FILES += \
    lua/sgs_ex.lua \
    lua/ai/yjcm-skill-ai.lua \
    lua/ai/yjcm-ai.lua \
    lua/ai/yitian-ai.lua \
    lua/ai/wisdom-ai.lua \
    lua/ai/wind-ai.lua \
    lua/ai/value_config.lua \
    lua/ai/thicket-skill-ai.lua \
    lua/ai/thicket-ai.lua \
    lua/ai/state-ai.lua \
    lua/ai/standard-skill-ai.lua \
    lua/ai/standard-ai.lua \
    lua/ai/sp-ai.lua \
    lua/ai/smart-ai.lua \
    lua/ai/sequence-ai.lua \
    lua/ai/playrule-ai.lua \
    lua/ai/nostalgia-ai.lua \
    lua/ai/new-ai.lua \
    lua/ai/mountain-ai.lua \
    lua/ai/joy-ai.lua \
    lua/ai/intention-filter.lua \
    lua/ai/intention-ai.lua \
    lua/ai/hulaoguan-ai.lua \
    lua/ai/guanxing-ai.lua \
    lua/ai/god-ai.lua \
    lua/ai/general_config.lua \
    lua/ai/fire-skill-ai.lua \
    lua/ai/fire-ai.lua \
    lua/ai/fancheng-ai.lua \
    lua/ai/choice-ai.lua \
    lua/ai/chat-ai.lua \
    lua/lib/sqlite3.lua \
    lua/lib/socket.lua \
    lua/lib/mime.lua \
    lua/lib/middleclass.lua \
    lua/lib/ltn12.lua \
    lua/lib/libluasqlite3-loader.lua \
    lua/lib/expand.lua \
    lua/lib/socket/url.lua \
    lua/lib/socket/tp.lua \
    lua/lib/socket/smtp.lua \
    lua/lib/socket/http.lua \
    lua/lib/socket/ftp.lua \
    lua/tools/send-result.lua \
    lua/tools/group.lua \
    lua/tools/genpro.lua
