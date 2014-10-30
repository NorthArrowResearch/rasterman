#define MY_DLL_EXPORT

#include "rastermanager_interface.h"
#include "extentrectangle.h"

#include "extentrectangle.h"
#include "rastermeta.h"

#include "raster.h"
#include "gdal_priv.h"

#include <limits>
#include <math.h>
#include <string>

#include <algorithm>
#include <iostream>

namespace RasterManager {

DLL_API GDALDataset * CreateOutputDS(const char * pOutputRaster,
                         GDALDataType eDataType,
                         bool bHasNoData,
                         double fNoDataValue,
                         int nCols, int nRows, double * newTransform, const char * projectionRef){

    RasterMeta pInputMeta(newTransform[3], newTransform[0], nRows, nCols, newTransform[1], newTransform[5], fNoDataValue, NULL, eDataType, projectionRef);

   return CreateOutputDS(pOutputRaster, &pInputMeta);
}

DLL_API GDALDataset * CreateOutputDS(const char * pOutputRaster, RasterMeta * pTemplateRastermeta){

    /* Create the new dataset. Determine the driver from the output file extension.
     * Enforce LZW compression for TIFs. The predictor 3 is used for floating point prediction.
     * Not using this value defaults the LZW to prediction to 1 which causes striping.
     */
    char **papszOptions = NULL;
    GDALDriver * pDR = NULL;

    if (pTemplateRastermeta->GetGDALDriver() == NULL){
        pDR = GetGDALDriverManager()->GetDriverByName(GetDriverFromFileName(pOutputRaster));
    }
    else {
        pDR = GetGDALDriverManager()->GetDriverByName(pTemplateRastermeta->GetGDALDriver());
    }

    if (strcmp( pDR->GetDescription() , "GTiff") == 0){
        papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "LZW");
    }

    GDALDataset * pDSOutput =  pDR->Create(pOutputRaster,
                                          pTemplateRastermeta->GetCols(),
                                          pTemplateRastermeta->GetRows(),
                                          1,
                                          pTemplateRastermeta->GetGDALDataType(),
                                          papszOptions);
    if (pDSOutput == NULL)
        return NULL;

    if (pTemplateRastermeta->GetNoDataValue() != NULL)
    {
        CPLErr er = pDSOutput->GetRasterBand(1)->SetNoDataValue(pTemplateRastermeta->GetNoDataValue());
        if (er == CE_Failure || er == CE_Fatal)
            return NULL;
    }

    double * newTransform = pTemplateRastermeta->GetGeoTransform();
    char * projectionRef = pTemplateRastermeta->GetProjectionRef();

    if (newTransform != NULL)
        pDSOutput->SetGeoTransform(newTransform);
    if (projectionRef != NULL)
        pDSOutput->SetProjection(projectionRef);

    return pDSOutput;

}

