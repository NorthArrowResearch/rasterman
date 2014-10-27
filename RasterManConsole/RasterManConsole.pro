#-------------------------------------------------
#
# Project created by QtCreator 2014-10-25T15:13:18
#
#-------------------------------------------------

QT       += core
QT       += xml

QT       -= gui

VERSION = 6.0.45
TARGET = rasterman
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -stdlib=libc++
QMAKE_CXXFLAGS += -std=c++11
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10 #2 Yosemite

SOURCES += main.cpp \
    rastermanengine.cpp

HEADERS += \
    rastermanengine.h

INCLUDEPATH += $$PWD/../RasterManager
DEPENDPATH += $$PWD/../RasterManager

Libs += -L$$PWD/../RasterManager

win32 {
    ## Windows common build here
    !contains(QMAKE_TARGET.arch, x86_64) {
        message("x86 build")
        LIBS += -L$$PWD/../Libraries/gdalwin32-1.10.1/lib/ -lgdal_i
        INCLUDEPATH += $$PWD/../Libraries/gdalwin32-1.10.1/include
        DEPENDPATH += $$PWD/../Libraries/gdalwin32-1.10.1/include
    } else {
        message("x86_64 build")
        LIBS += -L$$PWD/../Libraries/gdalwin64-1.10.1/lib/ -lgdal_i
        INCLUDEPATH += $$PWD/../Libraries/gdalwin64-1.10.1/include
        DEPENDPATH += $$PWD/../Libraries/gdalwin64-1.10.1/include
    }

    # Compile to a central location
    CONFIG(release, debug|release): DESTDIR = $$OUT_PWD/../../../Deploy/Release32
    else:CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/../../../Deploy/Debug32

    # Tell it where to find compiled RasterManager.dll
    CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../../Deploy/Release32 -lRasterManager
    else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../../Deploy/Debug32 -lRasterManager

}
macx{
    ## OSX common build here
    message("Mac OSX x86_64 build (64bit)")

    # Compile to a central location
    CONFIG(release, debug|release): DESTDIR = $$OUT_PWD/../../../Deploy/Release
    else:CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/../../../Deploy/Debug

    # Tell it where to find compiled RasterManager.dll
    CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../../Deploy/Release -lRasterManager
    else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../../Deploy/Debug -lRasterManager

    # Find GDAL on OSX
    LIBS += -L/Library/Frameworks/GDAL.framework/Versions/1.11/unix/lib -lgdal
    INCLUDEPATH += /Library/Frameworks/GDAL.framework/Versions/1.11/unix/include
    DEPENDPATH  += /Library/Frameworks/GDAL.framework/Versions/1.11/unix/include

    message("Building to: $$DESTDIR")

}


