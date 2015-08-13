#-------------------------------------------------
#
# Project created by QtCreator 2015-02-04T14:48:12
#
#-------------------------------------------------

QT       += core
QT       += widgets
QT       -= gui

VERSION = 6.1.9
TARGET = Raster2PNG
TARGET_EXT = .dll # prevent version suffix on dll
TEMPLATE = lib

DEFINES += LIBVERSION=\\\"$$VERSION\\\" # Makes verion available to c++
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
    LIBS += -L$$GDALWIN/lib/ -lgdal_i
    INCLUDEPATH += $$GDALWIN/include
    DEPENDPATH += $$GDALWIN/include

    # Compile to a central location
    DESTDIR = $$OUT_PWD/../../../Deploy/$$BUILD_TYPE$$ARCH
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

