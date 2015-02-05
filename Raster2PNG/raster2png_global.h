#ifndef RASTER2PNG_GLOBAL_H
#define RASTER2PNG_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(RASTER2PNG_LIBRARY)
#  define RASTER2PNGSHARED_EXPORT Q_DECL_EXPORT
#else
#  define RASTER2PNGSHARED_EXPORT Q_DECL_IMPORT
#endif

#if defined(_WIN32) || defined(_WIN64)
#   ifdef MY_DLL_EXPORT
#       define R2PNG_DLL_API __declspec(dllexport)
#   else
#       define R2PNG_DLL_API __declspec(dllimport)
#   endif
#else
#   define R2PNG_DLL_API
#endif

#endif // RASTER2PNG_GLOBAL_H
