#include "raster2png_interface.h"
#include "raster2png_global.h"
#include "raster2png_exception.h"
#include "renderer.h"
#include <QString>
namespace Raster2PNG {

extern "C" R2PNG_DLL_API int CreatePNG(const char * psInputRaster, const char * psOutputPNG, int nImageQuality, int nLongAxisPixels, int nLegend, int nTransparency, int eRasterType)
{
    try {
        Renderer myPNG(psInputRaster, eRasterType, nTransparency, FALSE );
        if (nLegend > 0)
            myPNG.printLegend();
        int eResult = myPNG.rasterToPNG(psOutputPNG, nImageQuality, nLongAxisPixels);
        return eResult;
    }
    catch (Raster2PNGException e){
        return e.GetErrorCode();
    }

}

extern "C" R2PNG_DLL_API int GetSymbologyStyleFromString(const char * psStyle)
{
    QString sStyle(psStyle);

    if (QString::compare(sStyle , "BlackWhite", Qt::CaseInsensitive) == 0)
        return CR_BlackWhite;
    else if (QString::compare(sStyle , "DEM", Qt::CaseInsensitive) == 0)
        return CR_DEM;
    else if (QString::compare(sStyle , "DoD", Qt::CaseInsensitive) == 0)
        return CR_DoD;
    else if (QString::compare(sStyle , "GrainSize", Qt::CaseInsensitive) == 0)
        return CR_GrainSize;
    else if (QString::compare(sStyle , "GreenBlue", Qt::CaseInsensitive) == 0)
        return CR_GreenBlue;
    else if (QString::compare(sStyle , "LtBlueDkBlue", Qt::CaseInsensitive) == 0)
        return CR_LtBlueDkBlue;
    else if (QString::compare(sStyle , "PartialSpectrum", Qt::CaseInsensitive) == 0)
        return CR_PartialSpectrum;
    else if (QString::compare(sStyle , "Precipitation", Qt::CaseInsensitive) == 0)
        return CR_Precipitation;
    else if (QString::compare(sStyle , "Slope", Qt::CaseInsensitive) == 0)
        return CR_Slope;
    else if (QString::compare(sStyle , "SlopeGCD", Qt::CaseInsensitive) == 0)
        return CR_SlopeGCD;
    else if (QString::compare(sStyle , "CR_WhiteRed", Qt::CaseInsensitive) == 0)
        return CR_WhiteRed;
    else
        return CR_BlackWhite;
}

}
