#-------------------------------------------------
#
# Project created by QtCreator 2014-06-17T13:41:46
#
#-------------------------------------------------

QT       += core
QT       -= gui widgets

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
    ## There's some trickiness in windows 32 vs 64-bits
    !contains(QMAKE_TARGET.arch, x86_64) {
        ARCH = "32"
        message("x86 build (32 bit) ")
    } else {
        message("x86_64 build (64 bit) ")
        ARCH = "64"
    }

    # GDAL is required
    GDALWIN = $$PWD/../Libraries/gdalwin$$ARCH-1.10.1
    TARGET_EXT = .dll # prevent version suffix on dll
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
    DESTDIR = $$OUT_PWD/../../../Deploy/$$TOOLDIR$$BUILD_TYPE$$ARCH
}
macx{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.11 #2 ElCapitan
    QMAKE_MAC_SDK = macosx10.11

    target.path = /usr/local/lib
    INSTALLS += target
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