DLL_API GDALDataset * CreateOutputDSfromRef(const char * pOutputRaster,
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


extern "C" DLL_API int BasicMath(const char * psRaster1,
                                 const char * psRaster2,
                                 const double dOperator,
                                 const int iOperation,
                                 const char * psOutput)
{

    if (iOperation == NULL)
        return NO_OPERATION_SPECIFIED;

    // Everything except square root needs at least one other parameter (raster or doube)
    if (psRaster2 == NULL && dOperator == NULL && iOperation != RM_BASIC_MATH_SQRT)
        return MISSING_ARGUMENT;

    /*****************************************************************************************
     * Raster 1
     */
    if (psRaster1 == NULL)
        return INPUT_FILE_ERROR;

    RasterMeta rmRasterMeta1(psRaster1);

    GDALDataset * pDS1 = (GDALDataset*) GDALOpen(psRaster1, GA_ReadOnly);
    if (pDS1 == NULL)
        return INPUT_FILE_ERROR;

    GDALRasterBand * pRBInput1 = pDS1->GetRasterBand(1);

    double * pInputLine1 = (double *) CPLMalloc(sizeof(double)*rmRasterMeta1.GetCols());

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */
    RasterMeta rmOutputMeta;
    rmOutputMeta = rmRasterMeta1;

    double fNoDataValue;
    if (rmRasterMeta1.GetNoDataValue() == NULL){
        fNoDataValue = (double) std::numeric_limits<float>::min();
    }
    else {
        fNoDataValue = rmRasterMeta1.GetNoDataValue();
    }

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, &rmRasterMeta1);

    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*rmOutputMeta.GetCols());

    /*****************************************************************************************
     * Raster 2 to be used
     */
    if (psRaster2 != NULL){

        GDALDataset * pDS2 = (GDALDataset*) GDALOpen(psRaster2, GA_ReadOnly);
        if (pDS1 == NULL)
            return INPUT_FILE_ERROR;

        RasterMeta rmRasterMeta2(psRaster2);

        GDALRasterBand * pRBInput2 = pDS2->GetRasterBand(1);

        /*****************************************************************************************
        /* Check that input rasters have the same numbers of rows and columns
         */

        if (pRBInput1->GetXSize() != pRBInput2->GetXSize())
            return COLS_ERROR;

        if (pRBInput1->GetYSize() != pRBInput2->GetYSize())
            return ROWS_ERROR;

        if (psOutput == NULL)
            return OUTPUT_FILE_MISSING;

        double * pInputLine2 = (double *) CPLMalloc(sizeof(double)*rmRasterMeta2.GetCols());

        int i, j;
        for (i = 0; i < rmOutputMeta.GetRows(); i++)
        {
            pRBInput1->RasterIO(GF_Read, 0,  i, rmRasterMeta1.GetCols(), 1, pInputLine1, rmRasterMeta1.GetCols(), 1, GDT_Float64, 0, 0);
            pRBInput2->RasterIO(GF_Read, 0,  i, rmRasterMeta2.GetCols(), 1, pInputLine2, rmRasterMeta2.GetCols(), 1, GDT_Float64, 0, 0);

            for (j = 0; j < rmOutputMeta.GetCols(); j++)
            {
                if ( (pInputLine1[j] == rmRasterMeta1.GetNoDataValue()) ||
                     (pInputLine2[j] == rmRasterMeta1.GetNoDataValue()) )
                {
                    pOutputLine[j] = rmOutputMeta.GetNoDataValue();
                }
                else
                {
                    if (iOperation == RM_BASIC_MATH_ADD)
                        pOutputLine[j] = pInputLine1[j] + pInputLine2[j];
                    else if (iOperation == RM_BASIC_MATH_SUBTRACT)
                        pOutputLine[j] = pInputLine1[j] - pInputLine2[j];
                    else if (iOperation == RM_BASIC_MATH_MULTIPLY)
                        pOutputLine[j] = pInputLine1[j] * pInputLine2[j];
                    else if (iOperation == RM_BASIC_MATH_DIVIDE){
                        // Remember to cover the divide by zero case
                        if (psRaster2 != 0)
                            pOutputLine[j] = pInputLine1[j] / pInputLine2[j];
                        else
                            pOutputLine[j] = fNoDataValue;
                    }
                    else if (iOperation == RM_BASIC_MATH_THRESHOLD_PROP_ERROR){
                        if (abs(pInputLine1[j]) > pInputLine2[j]){
                            pOutputLine[j] = pInputLine1[j];
                        }
                        else{
                            pOutputLine[j] = fNoDataValue;
                        }
                    }
                    else
                        return MISSING_ARGUMENT;
                }
            }

            pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, rmOutputMeta.GetCols(), 1, pOutputLine, rmOutputMeta.GetCols(), 1, GDT_Float64, 0, 0);
        }
        CPLFree(pInputLine2);
        GDALClose(pDS2);

    }
    else if (dOperator != NULL || iOperation == RM_BASIC_MATH_SQRT){
        /*****************************************************************************************
        * Numerical Value to be used
        */

        int i, j;
        for (i = 0; i < rmOutputMeta.GetRows(); i++)
        {
            pRBInput1->RasterIO(GF_Read, 0,  i, rmRasterMeta1.GetCols(), 1, pInputLine1, rmRasterMeta1.GetCols(), 1, GDT_Float64, 0, 0);

            for (j = 0; j < pRBInput1->GetXSize(); j++)
            {
                if ( (pInputLine1[j] == fNoDataValue))
                {
                    pOutputLine[j] = fNoDataValue;
                }
                else
                {
                    if (iOperation == RM_BASIC_MATH_ADD)
                        pOutputLine[j] = pInputLine1[j] + dOperator;

                    else if (iOperation == RM_BASIC_MATH_SUBTRACT)
                        pOutputLine[j] = pInputLine1[j] - dOperator;

                    else if (iOperation == RM_BASIC_MATH_MULTIPLY)
                        pOutputLine[j] = pInputLine1[j] * dOperator;

                    else if (iOperation == RM_BASIC_MATH_DIVIDE){
                        // Remember to cover the divide by zero case
                        if(dOperator != 0)
                            pOutputLine[j] = pInputLine1[j] / dOperator;
                        else
                            pOutputLine[j] = (float) fNoDataValue;
                    }
                    else if (iOperation == RM_BASIC_MATH_POWER){
                        // We're throwing away imaginary numbers
                        if (dOperator >= 0)
                            pOutputLine[j] = pow(pInputLine1[j], dOperator);
                        else
                            pOutputLine[j] = (float) fNoDataValue;
                    }
                    else if (iOperation == RM_BASIC_MATH_SQRT){
                        // Throw away imaginary numbers
                        if (pInputLine1[j] >= 0)
                            pOutputLine[j] = sqrt(pInputLine1[j]);
                        else
                            pOutputLine[j] = (float) fNoDataValue;
                    }
                    else
                        return MISSING_ARGUMENT;
                }
            }
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, rmOutputMeta.GetCols(), 1, pOutputLine,
                                                  rmOutputMeta.GetCols(), 1,
                                                  GDT_Float64, 0, 0);
        }
    }
    CPLFree(pInputLine1);
    CPLFree(pOutputLine);

    GDALClose(pDS1);
    GDALClose(pDSOutput);

    PrintRasterProperties(psOutput);

    return PROCESS_OK;
}

