#-------------------------------------------------
#
# Project created by QtCreator 2014-06-17T13:41:46
#
#-------------------------------------------------

QT       -= core gui

VERSION = 6.1.3
TARGET = RasterManager
TARGET_EXT = .dll # prevent version suffix on dll
TEMPLATE = lib

CONFIG += c++11

DEFINES += RASTERMANAGER_LIBRARY

SOURCES += \
    raster.cpp \
    rmexception.cpp \
    raster_resample.cpp \
    dodraster.cpp \
    rastermanager_interface.cpp \
    extentrectangle.cpp \
    rastermeta.cpp \
    raster_hillshade.cpp \
    raster_slope.cpp

HEADERS +=\
    rastermanager_global.h \
    raster.h \
    rmexception.h \
    dodraster.h \
    rastermanager_interface.h \
    extentrectangle.h \
    rastermeta.h

CONFIG(release, debug|release): BUILD_TYPE = Release
else:CONFIG(debug, debug|release): BUILD_TYPE = Debug

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
    LIBS += -L$$GDALWIN/lib -lgdal_i
    INCLUDEPATH += $$GDALWIN/include
    DEPENDPATH += $$GDALWIN/include

    # Compile to a central location
    DESTDIR = $$OUT_PWD/../../../Deploy/$$BUILD_TYPE$$ARCH
}
macx{
    ## OSX common build here
    message("Mac OSX x86_64 build (64bit)")
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10

    # Compile to a central location
    DESTDIR = $$OUT_PWD/../../../Deploy/$$BUILD_TYPE

    # GDAL is required
    GDALNIX = /Library/Frameworks/GDAL.framework/Versions/1.11/unix
    LIBS += -L$$GDALNIX/lib -lgdal
    INCLUDEPATH += $$GDALNIX/include
    DEPENDPATH  += $$GDALNIX/include
}
unix:!macx {
    message("Unix")
    # Compile to a central location
    DESTDIR = /usr/lib

    # GDAL is required
    LIBS += -L/usr/lib -lgdal
    INCLUDEPATH += /usr/include/gdal
    DEPENDPATH  += /usr/include/gdal
}

message("Building to: $$DESTDIR")
