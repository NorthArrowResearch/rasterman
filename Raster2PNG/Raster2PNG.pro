#-------------------------------------------------
#
# Project created by QtCreator 2015-02-04T14:48:12
#
#-------------------------------------------------

QT       += core
QT       += widgets
QT       -= gui

TARGET = Raster2PNG
TARGET_EXT = .dll # prevent version suffix on dll
TEMPLATE = lib

VERSION = 6.1.5
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
    LIBS += -L$$GDALWIN/lib/ -lgdal_i
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
    # GDALNIX = /Users/matt/Projects/nar/gdal/gdal-1.11-debug
    GDALNIX = /Library/Frameworks/GDAL.framework/Versions/1.11/unix
    #    SOURCES += /Users/matt/Projects/nar/gdal/gdal/alg/gdalrasterize.cpp
    LIBS += -L$$GDALNIX/lib -lgdal
    INCLUDEPATH += $$GDALNIX/include
    DEPENDPATH  += $$GDALNIX/include
}
unix:!macx {
    message("Unix")
    # Compile to a central location
    DESTDIR = $$OUT_PWD/../../../Deploy/$$BUILD_TYPE

    target.path = /usr/lib
    INSTALLS += target

    # GDAL is required
    LIBS += -L/usr/lib -lgdal
    INCLUDEPATH += /usr/include/gdal
    DEPENDPATH  += /usr/include/gdal
}



message("Building to: $$DESTDIR")
