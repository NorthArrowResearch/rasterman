#ifndef RASTERMANAGER_GLOBAL_H
#define RASTERMANAGER_GLOBAL_H

/*! \mainpage Raster Manager Library
 *
 * This raster manager library contains a set of generic, reusable classes
 * for reading and writing [GDAL](http://www.gdal.org/) compatible rasters.
 *
 * The classes here are deliberately low-level and flexible. You can extend
 * them for more specific purposes. See the GCD documentation for an example
 * of how this is done.
 *
 * This library contains a series of pure C methods that expose common raster
 * opertations for use both in other C++ libraries but also Visual Studio.
 * See the definitions in rastermanager_interface.h for more details. These
 * methods can be used directly within VB.net or C# without the need for any
 * intermediary technology (such as [SWIG](http://www.swig.org/)).
 *
 * \section Dependencies
 * The raster manager avoids any unnecessary dependencies. For example there
 * are no Qt classes used so as to make deployment as simple as possible.
 *
 * - [GDAL](http://www.gdal.org/) 1.10.1
 * - Standard C++ libraries
 */

//#include <QtCore/qglobal.h>


#if defined(RASTERMANAGER_LIBRARY)
#  define RASTERMANAGERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define RASTERMANAGERSHARED_EXPORT Q_DECL_IMPORT
#endif
 
#if defined(_WIN32) || defined(_WIN64)
#   ifdef MY_DLL_EXPORT
#       define DLL_API __declspec(dllexport)
#   else
#       define DLL_API
#   endif
#else
#   define DLL_API __declspec(dllimport)
#endif

#endif // RASTERMANAGER_GLOBAL_H
