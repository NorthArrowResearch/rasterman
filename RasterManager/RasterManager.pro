#-------------------------------------------------
#
# Project created by QtCreator 2014-06-17T13:41:46
#
#-------------------------------------------------

QT       += core widgets
QT       -= gui

TARGET = RasterManager
TEMPLATE = lib

VERSION = 6.1.9
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

}
macx{
    ## OSX common build here
    message("Mac OSX x86_64 build (64bit)")
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10

    # If the GDAL environment variable is specified use that.
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

    target.path = /usr/local/lib
    INSTALLS += target

}
unix:!macx {
    message("Unix")

    target.path = /usr/lib
    INSTALLS += target

    # GDAL is required
    LIBS += -L/usr/local/lib -lgdal
    INCLUDEPATH += /usr/include/gdal
    DEPENDPATH  += /usr/include/gdal
}