extern "C" DLL_API int RootSumSquares(const char * psRaster1, const char * psRaster2, const char * psOutput)
{
    /*****************************************************************************************
     * Raster 1
     */
    if (psRaster1 == NULL)
        return INPUT_FILE_ERROR;

    GDALDataset * pDS1 = (GDALDataset*) GDALOpen(psRaster1, GA_ReadOnly);
    if (pDS1 == NULL)
        return INPUT_FILE_ERROR;

    GDALRasterBand * pRBInput1 = pDS1->GetRasterBand(1);
    int nHasNoData1 = 0;
    double fNoDataValue1 = pRBInput1->GetNoDataValue(&nHasNoData1);

    /*****************************************************************************************
     * Raster 2
     */
    if (psRaster2 == NULL)
        return INPUT_FILE_ERROR;

    GDALDataset * pDS2 = (GDALDataset*) GDALOpen(psRaster2, GA_ReadOnly);
    if (pDS1 == NULL)
        return INPUT_FILE_ERROR;

    GDALRasterBand * pRBInput2 = pDS2->GetRasterBand(1);
    int nHasNoData2 = 0;
    double fNoDataValue2 = pRBInput2->GetNoDataValue(&nHasNoData2);

    /*****************************************************************************************
    /* Check that input rasters have the same numbers of rows and columns
     */
    if (pRBInput1->GetXSize() != pRBInput2->GetXSize())
        return COLS_ERROR;

    if (pRBInput1->GetYSize() != pRBInput2->GetYSize())
        return ROWS_ERROR;

    if (psOutput == NULL)
        return OUTPUT_FILE_MISSING;

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */
    float fNoDataValue = (float) std::numeric_limits<float>::min();

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDSfromRef(psOutput, GDT_Float32, true, fNoDataValue, pDS1);


    /*****************************************************************************************
     * Allocate the memory for the input / output lines
     */
    float * pInputLine1 = (float *) CPLMalloc(sizeof(float)*pRBInput1->GetXSize());
    float * pInputLine2 = (float *) CPLMalloc(sizeof(float)*pRBInput2->GetXSize());
    float * pOutputLine = (float *) CPLMalloc(sizeof(float)*pDSOutput->GetRasterBand(1)->GetXSize());

    int i, j;
    for (i = 0; i < pRBInput1->GetYSize(); i++)
    {
        pRBInput1->RasterIO(GF_Read, 0,  i, pRBInput1->GetXSize(), 1, pInputLine1, pRBInput1->GetXSize(), 1, GDT_Float32, 0, 0);
        pRBInput2->RasterIO(GF_Read, 0,  i, pRBInput2->GetXSize(), 1, pInputLine2, pRBInput2->GetXSize(), 1, GDT_Float32, 0, 0);

        for (j = 0; j < pRBInput1->GetXSize(); j++)
        {
            if ( (nHasNoData1 != 0 && pInputLine1[j] == (float) fNoDataValue1) ||
                 (nHasNoData2 != 0 && pInputLine2[j] == fNoDataValue2) )
            {
                pOutputLine[j] = (float) fNoDataValue;
            }
            else
                pOutputLine[j] = sqrt( pow(pInputLine1[j], 2) + pow(pInputLine2[j], 2) );
        }

        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, pRBInput2->GetXSize(), 1, pOutputLine, pDSOutput->GetRasterBand(1)->GetXSize(), 1, GDT_Float32, 0, 0);
    }

    CPLFree(pInputLine1);
    CPLFree(pInputLine2);
    CPLFree(pOutputLine);

    GDALClose(pDS1);
    GDALClose(pDS2);
    GDALClose(pDSOutput);

    return PROCESS_OK;
}


