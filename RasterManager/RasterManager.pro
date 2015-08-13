#-------------------------------------------------
#
# Project created by QtCreator 2014-06-17T13:41:46
#
#-------------------------------------------------

QT       += core widgets
QT       -= gui

VERSION = 6.1.9
TARGET = RasterManager
TEMPLATE = lib

DEFINES += RMLIBVERSION=\\\"$$VERSION\\\" # Makes verion available to c++
DEFINES += MINGDAL=\\\"1.11.1\\\" # Minimum Version of GDAL we need

DEFINES += RASTERMANAGER_LIBRARY

SOURCES += \
    raster.cpp \
    raster_resample.cpp \
    dodraster.cpp \
    rastermanager_interface.cpp \
    extentrectangle.cpp \
    rastermeta.cpp \
    raster_hillshade.cpp \
    raster_slope.cpp \
    raster_math.cpp \
    raster_csv.cpp \
    raster_concurrency.cpp \
    raster_mask.cpp \
    raster_rootsumsquare.cpp \
    raster_mosaic.cpp \
    rastermanager.cpp \
    raster_invert.cpp \
    raster_normalize.cpp \
    raster_filter.cpp \
    raster_extractvals.cpp \
    raster_pitremove.cpp \
    raster_eucliddist.cpp \
    raster_stats.cpp \
    raster_linthresh.cpp \
    rasterarray.cpp \
    raster_combine.cpp \
    raster_smoothedge.cpp \
    raster_area.cpp \
    raster_gutpolygon.cpp \
    raster_vector2raster.cpp \
    raster_setnull.cpp

HEADERS +=\
    rastermanager_global.h \
    raster.h \
    dodraster.h \
    rastermanager_interface.h \
    extentrectangle.h \
    rastermeta.h \
    rastermanager_exception.h \
    rastermanager.h \
    raster_pitremove.h \
    benchmark.h \
    rasterarray.h \
    raster_gutpolygon.h


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
    TARGET_EXT = .dll # prevent version suffix on dll
    LIBS += -L$$GDAL -lgdal_i
    INCLUDEPATH += $$GDAL/include
    DEPENDPATH += $$GDAL/include
}
macx{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
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
