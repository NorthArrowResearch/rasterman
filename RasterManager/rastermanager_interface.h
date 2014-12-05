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

enum RasterManagerOutputCodes {
    PROCESS_OK = 0
    , INPUT_FILE_ERROR = 1
    , INPUT_FILE_TRANSFORM_ERROR = 2
    , OUTPUT_FILE_MISSING = 3
    , OUTPUT_FILE_ERROR = 4
    , OUTPUT_NO_DATA_ERROR = 5
    , OUTPUT_FILE_EXT_ERROR = 6
    , OUTPUT_UNHANDLED_DRIVER = 7
    , CELL_SIZE_ERROR = 8
    , LEFT_ERROR = 9
    , TOP_ERROR = 10
    , ROWS_ERROR = 11
    , COLS_ERROR = 12
    , NO_OPERATION_SPECIFIED = 13
    , MISSING_ARGUMENT = 14

    , RM_PNG_QUALITY = 22
    , RM_PNG_TRANSPARENCY = 23
    , RM_PNG_LONG_AXIS =24
    , OTHER_ERROR = 999
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
extern "C" DLL_API const char * GetReturnCodeAsString(int eErrorCode);

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


DLL_API GDALDataset * CreateOutputDSfromRef(const char * pOutputRaster, GDALDataType eDataType, bool bHasNoData, double fNoDataValue, GDALDataset * pReferenceDS);

DLL_API GDALDataset * CreateOutputDS(const char * pOutputRaster, RasterMeta * pTemplateRastermeta);

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
 * @brief
 *
 * @param psInputRaster
 * @param psOutput
 * @return int
 */
extern "C" DLL_API int Mask(const char * psInputRaster, const char *psMaskRaster, const char * psOutput);

extern "C" DLL_API int CreateHillshade(const char * psInputRaster, const char * psOutputHillshade);

extern "C" DLL_API int CreateSlope(const char * psInputRaster, const char * psOutputSlope, int nSlopeType);


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
