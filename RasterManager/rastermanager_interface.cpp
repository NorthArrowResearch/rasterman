#define MY_DLL_EXPORT

#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "extentrectangle.h"

#include <stdio.h>
#include "extentrectangle.h"
#include "rastermeta.h"
#include "rastermanager.h"

#include "raster.h"
#include "gdal_priv.h"

#include <limits>
#include <math.h>
#include <string>

#include <algorithm>
#include <iostream>

namespace RasterManager {

RM_DLL_API const char * GetLibVersion(){ return LIBVERSION; }

RM_DLL_API GDALDataset * CreateOutputDS(const char * pOutputRaster,
                         GDALDataType eDataType,
                         bool bHasNoData,
                         double fNoDataValue,
                         int nCols, int nRows, double * newTransform, const char * projectionRef){

    RasterMeta pInputMeta(newTransform[3], newTransform[0], nRows, nCols, &newTransform[5], &newTransform[1], &fNoDataValue, NULL, &eDataType, projectionRef);
    if (bHasNoData){

    }
   return CreateOutputDS(pOutputRaster, &pInputMeta);
}

RM_DLL_API GDALDataset * CreateOutputDS(const char * pOutputRaster, RasterMeta * pTemplateRastermeta){

    // Make sure the file doesn't exist. Throws an exception if it does.
    CheckFile(pOutputRaster, false);

    /* Create the new dataset. Determine the driver from the output file extension.
     * Enforce LZW compression for TIFs. The predictor 3 is used for floating point prediction.
     * Not using this value defaults the LZW to prediction to 1 which causes striping.
     */

    char **papszOptions = NULL;
    GDALDriver * pDR = NULL;


    if ( pTemplateRastermeta->GetGDALDriver() ){
        pDR = GetGDALDriverManager()->GetDriverByName(pTemplateRastermeta->GetGDALDriver());
    }
    else {
        pDR = GetGDALDriverManager()->GetDriverByName(GetDriverFromFileName(pOutputRaster));
    }

    if (strcmp( pDR->GetDescription() , "GTiff") == 0){
        papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "LZW");
    }

    GDALDataset * pDSOutput =  pDR->Create(pOutputRaster,
                                          pTemplateRastermeta->GetCols(),
                                          pTemplateRastermeta->GetRows(),
                                          1,
                                          *pTemplateRastermeta->GetGDALDataType(),
                                          papszOptions);

    CSLDestroy( papszOptions );

    if (pDSOutput == NULL)
        return NULL;

    if (pTemplateRastermeta->HasNoDataValue())
    {
        CPLErr er = pDSOutput->GetRasterBand(1)->SetNoDataValue(pTemplateRastermeta->GetNoDataValue());
        if (er == CE_Failure || er == CE_Fatal)
            return NULL;
    }
    else{
        CPLErr er = pDSOutput->GetRasterBand(1)->SetNoDataValue(0);
        if (er) { }
    }

    double * newTransform = pTemplateRastermeta->GetGeoTransform();
    char * projectionRef = pTemplateRastermeta->GetProjectionRef();

    // Fill the new raster set with nodatavalue
    pDSOutput->GetRasterBand(1)->Fill(pTemplateRastermeta->GetNoDataValue());

    if (newTransform != NULL)
        pDSOutput->SetGeoTransform(newTransform);
    if (projectionRef != NULL)
        pDSOutput->SetProjection(projectionRef);
    return pDSOutput;

}

RM_DLL_API GDALDataset * CreateOutputDSfromRef(const char * pOutputRaster,
                                            GDALDataType eDataType,
                                            bool bHasNoData,
                                            double fNoDataValue,
                                            GDALDataset * pReferenceDS)
{

    int nCols = pReferenceDS->GetRasterBand(1)->GetXSize();
    int nRows = pReferenceDS->GetRasterBand(1)->GetYSize();

    double fTransform[6];
    pReferenceDS->GetGeoTransform(fTransform);

    double newTransform[6];
    newTransform[0] = fTransform[0];
    newTransform[1] = fTransform[1];
    newTransform[2] = 0;
    newTransform[3] = fTransform[3];
    newTransform[4] = 0;
    newTransform[5] = fTransform[5];

    const char * newProjetionRef = pReferenceDS->GetProjectionRef();

    return CreateOutputDS(pOutputRaster, eDataType, bHasNoData, fNoDataValue, nCols, nRows, newTransform, newProjetionRef);

}


