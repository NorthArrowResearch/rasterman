#ifndef RASTER2PNG_INTERFACE_H
#define RASTER2PNG_INTERFACE_H

#include "raster2png_global.h"

namespace Raster2PNG {

extern "C" R2PNG_DLL_API int CreatePNG(const char * psInputRaster, const char * psOutputPNG, int nImageQuality, int nLongAxisPixels, int nTransparency, int eRasterType);

/**
 * @brief GetSymbologyStyleFromString
 * @param psStyle
 * @return
 */
extern "C" R2PNG_DLL_API int GetSymbologyStyleFromString(const char * psStyle);


}
#endif // RASTER2PNG_INTERFACE_H