extern "C" DLL_API int Mosaic(const char * csRasters, const char * psOutput)
{
    // Loop through the strings, delimited by ;
    std::string RasterFileName, RasterFilesToken(csRasters);

    // The output raster info
    RasterMeta OutputMeta;


    /*****************************************************************************************
     * Open all the relevant files and figure out the bounds of the final file.
     */
    int counter = 0;
    while(RasterFilesToken != ""){
        counter++;
        RasterFileName = RasterFilesToken.substr(0,RasterFilesToken.find_first_of(";"));
        RasterFilesToken = RasterFilesToken.substr(RasterFilesToken.find_first_of(";") + 1);

        if (RasterFileName.c_str() == NULL)
            return INPUT_FILE_ERROR;

        RasterMeta erRasterInput (RasterFileName.c_str());

        // First time round set the bounds to the first raster we give it.
        if (counter==1){
            OutputMeta = erRasterInput;
        }
        else{
            OutputMeta.Union(&erRasterInput);
        }
    }


    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, &OutputMeta);
    double fNoDataValue = OutputMeta.GetNoDataValue();

    //projectionRef use from inputs.

    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*OutputMeta.GetCols());

    /*****************************************************************************************
     * Loop over the output file to make sure every cell gets a value of fNoDataValue
     * Every line is the same so we can have the for loops adjacent
     */
    for (int outj = 0; outj < OutputMeta.GetCols(); outj++){
        pOutputLine[outj] = fNoDataValue;
    }
    for (int outi = 0; outi < OutputMeta.GetRows(); outi++){
        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  outi, OutputMeta.GetCols(), 1, pOutputLine, OutputMeta.GetCols(), 1, GDT_Float64, 0, 0);
    }

    /*****************************************************************************************
     * Loop over the inputs and then the rows and columns
     *
     */
    int i,j;
    std::string sRasterFiles(csRasters);
    RasterFileName = "";

    while(sRasterFiles != ""){
        RasterFileName = sRasterFiles.substr(0,sRasterFiles.find_first_of(";"));
        sRasterFiles = sRasterFiles.substr(sRasterFiles.find_first_of(";") + 1);

        GDALDataset * pDS = (GDALDataset*) GDALOpen(RasterFileName.c_str(), GA_ReadOnly);
        GDALRasterBand * pRBInput = pDS->GetRasterBand(1);

        RasterMeta inputRect (RasterFileName.c_str());

        // We need to figure out where in the output the input lives.
        int trans_i = OutputMeta.GetRowTranslation(&inputRect);
        int trans_j = OutputMeta.GetColTranslation(&inputRect);

        double * pInputLine = (double *) CPLMalloc(sizeof(double)*pRBInput->GetXSize());

        for (i = 0; i < pRBInput->GetYSize(); i++){
            pRBInput->RasterIO(GF_Read, 0,  i, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
            // here's where we need to get the correct row of the output. Replace
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Read, 0,  trans_i+i, OutputMeta.GetCols(), 1, pOutputLine, OutputMeta.GetCols(), 1, GDT_Float64, 0, 0);

            for (j = 0; j < pRBInput->GetXSize(); j++){
                // If the input line is empty then do nothing
                if ( (pInputLine[j] != fNoDataValue)
                     && pOutputLine[trans_j+j] ==  fNoDataValue)
                {
                    pOutputLine[trans_j+j] = pInputLine[j];
                }
            }
            // here's where we need to get the correct row of the output. Replace
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  trans_i+i, OutputMeta.GetCols(), 1, pOutputLine, OutputMeta.GetCols(), 1, GDT_Float64, 0, 0);

        }
        CPLFree(pInputLine);

    }

    /*****************************************************************************************
     * Now Close everything and clean it all up
     */
    CPLFree(pOutputLine);
    GDALClose(pDSOutput);

    PrintRasterProperties(psOutput);

    return PROCESS_OK;
}


