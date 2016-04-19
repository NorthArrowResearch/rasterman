#-------------------------------------------------
#
# Project created by QtCreator 2014-10-25T15:13:18
#
#-------------------------------------------------

QT       += core xml
QT       -= gui

VERSION = 6.2.0
DEFINES += EXEVERSION=\\\"$$VERSION\\\" # Makes verion available to c++

TARGET = rasterman

CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

SOURCES += main.cpp \
    rastermanengine.cpp

HEADERS += \
    rastermanengine.h

CONFIG(release, debug|release): BUILD_TYPE = release
else:CONFIG(debug, debug|release): BUILD_TYPE = debug

GDALLIB = $$(GDALLIBDIR)
isEmpty(GDALLIB){
    error("GDALLIBDIR not set. This will cause failures")
}

win32 {

    ## There's some trickiness in windows 32 vs 64-bits
    !contains(QMAKE_TARGET.arch, x86_64) {
        ARCH = "32"
        message("x86 build (32 bit) ")
    } else {
        message("x86_64 build (64 bit) ")
        ARCH = "64"
    }

    # GDAL is required
    LIBS += -L$$GDALLIB/lib -lgdal_i
    INCLUDEPATH += $$GDALLIB/include
    DEPENDPATH += $$GDALLIB/include

    # When we compile this for an ESRI Addin we have change its name
    # To Avoid Collisions
    TOOL = $$(TOOLSUFFIX)
    isEmpty(TOOL){
        TOOLDIR= ""
    }else{
        TOOLDIR=$$TOOL/
    }

    TARGET = $$TARGET$$TOOL
    DESTDIR = $$OUT_PWD/../../../Deploy/$$TOOLDIR$$BUILD_TYPE$$ARCH

    LIBS += -L$$DESTDIR -lRasterManager$$TOOL
    LIBS += -L$$DESTDIR -lRaster2PNG$$TOOL

}
macx{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.11 #2 ElCapitan
    QMAKE_MAC_SDK = macosx10.11

    # Compile to a central location
    DESTDIR = $$OUT_PWD/../../../Deploy/$$BUILD_TYPE

    # GDAL is required
    LIBS += -L$$GDALLIB/lib -lgdal
    INCLUDEPATH += $$GDALLIB/include
    DEPENDPATH  += $$GDALLIB/include

    LIBS += -L$$DESTDIR -lRasterManager
    LIBS += -L$$DESTDIR -lRaster2PNG
}
linux{
    # GDAL is required
    LIBS += -L$$GDALLIB/lib -lgdal
    INCLUDEPATH += $$GDALLIB/include
    DEPENDPATH  += $$GDALLIB/include

    # Where are we installing to
    target.path = /usr/bin
    INSTALLS += target

    LIBS += -L$$OUT_PWD/../RasterManager -lRasterManager
    LIBS += -L$$OUT_PWD/../Raster2PNG -lRaster2PNG
}

INCLUDEPATH += $$PWD/../RasterManager
DEPENDPATH += $$PWD/../RasterManager
