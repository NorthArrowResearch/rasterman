TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += \
    Raster2PNG \
    RasterManager \
    RasterManConsole \

RasterManConsole.depends = RasterManager Raster2PNG

win32{
  message(Building for Win32)
}
win64{
  message(Building for Win64)
}
macx{
  message(Building for macx)
}
linux{
  message(Building for linux)
}