/*******************************************************************************************************
 *******************************************************************************************************
 * Raster property methods
 */

extern "C" DLL_API void GetRasterProperties(const char * ppszRaster,
                                                          double & fCellHeight, double & fCellWidth,
                                                          double & fLeft, double & fTop, int & nRows, int & nCols,
                                                          double & fNoData, int & bHasNoData, int & nDataType)
{
    RasterManager::Raster r(ppszRaster);
    fCellHeight = r.GetCellHeight();
    fCellWidth = r.GetCellWidth();
    fLeft = r.GetLeft();
    fTop = r.GetTop();
    nRows = r.GetRows();
    nCols = r.GetCols();
    fNoData = r.GetNoDataValue();
    bHasNoData = (int) r.HasNoDataValue();
    nDataType = (int) r.GetGDALDataType();
}


extern "C" DLL_API void PrintRasterProperties(const char * ppszRaster)
{
    double fCellWidth = 0;
    double fCellHeight = 0;
    double fLeft = 0;
    double fTop = 0;
    int nRows = 0;
    int nCols = 0;
    double fNoData = 0;
    int bHasNoData = 0;
    int nDataType;

    RasterManager::Raster r(ppszRaster);
    fCellHeight = r.GetCellHeight();
    fCellWidth = r.GetCellWidth();
    fLeft = r.GetLeft();
    fTop = r.GetTop();
    nRows = r.GetRows();
    nCols = r.GetCols();
    fNoData = r.GetNoDataValue();
    bHasNoData = (int) r.HasNoDataValue();
    nDataType = (int) r.GetGDALDataType();

    std::cout << "\n    Raster: " << ppszRaster;
    std::printf( "\n      Left: %.8lf", fLeft);
    std::printf( "\n      Top: %.8lf", fTop);
    //std::cout << "\n       Top: " << fTop;
    std::cout << "\n      Rows: " << nRows;
    std::cout << "\n      Cols: " << nCols;

    std::cout << "\n Data Type: ";

    switch (nDataType)
    {
    // Note 0 = unknown;
    case  1: std::cout << "1, GDT_Byte, Eight bit unsigned integer"; break;
    case  2: std::cout << "2, GDT_UInt16, Sixteen bit unsigned integer"; break;
    case  3: std::cout << "3, GDT_Int16, Sixteen bit signed integer"; break;
    case  4: std::cout << "4, GDT_UInt32, Thirty two bit unsigned integer"; break;
    case  5: std::cout << "5, GDT_Int32, Thirty two bit signed integer"; break;
    case  6: std::cout << "6, GDT_Float32, Thirty two bit floating point"; break;
    case  7: std::cout << "7, GDT_Float64, Sixty four bit floating point"; break;
    case  8: std::cout << "8, GDT_CInt16, Complex Int16"; break;
    case  9: std::cout << "9, GDT_CInt32, Complex Int32"; break;
    case 10: std::cout << "10, GDT_CFloat32, Complex Float32"; break;
    case 11: std::cout << "11, GDT_CFloat64, Complex Float64"; break;
    default: std::cout << "Unknown"; break;
    }
    if (bHasNoData)
        std::cout << "\n   No Data: " << fNoData;
    else
        std::cout << "\n   No Data: none";
}