extern "C" RM_DLL_API void RegisterGDAL() { GDALAllRegister();}
extern "C" RM_DLL_API void DestroyGDAL() { GDALDestroyDriverManager();}

extern "C" RM_DLL_API int BasicMath(const char * psRaster1,
                                 const char * psRaster2,
                                 const double * dOperator,
                                 const int iOperation,
                                 const char * psOutput)
{
    try {
        return Raster::RasterMath(psRaster1,
                   psRaster2,
                   dOperator,
                   iOperation,
                   psOutput);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }
}

extern "C" RM_DLL_API int RasterInvert(const char * psRaster1,
                                 const char * psRaster2,
                                 double dValue)
{
    try {
        return Raster::InvertRaster(
                    psRaster1,
                    psRaster2,
                    dValue);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }
}


extern "C" RM_DLL_API int RasterNormalize(const char * psRaster1,
                                 const char * psRaster2)
{
    try {
        return Raster::NormalizeRaster(
                    psRaster1,
                    psRaster2);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }
}


extern "C" RM_DLL_API int CreateHillshade(const char * psInputRaster, const char * psOutputHillshade){

    try {
        Raster pDemRaster (psInputRaster);
        return pDemRaster.Hillshade(psOutputHillshade);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }

}

extern "C" RM_DLL_API int CreateSlope(const char * psInputRaster, const char * psOutputSlope, int nSlopeType){

    try{
        Raster pDemRaster (psInputRaster);
        return pDemRaster.Slope(psOutputSlope, nSlopeType);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }

}

extern "C" RM_DLL_API int Mask(const char * psInputRaster, const char * psMaskRaster, const char * psOutput)
{
    try{
        return Raster::RasterMask(psInputRaster, psMaskRaster, psOutput);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }
}

extern "C" RM_DLL_API int RootSumSquares(const char * psRaster1,
                                      const char * psRaster2,
                                      const char * psOutput)
{
    try{
        return Raster::RasterRootSumSquares(psRaster1, psRaster2, psOutput);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }

}


extern "C" RM_DLL_API int Mosaic(const char * csRasters, const char * psOutput)
{
    try{
        return Raster::RasterMosaic(csRasters, psOutput);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }

}

extern "C" RM_DLL_API int IsConcurrent(const char * csRaster1, const char * csRaster2)
{
    try{
        RasterManager::RasterMeta rmRaster1(csRaster1);
        RasterManager::RasterMeta rmRaster2(csRaster2);

        return rmRaster1.IsConcurrent(&rmRaster2);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }

}

extern "C" RM_DLL_API int MakeConcurrent(const char * csRasters, const char * csRasterOutputs)
{
    try{
        return Raster::MakeRasterConcurrent(csRasters, csRasterOutputs);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }
}

/*******************************************************************************************************
 *******************************************************************************************************
 * Raster property methods
 */

extern "C" RM_DLL_API void GetRasterProperties(const char * ppszRaster,
                                                          double & fCellHeight, double & fCellWidth,
                                                          double & fLeft, double & fTop, int & nRows, int & nCols,
                                                          double & fNoData, int & bHasNoData, int & nDataType)
{
    try{
        RasterManager::Raster r(ppszRaster);
        fCellHeight = r.GetCellHeight();
        fCellWidth = r.GetCellWidth();
        fLeft = r.GetLeft();
        fTop = r.GetTop();
        nRows = r.GetRows();
        nCols = r.GetCols();
        fNoData = r.GetNoDataValue();
        bHasNoData = (int) r.HasNoDataValue();
        nDataType = (int) *r.GetGDALDataType();
    }
    catch (RasterManagerException e){
        // e.GetErrorCode();
    }
}

