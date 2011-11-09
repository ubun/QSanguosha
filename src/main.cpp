/**************************************************************************************************
**QSANGUOSHA/OMEGASLASH CREATIVESUIT SOURCE CODE BEGIN
**
**AUTHOR: MOLIGALOO (SINCE 45.20 AS 2010.6.20) (moligaloo@gmail.com)
**
**CONTRIBUTORS: (UNTIL 62.06 AS 2011.11.6)
**HYPERCROSS (SINCE 55.3 AS 2011.4.3)
**DONLE (SINCE 56.8 AS 2011.5.8)
**UBUN (SINCE 57.26 AS 2011.6.26) (ubun@foxmail.com)
**WILLIAM8915 (SINCE 61.16 AS 2011.10.16)
**IBICDLCOD (MODDER OF OSCS)(SINCE 60.25 AS 2011.9.25) (micheallsy@gmail.com)
**
**QSANGUOSHA, A FREE ONLINE VERSION OF FAMOUS BOARD GAME SANGUOSHA
**Copyright (C) 2010,2011 Moligaloo
**ATTENTION:
**This program is free software: you can redistribute it and/or modify it under the terms of GNU
**Public License as published by the Free Software Foundation, either version 3 of the License, or
**(at your option) any later version.
**
**This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
**even the implied warranty of MERCHANTABLITY of FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**General Public License for more details.
**
**You should have received a copy of the GNU General Public License along with the source code.
**(gpl-3.0.txt at root direction) If not, see <http://www.gnu.org/licenses/>.
**************************************************************************************************/

#include <QtGui/QApplication>

#include <QCoreApplication>
#include <QTranslator>
#include <QDir>
#include <cstring>
#include <QDateTime>

#include <QFile>

#include "mainwindow.h"
#include "settings.h"
#include "banpairdialog.h"
#include "server.h"

#include "engine.h"

#ifdef AUDIO_SUPPORT
#ifdef  Q_OS_WIN32
    #include "irrKlang.h"
    irrklang::ISoundEngine *SoundEngine;
#else
    #include <phonon/MediaObject>
    #include <phonon/AudioOutput>
    Phonon::MediaObject *SoundEngine;
    Phonon::AudioOutput *SoundOutput;
#endif
#endif

int main(int argc, char *argv[])
{
    QString dir_name = QDir::current().dirName();
    if(dir_name == "release" || dir_name == "debug")
        QDir::setCurrent("..");

    if(argc > 1 && strcmp(argv[1], "-server") == 0)
        new QCoreApplication(argc, argv);
    else
        new QApplication(argc, argv);

    // initialize random seed for later use
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    QTranslator qt_translator, translator;
    qt_translator.load("qt_zh_CN.qm");

#ifdef OSCS
    translator.load("sanguoshaoe.qm");
#else
    translator.load("sanguosha.qm");
#endif

    qApp->installTranslator(&qt_translator);
    qApp->installTranslator(&translator);

    Config.init();
    Sanguosha = new Engine;
    BanPair::loadBanPairs();

    if(qApp->arguments().contains("-server")){
        Server *server = new Server(qApp);
        printf("Server is starting on port %u\n", Config.ServerPort);

        if(server->listen())
            printf("Starting successfully\n");
        else
            printf("Starting failed!\n");

        return qApp->exec();
    }

#ifdef AUDIO_SUPPORT

#ifdef  Q_OS_WIN32
    SoundEngine = irrklang::createIrrKlangDevice();
    if(SoundEngine)
        SoundEngine->setSoundVolume(Config.Volume);
#else
    SoundEngine = new Phonon::MediaObject(qApp);
    SoundOutput = new Phonon::AudioOutput(Phonon::GameCategory, qApp);
    Phonon::createPath(SoundEngine, SoundOutput);
#endif

#endif

    MainWindow *main_window = new MainWindow;

    Sanguosha->setParent(main_window);
    main_window->show();

    return qApp->exec();
}