/*******************************************************************************************************
 *******************************************************************************************************
 * Raster copy and resample methods
 */

extern "C" DLL_API int Copy(const char * ppszOriginalRaster,
                                          const char *ppszOutputRaster,
                                          double fNewCellSize,
                                          double fLeft, double fTop, int nRows, int nCols)
{
    RasterManager::Raster ra(ppszOriginalRaster);
    return ra.Copy(ppszOutputRaster, fNewCellSize, fLeft, fTop, nRows, nCols);
}

extern "C" DLL_API int BiLinearResample(const char * ppszOriginalRaster,
                                                      const char *ppszOutputRaster,
                                                      double fNewCellSize,
                                                      double fLeft, double fTop, int nRows, int nCols)
{
    RasterManager::Raster ra(ppszOriginalRaster);
    return ra.ReSample(ppszOutputRaster, fNewCellSize, fLeft, fTop, nRows, nCols);
}

extern "C" DLL_API const char * ExtractFileExt(const char * FileName)
{
    std::string s = FileName;
    int Len = s.length();
    while(TRUE)
    {
        if(FileName[Len] != '.')
            Len--;
        else
        {
            char *Ext = new char[s.length()-Len+1];
            for(unsigned int a=0; a < s.length()-Len; a++)
                Ext[a] = FileName[s.length()-(s.length()-Len)+a];
            Ext[s.length()-Len] = '\0';
            return Ext;
        }
    }
}

extern "C" DLL_API const char * GetDriverFromFilename(const char * FileName)
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

extern "C" DLL_API const char * GetDriverFromFileName(const char * psFileName)
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

extern "C" DLL_API const char * GetReturnCodeAsString(int eErrorCode)
{
    switch (eErrorCode)
    {
    case PROCESS_OK:
        return "process completed successfully.";

    case INPUT_FILE_ERROR:
        return "input file error.";

    case INPUT_FILE_TRANSFORM_ERROR:
        return "input raster map projection error.";

    case OUTPUT_FILE_MISSING:
        return "input raster file is missing or cannot be found.";

    case  OUTPUT_FILE_ERROR:
        return "output file error";

    case  OUTPUT_NO_DATA_ERROR:
        return "NoData error on output raster";

    case  OUTPUT_FILE_EXT_ERROR:
        return "Output raster file extension error.";

    case OUTPUT_UNHANDLED_DRIVER:
        return "Unhandled output raster type.";

    case CELL_SIZE_ERROR:
        return "Cell size error.";

    case LEFT_ERROR:
        return "Invalid raster left coordinate.";

    case TOP_ERROR:
        return "Invalid raster top coordinate.";

    case ROWS_ERROR:
        return "Invalid raster number of rows.";

    case COLS_ERROR:
        return "Invalid raster number of columns.";

    case NO_OPERATION_SPECIFIED:
        return "No operation specified.";

    case MISSING_ARGUMENT:
        return "Missing argument";

    case OTHER_ERROR:
        return "Unspecified error.";

    default:
        throw "Unhandled Raster Manager return code.";
    }

}

} // namespace