extern "C" RM_DLL_API void PrintRasterProperties(const char * ppszRaster)
{

    try{
        double fCellWidth = 0;
        double fCellHeight = 0;
        double fLeft = 0;
        double fTop = 0;
        double fBottom = 0;
        double fRight = 0;
        int nRows = 0;
        int nCols = 0;
        double fNoData = 0;
        double dRasterMax = 0;
        double dRasterMin = 0;
        int bHasNoData = 0;
        int orthogonal = 0;
        int nDataType;

        RasterManager::Raster r(ppszRaster);
        std::string projection = r.GetProjectionRef();

        fCellHeight = r.GetCellHeight();
        fCellWidth = r.GetCellWidth();
        fLeft = r.GetLeft();
        fRight = r.GetRight();
        fTop = r.GetTop();
        fBottom = r.GetBottom();
        nRows = r.GetRows();
        nCols = r.GetCols();
        dRasterMax = r.GetMaximum();
        dRasterMin = r.GetMinimum();
        orthogonal = r.IsOthogonal();
        fNoData = r.GetNoDataValue();
        bHasNoData = (int) r.HasNoDataValue();
        nDataType = (int) *r.GetGDALDataType();

        printLine( QString("     Raster: %1").arg(ppszRaster));
        printLine( QString("       Left: %1   Right: %2").arg(fLeft).arg(fRight));

        printLine( QString("        Top: %1     Bottom: %2").arg(fTop).arg(fBottom));
        printLine( QString("       Rows: %1    Cols: %2").arg(nRows).arg(nCols));
        printLine( QString("        "));
        printLine( QString("       Cell Width: %1").arg(fCellWidth));
        printLine( QString("              Min: %1      Max: %2").arg(dRasterMin).arg(dRasterMax));
        printLine( QString("             Left: %1      Right: %2").arg(fLeft).arg(fRight));

        if (orthogonal == 1 ){
            printLine( QString("       Orthogonal: True" ) );
        }
        else {
            printLine( QString("       Orthogonal: False" ) );
        }
        std::cout << "\n";
        switch (nDataType)
        {
        // Note 0 = unknown;
        case  1: std::cout << "\n      Data Type: 1, GDT_Byte, Eight bit unsigned integer"; break;
        case  2: std::cout << "\n      Data Type: 2, GDT_UInt16, Sixteen bit unsigned integer"; break;
        case  3: std::cout << "\n      Data Type: 3, GDT_Int16, Sixteen bit signed integer"; break;
        case  4: std::cout << "\n      Data Type: 4, GDT_UInt32, Thirty two bit unsigned integer"; break;
        case  5: std::cout << "\n      Data Type: 5, GDT_Int32, Thirty two bit signed integer"; break;
        case  6: std::cout << "\n      Data Type: 6, GDT_Float32, Thirty two bit floating point"; break;
        case  7: std::cout << "\n      Data Type: 7, GDT_Float64, Sixty four bit floating point"; break;
        case  8: std::cout << "\n      Data Type: 8, GDT_CInt16, Complex Int16"; break;
        case  9: std::cout << "\n      Data Type: 9, GDT_CInt32, Complex Int32"; break;
        case 10: std::cout << "\n      Data Type: 10, GDT_CFloat32, Complex Float32"; break;
        case 11: std::cout << "\n      Data Type: 11, GDT_CFloat64, Complex Float64"; break;
        default: std::cout << "\n      Data Type: Unknown"; break;
        }
        if (bHasNoData)
            std::cout << "\n        No Data: " << fNoData;
        else
            std::cout << "\n        No Data: none";

        std::cout << "\n     Projection: " << projection.substr(0,70) << "...";
        std::cout << "\n ";

    }
    catch (RasterManagerException e){
        //e.GetErrorCode();
    }
}

extern "C" RM_DLL_API int CreatePNG(const char * psInputRaster, const char * psOutputPNG, int nImageQuality, int nLongAxisPixels, int nOpacity, int eRasterType)
{
    RasterManager::Raster rOriginal(psInputRaster);
    int eResult = rOriginal.RastertoPng(psOutputPNG, nImageQuality, nLongAxisPixels, nOpacity, (Raster_SymbologyStyle) eRasterType);
    return eResult;
}

/*******************************************************************************************************
 *******************************************************************************************************
 * Raster copy and resample methods
 */

