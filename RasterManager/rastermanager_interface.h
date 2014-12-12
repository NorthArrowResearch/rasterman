#ifndef RASTERINTERFACE_H
#define RASTERINTERFACE_H

#include "rastermanager_global.h"
#include "rastermeta.h"

#include <limits>
#include <math.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>

class GDALDataset;

namespace RasterManager {

/**
 * @brief
 *
 */
enum RasterManagerInputCodes {
    SLOPE_DEGREES = 0,
    SLOPE_PERCENT = 1
};

enum RasterManagerOperators {
    RM_BASIC_MATH_ADD = 1
    , RM_BASIC_MATH_SUBTRACT = 2
    , RM_BASIC_MATH_MULTIPLY = 3
    , RM_BASIC_MATH_DIVIDE = 4
    , RM_BASIC_MATH_POWER = 5
    , RM_BASIC_MATH_SQRT = 6
    , RM_BASIC_MATH_THRESHOLD_PROP_ERROR = 7
};

enum Raster_SymbologyStyle{
    GSS_DEM      = 1,  // DEM
    GSS_DoD      = 2,  // DoD
    GSS_Error    = 3,  // Error
    GSS_Hlsd     = 4,  // HillShade
    GSS_PtDens   = 5,  // PointDensity
    GSS_SlopeDeg = 6,  // SlopeDeg
    GSS_SlopePer = 7,  // SlopePC
    GSS_Unknown  = 8,  // This one is for when the user doesn't enter it.
};

//!Extract File Extension
//* Take a full file path and return just the file extension, excluding the period */
/**
 * @brief
 *
 * @param FileName
 * @return const char
 */
extern "C" DLL_API const char * ExtractFileExt(const char * FileName);

/**
 * @brief
 *
 * @param psFileName
 * @return const char
 */
extern "C" DLL_API const char * GetDriverFromFileName(const char *psFileName);

/**
 * @brief Retrieves the plain english words for a particular raster manager error code
 *
 * @param eErrorCode One of the enumeration integers for the raster manager error codes. e.g. INPUT_FILE_ERROR = 1
 * @return const char
 * A string representing the plain English words describing the error code.
 * e.g. passing the argument INPUT_FILE_ERROR would return the string "input file error."
 */
extern "C" DLL_API void GetReturnCodeAsString(int eErrorCode, const char ** sErr);

/**
 * @brief
 *
 */
extern "C" DLL_API void RegisterGDAL();
extern "C" DLL_API void DestroyGDAL();

/**
 * @brief GetRasterProperties
 *
 * @param ppszRaster
 * @param fCellHeight
 * @param fCellWidth
 * @param fLeft
 * @param fTop
 * @param nRows
 * @param nCols
 * @param fNoData
 * @param bHasNoData
 * @param nDataType
 */
extern "C" DLL_API void GetRasterProperties(const char * ppszRaster,
                                                          double & fCellHeight, double & fCellWidth,
                                                          double & fLeft, double & fTop, int & nRows, int & nCols,
                                                          double & fNoData, int & bHasNoData, int & nDataType);
/**
 * @brief PrintRasterProperties
 *
 * @param ppszRaster
 * @param fCellHeight
 * @param fCellWidth
 * @param fLeft
 * @param fTop
 * @param nRows
 * @param nCols
 * @param fNoData
 * @param bHasNoData
 * @param nDataType
 */
extern "C" DLL_API void PrintRasterProperties(const char * ppszRaster);

/**
 * @brief BiLinearResample
 *
 * @param ppszOriginalRaster
 * @param ppszOutputRaster
 * @param fNewCellSize
 * @param fLeft
 * @param fTop
 * @param nRows
 * @param nCols
 * @return int
 */
extern "C" DLL_API int BiLinearResample(const char * ppszOriginalRaster,
                                                      const char *ppszOutputRaster, double fNewCellSize,
                                                      double fLeft, double fTop, int nRows, int nCols);

/**
 * @brief Copy
 *
 * @param ppszOriginalRaster
 * @param ppszOutputRaster
 * @param fNewCellSize
 * @param fLeft
 * @param fTop
 * @param nRows
 * @param nCols
 * @return int
 */
extern "C" DLL_API int Copy(const char * ppszOriginalRaster,
                                          const char *ppszOutputRaster, double fNewCellSize,
                                          double fLeft, double fTop, int nRows, int nCols);


/**
 * @brief BasicMath
 *
 * @param ppszOriginalRaster1
 * @param ppszOriginalRaster2
 * @param ppszOutputRaster
 */
extern "C" DLL_API int BasicMath(const char * ppszOriginalRaster1,
                                 const char * ppszOriginalRaster2,
                                 const double dOperator,
                                 const int iOperation,
                                 const char * psOutput);

/**
 * @brief CreateOutputDSfromRef
 * @param pOutputRaster
 * @param eDataType
 * @param bHasNoData
 * @param fNoDataValue
 * @param pReferenceDS
 * @return
 */
DLL_API GDALDataset * CreateOutputDSfromRef(const char * pOutputRaster, GDALDataType eDataType, bool bHasNoData, double fNoDataValue, GDALDataset * pReferenceDS);

/**
 * @brief CreateOutputDS
 * @param pOutputRaster
 * @param pTemplateRastermeta
 * @return
 */
DLL_API GDALDataset * CreateOutputDS(const char * pOutputRaster, RasterMeta * pTemplateRastermeta);

/**
 * @brief CreateOutputDSfromRef
 * @param pOutputRaster
 * @param eDataType
 * @param fNoDataValue
 * @param pReferenceDS
 * @return
 */
DLL_API GDALDataset * CreateOutputDSfromRef(const char * pOutputRaster, GDALDataType eDataType, double fNoDataValue, GDALDataset * pReferenceDS);

/**
 * @brief
 *
 * @param psRaster1
 * @param psRaster2
 * @param psOutput
 * @return int
 */
extern "C" DLL_API int RootSumSquares(const char * psRaster1, const char * psRaster2, const char * psOutput);

/**
 * @brief
 *
 * @param psRasters
 * @param psOutput
 * @return int
 */
extern "C" DLL_API int Mosaic(const char *psRasters, const char * psOutput);

/**
 * @brief
 *
 * @param csRasters
 * @param csRasterOutputs
 * @return int
 */
extern "C" DLL_API int MakeConcurrent(const char * csRasters, const char * csRasterOutputs);

/**
 * @brief IsConcurrent
 * @param csRaster
 * @return
 */
extern "C" DLL_API int IsConcurrent(const char * csRaster1, const char * csRaster2);

/**
 * @brief
 *
 * @param psInputRaster
 * @param psOutput
 * @return int
 */
extern "C" DLL_API int Mask(const char * psInputRaster, const char *psMaskRaster, const char * psOutput);

/**
 * @brief CreateHillshade
 * @param psInputRaster
 * @param psOutputHillshade
 * @return
 */
extern "C" DLL_API int CreateHillshade(const char * psInputRaster, const char * psOutputHillshade);

/**
 * @brief CreateSlope
 * @param psInputRaster
 * @param psOutputSlope
 * @param nSlopeType
 * @return
 */
extern "C" DLL_API int CreateSlope(const char * psInputRaster, const char * psOutputSlope, int nSlopeType);

extern "C" DLL_API int CreatePNG(const char * psInputRaster, const char * psOutputPNG, int nImageQuality, int nLongAxisPixels, int nOpacity, int eRasterType);

/**
 * @brief GetSymbologyStyleFromString
 * @param psStyle
 * @return
 */
extern "C" DLL_API Raster_SymbologyStyle GetSymbologyStyleFromString(const char * psStyle);

/**
 * @brief
 *
 * @param psFullString
 * @param psEnding
 * @return bool
 */
bool EndsWith(const char * psFullString, const char * psEnding);

/**
 * @brief Exception class for when a bad conversion happens.
 *
 * TODO: complete this documentation. Is this from Chris Gerard's
 * old code and used when a raster value is read into a 32 bit float
 * but the data do not fit?
 */
class BadConversion : public std::runtime_error {
public:
    /**
     * @brief
     *
     * @param s
     */
    BadConversion(const std::string& s)	: std::runtime_error(s)	{ }
};

/**
 * @brief
 *
 * @param x
 * @return std::string
 */
inline DLL_API std::string stringify(double x) {
    std::ostringstream o;
    if (!(o << x))
        throw BadConversion("stringify(double)");
    return o.str();
}

inline float roundf(float x)
{
   return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f);
}


} // namespace

#endif // RASTERINTERFACE_H
