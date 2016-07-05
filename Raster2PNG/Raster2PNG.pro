#-------------------------------------------------
#
# Project created by QtCreator 2015-02-04T14:48:12
#
#-------------------------------------------------

QT       += core widgets
QT       -= gui

VERSION = 6.4.0
TARGET = Raster2PNG
TEMPLATE = lib

DEFINES += PNGLIBVERSION=\\\"$$VERSION\\\" # Makes verion available to c++
DEFINES += MINGDAL=\\\"2.0.1\\\" # Minimum Version of GDAL we need

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

CONFIG(release, debug|release): BUILD_TYPE = release
else:CONFIG(debug, debug|release): BUILD_TYPE = debug

GDALLIB = $$(GDALLIBDIR)

win32 {

    ## There's some trickiness in windows 32 vs 64-bits
    !contains(QMAKE_TARGET.arch, x86_64) {
        ARCH = "32"
        message("x86 build (32 bit) ")
    } else {
        message("x86_64 build (64 bit) ")
        ARCH = "64"
    }

    isEmpty(GDALLIB){
        error("GDALLIBDIR not set. This will cause failures")
    }

    # GDAL is required
    TARGET_EXT = .dll # prevent version suffix on dll
    LIBS += -L$$GDALLIB/lib/ -lgdal_i
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
    DESTDIR = $$OUT_PWD/../../Deploy/$$TOOLDIR$$BUILD_TYPE$$ARCH
}
macx{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.11 #2 ElCapitan
    QMAKE_MAC_SDK = macosx10.11

    # This is mostly to keep the debug builds sane
    DESTDIR = $$OUT_PWD/../../Deploy/$$BUILD_TYPE$$ARCH

    isEmpty(GDALLIB){
        warning("GDALLIBDIR not set. Defaulting to /usr/local")
        GDALLIB = /usr/local
    }

    # GDAL is required
    LIBS += -L$$GDALLIB/lib -lgdal
    INCLUDEPATH += $$GDALLIB/include
    DEPENDPATH  += $$GDALLIB/include

    # Where are we installing to
    target.path = /usr/local/lib
    INSTALLS += target

}
linux{
    # Where are we installing to
    target.path = /usr/local/lib
    INSTALLS += target

    isEmpty(GDALLIB){
        warning("GDALLIBDIR not set. Defaulting to /usr/local")
        GDALLIB = /usr/local
    }

    # GDAL is required
    LIBS += -L$$GDALLIB/lib -lgdal
    INCLUDEPATH += $$GDALLIB/include
    DEPENDPATH  += $$GDALLIB/include
}

