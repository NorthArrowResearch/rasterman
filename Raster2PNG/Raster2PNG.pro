#-------------------------------------------------
#
# Project created by QtCreator 2015-02-04T14:48:12
#
#-------------------------------------------------

QT       += core widgets
QT       -= gui

VERSION = 6.1.9
TARGET = Raster2PNG
TEMPLATE = lib

DEFINES += PNGLIBVERSION=\\\"$$VERSION\\\" # Makes verion available to c++
DEFINES += MINGDAL=\\\"1.11.1\\\" # Minimum Version of GDAL we need

DEFINES += RASTER2PNG_LIBRARY

SOURCES += \
    renderer_bytedata.cpp \
    renderer_classified.cpp \
    renderer_gcderror.cpp \
    renderer_gcdptdens.cpp \
    renderer_gcdslopedeg.cpp \
    renderer_gcdslopeper.cpp \
    renderer_stretchminmax.cpp \
    renderer_stretchstddev.cpp \
    renderer.cpp \
    raster2png_interface.cpp

HEADERS +=\
        raster2png_global.h \
    renderer_bytedata.h \
    renderer_classified.h \
    renderer_gcderror.h \
    renderer_gcdptdens.h \
    renderer_gcdslopedeg.h \
    renderer_gcdslopeper.h \
    renderer_stretchminmax.h \
    renderer_stretchstddev.h \
    renderer.h \
    raster2png_interface.h \
    raster2png_exception.h

# When we compile this for an ESRI Addin we have change its name
# To Avoid Collisions
TOOL = $$(TOOLSUFFIX)
TARGET = $$TARGET$$TOOL

win32 {
    # Look for GDAL in a standard place
    GDAL = $$(GDALDIR)
    isEmpty(GDAL){
        GDAL= $$PWD/../lib/gdal
    }else{
        GDAL= $$(GDALDIR)
    }

    TARGET_EXT = .dll # prevent version suffix on dll
    INCLUDEPATH += $$GDAL/include
    DEPENDPATH += $$GDAL/include
    LIBS += -L$$GDAL/lib -lgdal_i
}
macx{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10 #2 Yosemite
}
unix{
    # Where are we installing to
    target.path = /usr/local/lib
    INSTALLS += target

    # GDAL is required
    LIBS += -L/usr/local/lib -lgdal
    INCLUDEPATH += /usr/local/include
    DEPENDPATH  += /usr/local/include
}

