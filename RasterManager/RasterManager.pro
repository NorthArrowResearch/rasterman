#-------------------------------------------------
#
# Project created by QtCreator 2014-06-17T13:41:46
#
#-------------------------------------------------

QT       -= core gui

VERSION = 6.0.45
TARGET = RasterManager
TEMPLATE = lib

QMAKE_CXXFLAGS += -stdlib=libc++
QMAKE_CXXFLAGS += -std=c++11
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10 #2



DEFINES += RASTERMANAGER_LIBRARY

SOURCES += \
    raster.cpp \
    rmexception.cpp \
    raster_resample.cpp \
    dodraster.cpp \
    rastermanager_interface.cpp \
    extentrectangle.cpp \
    rastermeta.cpp

HEADERS +=\
    rastermanager_global.h \
    raster.h \
    rmexception.h \
    dodraster.h \
    rastermanager_interface.h \
    extentrectangle.h \
    rastermeta.h

win32 {
    ## Windows common build here
    !contains(QMAKE_TARGET.arch, x86_64) {
        message("x86 build (32 bit) ")
        LIBS += -L$$PWD/../Libraries/gdalwin32-1.10.1/lib/ -lgdal_i
        INCLUDEPATH += $$PWD/../Libraries/gdalwin32-1.10.1/include
        DEPENDPATH += $$PWD/../Libraries/gdalwin32-1.10.1/include
    } else {
        message("x86_64 build (64 bit) ")
        LIBS += -L$$PWD/../Libraries/gdalwin64-1.10.1/lib/ -lgdal_i
        INCLUDEPATH += $$PWD/../Libraries/gdalwin64-1.10.1/include
        DEPENDPATH += $$PWD/../Libraries/gdalwin64-1.10.1/include
    }
}
macx{
    ## OSX common build here
    message("Mac OSX x86_64 build (64bit)")

    CONFIG(release, debug|release): DESTDIR = $$OUT_PWD/../../../Deploy/Release
    else:CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/../../../Deploy/Debug

    LIBS += -L/Library/Frameworks/GDAL.framework/Versions/1.11/unix/lib -lgdal
    INCLUDEPATH += /Library/Frameworks/GDAL.framework/Versions/1.11/unix/include
    DEPENDPATH  += /Library/Frameworks/GDAL.framework/Versions/1.11/unix/include

    message("Building to: $$DESTDIR")
}
