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

# When we compile this for an ESRI Addin we have change its name
# To Avoid Collisions
TOOL = $$(TOOLSUFFIX)
TARGET = $$TARGET$$TOOL

win32 {
    # Look for GDAL in a standard place
    GDAL = $$(GDALDIR)
    isEmpty(GDAL){
        GDAL= ""
    }else{
        GDAL=$$PWD../lib/gdal
    }
    LIBS += -L$$GDAL -lgdal_i
    INCLUDEPATH += $$GDAL/include
    DEPENDPATH += $$GDAL/include
}
macx{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10 #2 Yosemite
}
unix{
    # Where are we installing to
    target.path = /usr/local/bin
    INSTALLS += target

    # GDAL is required
    LIBS += -L/usr/local/lib -lgdal
    INCLUDEPATH += /usr/local/include
    DEPENDPATH  += /usr/local/include
}

INCLUDEPATH += $$PWD/../RasterManager
DEPENDPATH += $$PWD/../RasterManager

# Tell it where to find compiled RasterManager.dll
LIBS += -L$$OUT_PWD/../RasterManager -lRasterManager$$TOOL
LIBS += -L$$OUT_PWD/../Raster2PNG -lRaster2PNG$$TOOL