extern "C" RM_DLL_API int Copy(const char * ppszOriginalRaster,
                                          const char *ppszOutputRaster,
                                          double fNewCellSize,
                                          double fLeft, double fTop, int nRows, int nCols)
{
    try{
        RasterManager::Raster ra(ppszOriginalRaster);
        return ra.Copy(ppszOutputRaster, &fNewCellSize, fLeft, fTop, nRows, nCols);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }
}

extern "C" RM_DLL_API int BiLinearResample(const char * ppszOriginalRaster,
                                                      const char *ppszOutputRaster,
                                                      double fNewCellSize,
                                                      double fLeft, double fTop, int nRows, int nCols)
{
    try{
        RasterManager::Raster ra(ppszOriginalRaster);
        return ra.ReSample(ppszOutputRaster, fNewCellSize, fLeft, fTop, nRows, nCols);
    }
    catch (RasterManagerException e){
        return e.GetErrorCode();
    }
}

extern "C" RM_DLL_API const char * ExtractFileExt(const char * FileName)
{
    for (int i = strlen(FileName); i >= 0; i--) {
        if (FileName[i] == '.' ){
            return &FileName[i];
        }
    }
    return NULL;
}

extern "C" RM_DLL_API const char * GetDriverFromFilename(const char * FileName)
{
    const char * pSuffix = ExtractFileExt(FileName);

    if (pSuffix == NULL)
        return NULL;
    else
    {
        if (strcmp(pSuffix, ".tif") == 0)
        {
            return "GTiff";
        }
        else if (strcmp(pSuffix, ".img") == 0)
            return "HFA";
        else
            return NULL;
    }
}

extern "C" RM_DLL_API const char * GetDriverFromFileName(const char * psFileName)
{
    if (EndsWith(psFileName, ".tif"))
    {
        return "GTiff";
    } else if (EndsWith(psFileName, ".img"))
    {
        return "HFA";
    }
    else
        throw std::runtime_error("Unhandled raster format without a GDAL driver specification.");
}

bool EndsWith(const char * psFullString, const char * psEnding)
{
    std::string sFullString(psFullString);
    std::string sEnding(psEnding);

    // Ensure both strings are lower case
    std::transform(sFullString.begin(), sFullString.end(), sFullString.begin(), ::tolower);
    std::transform(sEnding.begin(), sEnding.end(), sEnding.begin(), ::tolower);

    if (sFullString.length() >= sEnding.length()) {
        return (0 == sFullString.compare (sFullString.length() - sEnding.length(), sEnding.length(), sEnding));
    } else {
        return false;
    }
}

extern "C" RM_DLL_API Raster_SymbologyStyle GetSymbologyStyleFromString(const char * psStyle)
{
    QString sStyle(psStyle);

    if (QString::compare(sStyle , "DEM", Qt::CaseInsensitive) == 0)
        return GSS_DEM;
    else if (QString::compare(sStyle , "DoD", Qt::CaseInsensitive) == 0)
        return GSS_DoD;
    else if (QString::compare(sStyle , "Error", Qt::CaseInsensitive) == 0)
        return GSS_Error;
    else if (QString::compare(sStyle , "HillShade", Qt::CaseInsensitive) == 0)
        return GSS_Hlsd;
    else if (QString::compare(sStyle , "PointDensity", Qt::CaseInsensitive) == 0)
        return GSS_PtDens;
    else if (QString::compare(sStyle , "SlopeDeg", Qt::CaseInsensitive) == 0)
        return GSS_SlopeDeg;
    else if (QString::compare(sStyle , "SlopePC", Qt::CaseInsensitive) == 0)
        return GSS_SlopePer;
    else
        return GSS_Unknown;
}

extern "C" RM_DLL_API void GetReturnCodeAsString(unsigned int eErrorCode, char * sErr, unsigned int iBufferSize)
{
    const char * pHabErr = RasterManagerException::GetReturnCodeOnlyAsString(eErrorCode);
    strncpy(sErr, pHabErr, iBufferSize);
    sErr[ iBufferSize - 1 ] = 0;
}

void printLine(QString theString)
{
    std::string sString = theString.toStdString();
    std::cout << "\n" << sString;
}




} // namespace
