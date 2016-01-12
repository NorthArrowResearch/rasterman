TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += \
    Raster2PNG \
    RasterManager \
    RasterManConsole \

win32{
  message(Building fow Win32)
}
win64{
  message(Building fow Win64)
}
macx{
  message(Building fow macx)
}
linux{
  message(Building fow linux)
}