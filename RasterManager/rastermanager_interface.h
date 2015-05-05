#ifndef RASTERINTERFACE_H
#define RASTERINTERFACE_H

#include "rastermanager_global.h"
#include "rastermanager_exception.h"
#include "rastermeta.h"

#include <limits>
#include <math.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <QString>

class GDALDataset;

namespace RasterManager {

/**
 * @brief
 *
 */
enum RasterManagerInputCodes { SLOPE_DEGREES, SLOPE_PERCENT };

enum FillMode { FILL_MINCOST, FILL_BAL , FILL_CUT };

enum RasterManagerOperators {
    RM_BASIC_MATH_ADD
    , RM_BASIC_MATH_SUBTRACT
    , RM_BASIC_MATH_MULTIPLY
    , RM_BASIC_MATH_DIVIDE
    , RM_BASIC_MATH_POWER
    , RM_BASIC_MATH_SQRT
    , RM_BASIC_MATH_THRESHOLD_PROP_ERROR
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


enum Raster_Stats_Operation{
    STATS_MEAN,
    STATS_MEDIAN,
    STATS_MAJORITY,
    STATS_MINORITY,
    STATS_MAXIMUM,
    STATS_MINIMUM,
    STATS_STD,
    STATS_SUM,
    STATS_VARIETY,
    STATS_RANGE,
};

extern "C" RM_DLL_API const char * GetLibVersion();

extern "C" RM_DLL_API const char * GetMinGDALVersion();

extern "C" RM_DLL_API int CheckVersions();

//!Extract File Extension
//* Take a full file path and return just the file extension, excluding the period */
/**
 * @brief
 *
 * @param FileName
 * @return const char
 */
extern "C" RM_DLL_API const char * ExtractFileExt(const char * FileName);

/**
 * @brief
 *
 * @param psFileName
 * @return const char
 */
extern "C" RM_DLL_API const char * GetDriverFromFileName(const char *psFileName);

/**
 * @brief GetMathOpFromString
 * @param psOp
 * @return
 */
extern "C" RM_DLL_API int GetMathOpFromString(const char * psOp);

/**
 * @brief GetStatFromString
 * @param psStat
 * @return
 */
extern "C" RM_DLL_API int GetStatFromString(const char * psStat);

/**
 * @brief CompareRasters
 * @param ppszRaster1
 * @param ppszRaster2
 * @param sErr
 * @return
 */
extern "C" RM_DLL_API int RasterCompare(const char * ppszRaster1, const char * ppszRaster2, char * sErr);

/**
 * @brief Retrieves the plain english words for a particular raster manager error code
 *
 * @param eErrorCode One of the enumeration integers for the raster manager error codes. e.g. INPUT_FILE_ERROR = 1
 * @return const char
 * A string representing the plain English words describing the error code.
 * e.g. passing the argument INPUT_FILE_ERROR would return the string "input file error."
 */
extern "C" RM_DLL_API void GetReturnCodeAsString(unsigned int eErrorCode, char * sErr);

/**
 * @brief
 *
 */
extern "C" RM_DLL_API void RegisterGDAL();
extern "C" RM_DLL_API void DestroyGDAL();

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
extern "C" RM_DLL_API int GetRasterProperties(const char * ppszRaster,
                                               double & fCellHeight, double & fCellWidth,
                                               double & fLeft, double & fTop, int & nRows, int & nCols,
                                               double & fNoData, int & bHasNoData, int & nDataType, char *sErr);
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
extern "C" RM_DLL_API void PrintRasterProperties(const char * ppszRaster);

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
extern "C" RM_DLL_API int BiLinearResample(const char * ppszOriginalRaster,
                                           const char *ppszOutputRaster, double fNewCellSize,
                                           double fLeft, double fTop, int nRows, int nCols, char *sErr);

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
extern "C" RM_DLL_API int Copy(const char * ppszOriginalRaster,
                               const char *ppszOutputRaster, double fNewCellSize,
                               double fLeft, double fTop, int nRows, int nCols, char * sErr);


/**
 * @brief BasicMath
 *
 * @param ppszOriginalRaster1
 * @param ppszOriginalRaster2
 * @param ppszOutputRaster
 */
extern "C" RM_DLL_API int BasicMath(const char * ppszOriginalRaster1,
                                    const char * ppszOriginalRaster2,
                                    const double *dNumericArg,
                                    const char *psOperation,
                                    const char * psOutput, char *sErr);

/**
 * @brief CreateOutputDSfromRef
 * @param pOutputRaster
 * @param eDataType
 * @param bHasNoData
 * @param fNoDataValue
 * @param pReferenceDS
 * @return
 */
RM_DLL_API GDALDataset * CreateOutputDSfromRef(const char * pOutputRaster,
                                               GDALDataType eDataType,
                                               bool bHasNoData,
                                               double fNoDataValue,
                                               GDALDataset * pReferenceDS);

/**
 * @brief CreateOutputDS
 * @param pOutputRaster
 * @param pTemplateRastermeta
 * @return
 */
RM_DLL_API GDALDataset * CreateOutputDS(const char * pOutputRaster, RasterMeta * pTemplateRastermeta);

RM_DLL_API GDALDataset * CreateOutputDS(QString sOutputRaster, RasterMeta * pTemplateRasterMeta);

/**
 * @brief CreateOutputDSfromRef
 * @param pOutputRaster
 * @param eDataType
 * @param fNoDataValue
 * @param pReferenceDS
 * @return
 */
RM_DLL_API GDALDataset * CreateOutputDSfromRef(const char * pOutputRaster, GDALDataType eDataType, double fNoDataValue, GDALDataset * pReferenceDS);

/**
 * @brief
 *
 * @param psRaster1
 * @param psRaster2
 * @param psOutput
 * @return int
 */
extern "C" RM_DLL_API int RootSumSquares(const char * psRaster1, const char * psRaster2, const char * psOutput, char *sErr);

/**
 * @brief
 *
 * @param psRasters
 * @param psOutput
 * @return int
 */
extern "C" RM_DLL_API int Mosaic(const char *psRasters, const char * psOutput, char *sErr);

/**
 * @brief Combine
 * @param csRasters
 * @param psOutput
 * @param psMethod
 * @return
 */
extern "C" RM_DLL_API int Combine(const char * csRasters, const char * psOutput,  const char * psMethod, char *sErr);


/**
 * @brief
 *
 * @param csRasters
 * @param csRasterOutputs
 * @return int
 */
extern "C" RM_DLL_API int MakeConcurrent(const char * csRasters, const char * csRasterOutputs, char *sErr);

/**
 * @brief LinearThreshold
 * @param psInputRaster
 * @param psOutputRaster
 * @param dLowThresh
 * @param dLowThreshVal
 * @param dHighThresh
 * @param dHighThreshVal
 * @return
 */
extern "C" RM_DLL_API int LinearThreshold(const char * psInputRaster,
                                          const char * psOutputRaster,
                                          double dLowThresh,
                                          double dLowThreshVal,
                                          double dHighThresh,
                                          double dHighThreshVal, char *sErr);
/**
 * @brief AreaThreshold
 * @param psInputRaster
 * @param psOutputRaster
 * @param dAreaThresh
 * @return
 */
extern "C" RM_DLL_API int AreaThreshold(const char * psInputRaster,
                                          const char * psOutputRaster,
                                          double dAreaThresh, char *sErr);

/**
 * @brief SmoothEdges
 * @param psInputRaster
 * @param psOutputRaster
 * @param nCells
 * @param sErr
 * @return
 */
extern "C" RM_DLL_API int SmoothEdges(const char * psInputRaster,
                                      const char * psOutputRaster,
                                      int nCells,
                                      char * sErr);

/**
 * @brief IsConcurrent
 * @param csRaster
 * @return
 */
extern "C" RM_DLL_API int IsConcurrent(const char * csRaster1, const char * csRaster2, char *sErr);

/**
 * @brief
 *
 * @param psInputRaster
 * @param psOutput
 * @return int
 */
extern "C" RM_DLL_API int Mask(const char * psInputRaster, const char *psMaskRaster, const char * psOutput, char *sErr);

/**
 * @brief
 *
 * @param psInputRaster
 * @param psOutput
 * @param dMaskValue
 * @return int
 */
extern "C" RM_DLL_API int MaskValue(const char * psInputRaster, const char * psOutput, double dMaskValue, char *sErr);

/**
 * @brief CreateHillshade
 * @param psInputRaster
 * @param psOutputHillshade
 * @return
 */
extern "C" RM_DLL_API int CreateHillshade(const char * psInputRaster, const char * psOutputHillshade, char *sErr);

/**
 * @brief CreateSlope
 * @param psInputRaster
 * @param psOutputSlope
 * @param nSlopeType
 * @return
 */
extern "C" RM_DLL_API int CreateSlope(const char * psInputRaster, const char * psOutputSlope, const char *psSlopeType, char *sErr);

/**
 * @brief RasterInvert
 * @param psRaster1
 * @param psRaster2
 * @param dValue
 * @return
 */
extern "C" RM_DLL_API int RasterInvert(const char * psRaster1,
                                       const char * psRaster2,
                                       double dValue,
                                       char *sErr);

/**
 * @brief RasterNormalize
 * @param psRaster1
 * @param psRaster2
 * @return
 */
extern "C" RM_DLL_API int RasterNormalize(const char * psRaster1,
                                          const char * psRaster2,
                                          char *sErr);

/**
 * @brief RasterFilter
 * @param psOperation
 * @param psRaster1
 * @param psRaster2
 * @param psWidth
 * @param psHeight
 * @return
 */
extern "C" RM_DLL_API int RasterFilter(const char * psOperation,
                                       const char * psInputRaster,
                                       const char * psOutputRaster,
                                       int nWidth,
                                       int nHeight ,
                                       char *sErr);

/**
 * @brief ExtractRasterPoints
 * @param sCSVInputSourcePath
 * @param sRasterInputSourcePath
 * @param sCSVOutputPath
 * @param sXField
 * @param sYField
 * @param sNodata
 * @return
 */
extern "C" RM_DLL_API int ExtractRasterPoints(const char * sCSVInputSourcePath,
                                              const char * sRasterInputSourcePath,
                                              const char * sCSVOutputPath,
                                              const char * sXField,
                                              const char * sYField,
                                              const char * sNodata,
                                              char *sErr);

/**
 * @brief CreatePNG
 * @param psInputRaster
 * @param psOutputPNG
 * @param nImageQuality
 * @param nLongAxisPixels
 * @param nOpacity
 * @param eRasterType
 * @return
 */
extern "C" RM_DLL_API int CreatePNG(const char * psInputRaster, const char * psOutputPNG, int nImageQuality,
                                    int nLongAxisPixels, int nOpacity, int eRasterType,
                                    char *sErr);

/**
 * @brief GetSymbologyStyleFromString
 * @param psStyle
 * @return
 */
extern "C" RM_DLL_API int GetSymbologyStyleFromString(const char * psStyle);

/**
 * @brief GetFillMethodFromString
 * @param psMethod
 * @return
 */
extern "C" RM_DLL_API int GetFillMethodFromString(const char * psMethod);

/**
 * @brief RasterStat
 * @param psOperation
 * @return
 */
extern "C" RM_DLL_API double RasterGetStat(const char * psOperation, const char *psInputRaster, char *sErr);

/**
 * @brief StackStats
 * @param psOperation
 * @param csRasters
 * @param psOutput
 * @param sErr
 * @return
 */
extern "C" RM_DLL_API int StackStats(const char * psOperation, const char * csRasters, const char * psOutput, char * sErr);


/**
 * @brief GetStatOperationFromString
 * @param psStyle
 * @return
 */
extern "C" RM_DLL_API int GetStatOperationFromString(const char * psStats);

/**
 * @brief RasterFromCSVandTemplate
 * @param sCSVSourcePath
 * @param psOutput
 * @param sRasterTemplate
 * @param sXField
 * @param sYField
 * @param sDataField
 * @return
 */
extern "C" RM_DLL_API int RasterFromCSVandTemplate(const char * sCSVSourcePath,
                                                   const char * psOutput,
                                                   const char * sRasterTemplate,
                                                   const char * sXField,
                                                   const char * sYField,
                                                   const char * sDataField ,
                                                   char *sErr);
/**
 * @brief RasterFromCSVandExtents
 * @param sCSVSourcePath
 * @param sOutput
 * @param dTop
 * @param dLeft
 * @param nRows
 * @param nCols
 * @param dCellWidth
 * @param dNoDataVal
 * @param sXField
 * @param sYField
 * @param sDataField
 * @return
 */
extern "C" RM_DLL_API int RasterFromCSVandExtents(const char * sCSVSourcePath,
                                                  const char * sOutput,
                                                  double dTop,
                                                  double dLeft,
                                                  int nRows,
                                                  int nCols,
                                                  double dCellWidth,
                                                  double dNoDataVal,
                                                  const char * sXField,
                                                  const char * sYField,
                                                  const char * sDataField,
                                                  char *sErr);

/**
 * @brief RasterEuclideanDistance
 * @param psRaster1
 * @param psRaster2
 * @return
 */
extern "C" RM_DLL_API int RasterEuclideanDistance(const char * psRaster1,
                                                  const char * psRaster2,
                                                  char *sErr);
/**
 * @brief InitCInterfaceError
 * @param sErr
 */
RM_DLL_API void InitCInterfaceError(char * sErr);

/**
 * @brief SetCInterfaceError
 * @param e
 * @param sErr
 */
RM_DLL_API void SetCInterfaceError(RasterManagerException e, char * sErr);

/**
 * @brief
 *
 * @param psFullString
 * @param psEnding
 * @return bool
 */
bool EndsWith(const char * psFullString, const char * psEnding);

/**
 * @brief printString
 * @param theString
 */
void RM_DLL_API printLine(QString theString);


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

// Helper class we use often
QString appendToBaseFileName(QString sFilePath, QString sAppendStr);

/**
 * @brief
 *
 * @param x
 * @return std::string
 */
inline RM_DLL_API std::string stringify(double x) {
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
