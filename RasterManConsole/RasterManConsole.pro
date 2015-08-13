#-------------------------------------------------
#
# Project created by QtCreator 2014-10-25T15:13:18
#
#-------------------------------------------------

QT       += core xml
QT       -= gui

VERSION = 6.1.9
DEFINES += EXEVERSION=\\\"$$VERSION\\\" # Makes verion available to c++

TARGET = rasterman

CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

SOURCES += main.cpp \
    rastermanengine.cpp

HEADERS += \
    rastermanengine.h

INCLUDEPATH += $$PWD/../RasterManager
DEPENDPATH += $$PWD/../RasterManager

CONFIG(release, debug|release): BUILD_TYPE = release
else:CONFIG(debug, debug|release): BUILD_TYPE = debug

win32 {
    ## There's some trickiness in windows 32 vs 64-bits
    !contains(QMAKE_TARGET.arch, x86_64) {
        ARCH = "32"
        message("x86 build (32 bit) ")
    } else {
        message("x86_64 build (64 bit) ")
        ARCH = "64"
    }

    GDALWIN = $$PWD/../Libraries/gdalwin$$ARCH-1.10.1
    LIBS += -L$$GDALWIN/lib/ -lgdal_i
    INCLUDEPATH += $$GDALWIN/include
    DEPENDPATH += $$GDALWIN/include

    # When we compile this for an ESRI Addin we have change its name
    # To Avoid Collisions
    TOOL = $$(TOOLSUFFIX)
    isEmpty(TOOL){
        TOOLDIR= ""
    }else{
        TOOLDIR=$$TOOL/
    }

    TARGET = $$TARGET$$TOOL
    LIBS += -L$$DESTDIR -lRasterManager$$TOOL

}
macx{
    ## OSX common build here
    message("Mac OSX x86_64 build (64bit)")
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10 #2 Yosemite

    # When we compile this for an ESRI Addin we have change its name
    # To Avoid Collisions
    GDAL = $$(GDALDIR)
    isEmpty(GDAL){
        GDAL= /Library/Frameworks/GDAL.framework/Versions/1.11/unix
    }
    # GDAL is required
    LIBS += -L$$GDAL/lib -lgdal
    INCLUDEPATH += $$GDAL/include
    DEPENDPATH  += $$GDAL/include

    target.path = /usr/local/bin
    INSTALLS += target

    # Tell it where to find compiled RasterManager.dll
    LIBS += -L/usr/local/lib -lRasterManager
    LIBS += -L/usr/local/lib -lRaster2PNG

}
unix:!macx {
    message("Unix")

    target.path = /usr/bin
    INSTALLS += target

    # GDAL is required
    LIBS += -L/usr/local/lib -lgdal
    INCLUDEPATH += /usr/include/gdal
    DEPENDPATH  += /usr/include/gdal

    # Tell it where to find compiled RasterManager.dll
    LIBS += -L/usr/lib -lRasterManager
    LIBS += -L/usr/lib -lRaster2PNG

}
