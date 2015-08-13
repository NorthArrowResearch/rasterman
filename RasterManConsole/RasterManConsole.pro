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
    CONFIG(release, debug|release): BUILD_TYPE = release
    else:CONFIG(debug, debug|release): BUILD_TYPE = debug
    # Look for GDAL in a standard place
    GDAL = $$(GDALDIR)
    isEmpty(GDAL){
        GDAL= $$PWD/../lib/gdal
    }else{
        GDAL= $$(GDALDIR)
    }

    INCLUDEPATH += $$GDAL/include
    DEPENDPATH += $$GDAL/include
    LIBS += -L$$GDAL/lib -lgdal_i

    # Tell it where to find compiled RasterManager.dll
    LIBS += -L$$OUT_PWD/../RasterManager/$$BUILD_TYPE -lRasterManager$$TOOL
    LIBS += -L$$OUT_PWD/../Raster2PNG/$$BUILD_TYPE -lRaster2PNG$$TOOL
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

    # Tell it where to find compiled RasterManager.dll
    LIBS += -L$$OUT_PWD/../RasterManager -lRasterManager
    LIBS += -L$$OUT_PWD/../Raster2PNG -lRaster2PNG
}

INCLUDEPATH += $$PWD/../RasterManager
DEPENDPATH += $$PWD/../RasterManager
