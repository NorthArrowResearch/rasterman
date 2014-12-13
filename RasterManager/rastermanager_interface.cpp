#define MY_DLL_EXPORT

#include "rastermanager_interface.h"
#include "extentrectangle.h"

#include <stdio.h>
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

    RasterMeta pInputMeta(newTransform[3], newTransform[0], nRows, nCols, newTransform[5], newTransform[1], fNoDataValue, NULL, eDataType, projectionRef);

   return CreateOutputDS(pOutputRaster, &pInputMeta);
}

DLL_API GDALDataset * CreateOutputDS(const char * pOutputRaster, RasterMeta * pTemplateRastermeta){

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
                                          pTemplateRastermeta->GetGDALDataType(),
                                          papszOptions);

    CSLDestroy( papszOptions );

    if (pDSOutput == NULL)
        return NULL;

    if (pTemplateRastermeta->GetNoDataValue() != NULL)
    {
        CPLErr er = pDSOutput->GetRasterBand(1)->SetNoDataValue(pTemplateRastermeta->GetNoDataValue());
        if (er == CE_Failure || er == CE_Fatal)
            return NULL;
    }
    else{
        CPLErr er = pDSOutput->GetRasterBand(1)->SetNoDataValue(0);
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


extern "C" DLL_API void RegisterGDAL() { GDALAllRegister();}
extern "C" DLL_API void DestroyGDAL() { GDALDestroyDriverManager();}

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
        fNoDataValue = (double) std::numeric_limits<float>::lowest();
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
         * Check that input rasters have the same numbers of rows and columns
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
                if (pInputLine1[j] == fNoDataValue)
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
                            pOutputLine[j] = fNoDataValue;
                    }
                    else if (iOperation == RM_BASIC_MATH_POWER){
                        // We're throwing away imaginary numbers
                        if (dOperator >= 0)
                            pOutputLine[j] = pow(pInputLine1[j], dOperator);
                        else
                            pOutputLine[j] = fNoDataValue;
                    }
                    else if (iOperation == RM_BASIC_MATH_SQRT){
                        // Throw away imaginary numbers
                        if (pInputLine1[j] >= 0)
                            pOutputLine[j] = sqrt(pInputLine1[j]);
                        else
                            pOutputLine[j] = fNoDataValue;
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

    Raster::CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDS1);
    GDALClose(pDSOutput);

    PrintRasterProperties(psOutput);

    return PROCESS_OK;
}

extern "C" DLL_API int CreateHillshade(const char * psInputRaster, const char * psOutputHillshade){

    Raster pDemRaster (psInputRaster);
    pDemRaster.Hillshade(psOutputHillshade);

    return PROCESS_OK;
}

extern "C" DLL_API int CreateSlope(const char * psInputRaster, const char * psOutputSlope, int nSlopeType){

    Raster pDemRaster (psInputRaster);
    pDemRaster.Slope(psOutputSlope, nSlopeType);

    return PROCESS_OK;
}

extern "C" DLL_API int Mask(const char * psInputRaster, const char * psMaskRaster, const char * psOutput)
{


    // Everything except square root needs at least one other parameter (raster or doube)
    if (psMaskRaster == NULL || psInputRaster == NULL || psOutput == NULL)
        return MISSING_ARGUMENT;

    /*****************************************************************************************
     * Raster 1
     */
    if (psInputRaster == NULL)
        return INPUT_FILE_ERROR;

    RasterMeta rmInputMeta(psInputRaster);

    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(psInputRaster, GA_ReadOnly);
    if (pDSInput == NULL)
        return INPUT_FILE_ERROR;

    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    double * pInputLine = (double *) CPLMalloc(sizeof(double)*rmInputMeta.GetCols());

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */
    RasterMeta rmOutputMeta;
    rmOutputMeta = rmInputMeta;

    double fNoDataValue;
    if (rmInputMeta.GetNoDataValue() == NULL){
        fNoDataValue = (double) std::numeric_limits<float>::lowest();
    }
    else {
        fNoDataValue = rmInputMeta.GetNoDataValue();
    }

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, &rmInputMeta);

    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*rmOutputMeta.GetCols());

    /*****************************************************************************************
     * The Mask Raster to be used: psMaskRaster
     */
    GDALDataset * pDSMask = (GDALDataset*) GDALOpen(psMaskRaster, GA_ReadOnly);
    if (pDSInput == NULL)
        return INPUT_FILE_ERROR;

    GDALRasterBand * pRBMask = pDSMask->GetRasterBand(1);

    RasterMeta rmMaskMeta(psMaskRaster);

    /*****************************************************************************************
     * Check that input rasters have the same numbers of rows and columns
     */

    if (pRBInput->GetXSize() != pRBMask->GetXSize())
        return COLS_ERROR;

    if (pRBInput->GetYSize() != pRBMask->GetYSize())
        return ROWS_ERROR;

    if (psOutput == NULL)
        return OUTPUT_FILE_MISSING;

    double * pMaskline = (double *) CPLMalloc(sizeof(double)*rmMaskMeta.GetCols());

    int i, j;
    for (i = 0; i < rmOutputMeta.GetRows(); i++)
    {
        pRBInput->RasterIO(GF_Read, 0,  i, rmInputMeta.GetCols(), 1, pInputLine, rmInputMeta.GetCols(), 1, GDT_Float64, 0, 0);
        pRBMask->RasterIO(GF_Read, 0,  i, rmMaskMeta.GetCols(), 1, pMaskline, rmMaskMeta.GetCols(), 1, GDT_Float64, 0, 0);

        for (j = 0; j < rmOutputMeta.GetCols(); j++)
        {
            if (pMaskline[j] == rmMaskMeta.GetNoDataValue())
            {
                pOutputLine[j] = rmOutputMeta.GetNoDataValue();
            }
            else
            {
                pOutputLine[j] = pInputLine[j];
            }
        }

        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, rmOutputMeta.GetCols(), 1, pOutputLine, rmOutputMeta.GetCols(), 1, GDT_Float64, 0, 0);
    }
    CPLFree(pMaskline);
    CPLFree(pInputLine);
    CPLFree(pOutputLine);

    Raster::CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDSInput);
    GDALClose(pDSOutput);
    GDALClose(pDSMask);

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
     * Check that input rasters have the same numbers of rows and columns
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
    float fNoDataValue = (float) std::numeric_limits<float>::lowest();

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDSfromRef(psOutput, GDT_Float32, true, fNoDataValue, pDS1);


    /*****************************************************************************************
     * Allocate the memory for the input / output lines
     */
    double * pInputLine1 = (double *) CPLMalloc(sizeof(double)*pRBInput1->GetXSize());
    double * pInputLine2 = (double *) CPLMalloc(sizeof(double)*pRBInput2->GetXSize());
    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*pDSOutput->GetRasterBand(1)->GetXSize());

    int i, j;
    for (i = 0; i < pRBInput1->GetYSize(); i++)
    {
        pRBInput1->RasterIO(GF_Read, 0,  i, pRBInput1->GetXSize(), 1, pInputLine1, pRBInput1->GetXSize(), 1, GDT_Float64, 0, 0);
        pRBInput2->RasterIO(GF_Read, 0,  i, pRBInput2->GetXSize(), 1, pInputLine2, pRBInput2->GetXSize(), 1, GDT_Float64, 0, 0);

        for (j = 0; j < pRBInput1->GetXSize(); j++)
        {
            if ( (nHasNoData1 != 0 && pInputLine1[j] == fNoDataValue1) ||
                 (nHasNoData2 != 0 && pInputLine2[j] == fNoDataValue2) )
            {
                pOutputLine[j] = fNoDataValue;
            }
            else
                pOutputLine[j] = sqrt( pow(pInputLine1[j], 2) + pow(pInputLine2[j], 2) );
        }

        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, pRBInput2->GetXSize(), 1, pOutputLine, pDSOutput->GetRasterBand(1)->GetXSize(), 1, GDT_Float64, 0, 0);
    }

    CPLFree(pInputLine1);
    CPLFree(pInputLine2);
    CPLFree(pOutputLine);

    Raster::CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDS1);
    GDALClose(pDS2);
    GDALClose(pDSOutput);



    return PROCESS_OK;
}


extern "C" DLL_API int Mosaic(const char * csRasters, const char * psOutput)
{
    // Loop through the strings, delimited by ;
    std::string RasterFileName, RasterFiles(csRasters), RasterFilesToken;

    // Terminate with a semicolon if it hasn't already been done
    if (!EndsWith(psOutput, ";")){
        RasterFiles.append(";");
    }
    RasterFilesToken = RasterFiles;

    // The output raster info
    RasterMeta OutputMeta;

    double fNoDataValue = (double) std::numeric_limits<float>::lowest();

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
            OutputMeta.SetNoDataValue(fNoDataValue);
            OutputMeta.SetGDALDataType(GDT_Float32);
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

    //projectionRef use from inputs.

    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*OutputMeta.GetCols());

    /*****************************************************************************************
     * Loop over the inputs and then the rows and columns
     *
     */
    int i,j;
    RasterFilesToken = RasterFiles;
    RasterFileName = "";

    while(RasterFilesToken != ""){
        RasterFileName = RasterFilesToken.substr(0,RasterFilesToken.find_first_of(";"));
        RasterFilesToken = RasterFilesToken.substr(RasterFilesToken.find_first_of(";") + 1);

        GDALDataset * pDS = (GDALDataset*) GDALOpen(RasterFileName.c_str(), GA_ReadOnly);
        GDALRasterBand * pRBInput = pDS->GetRasterBand(1);

        RasterMeta inputMeta (RasterFileName.c_str());

        // We need to figure out where in the output the input lives.
        int trans_i = OutputMeta.GetRowTranslation(&inputMeta);
        int trans_j = OutputMeta.GetColTranslation(&inputMeta);

        double * pInputLine = (double *) CPLMalloc(sizeof(double)*pRBInput->GetXSize());

        for (i = 0; i < pRBInput->GetYSize(); i++){
            pRBInput->RasterIO(GF_Read, 0,  i, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
            // here's where we need to get the correct row of the output. Replace
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Read, 0,  trans_i+i, OutputMeta.GetCols(), 1, pOutputLine, OutputMeta.GetCols(), 1, GDT_Float64, 0, 0);

            for (j = 0; j < pRBInput->GetXSize(); j++){
                // If the input line is empty then do nothing
                if ( (pInputLine[j] != inputMeta.GetNoDataValue())
                     && pOutputLine[trans_j+j] ==  OutputMeta.GetNoDataValue())
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
    Raster::CalculateStats(pDSOutput->GetRasterBand(1));
    GDALClose(pDSOutput);

    PrintRasterProperties(psOutput);

    return PROCESS_OK;
}

extern "C" DLL_API int IsConcurrent(const char * csRaster1, const char * csRaster2){

    RasterManager::RasterMeta rmRaster1(csRaster1);
    RasterManager::RasterMeta rmRaster2(csRaster2);

    return rmRaster1.IsConcurrent(&rmRaster2);
}

extern "C" DLL_API int MakeConcurrent(const char * csRasters, const char * csRasterOutputs)
{
    // Loop through the strings, delimited by ;
    std::string sInPutFileName,
            sOutputFileName,
            sRasterInputFiles(csRasters), sRasterInputTokens,
            sRasterOutputFiles(csRasterOutputs), sRasterOutputTokens;

    // Terminate with a semicolon if it hasn't already been done
    if (!EndsWith(csRasters, ";")){
        sRasterInputFiles.append(";");
    }

    sRasterInputTokens = sRasterInputFiles;
    if (!EndsWith(csRasterOutputs, ";")){
        sRasterOutputFiles.append(";");
    }

    sRasterOutputTokens = sRasterOutputFiles;
    // The Master meta is the one we will use to output all the raster files
    // It will be the boolean intersect of all
    RasterMeta MasterMeta;

    double dNoDataValue = (double) std::numeric_limits<float>::lowest();

    /*****************************************************************************************
     * Open all the relevant files and figure out the bounds of the final file.
     */
    int counter = 0;
    while(sRasterInputTokens != ""){
        counter++;
        sInPutFileName = sRasterInputTokens.substr(0,sRasterInputTokens.find_first_of(";"));
        sRasterInputTokens = sRasterInputTokens.substr(sRasterInputTokens.find_first_of(";") + 1);

        sOutputFileName = sRasterOutputTokens.substr(0,sRasterOutputTokens.find_first_of(";"));
        sRasterOutputTokens = sRasterOutputTokens.substr(sRasterOutputTokens.find_first_of(";") + 1);

        if (sInPutFileName.c_str() == NULL)
            return INPUT_FILE_ERROR;

        if (sOutputFileName == "")
            throw std::runtime_error("ERROR: Number of output filepaths does not match number of input filepaths.");

        RasterMeta erRasterInput (sInPutFileName.c_str());

        // First time round set the bounds to the first raster we give it.
        if (counter==1){
            MasterMeta = erRasterInput;
            MasterMeta.SetNoDataValue(dNoDataValue);
            MasterMeta.SetGDALDataType(GDT_Float32);
        }
        else{
            if (erRasterInput.IsOthogonal() == 0){
                throw std::runtime_error("ERROR: All rasters must be orthogonal.");
            }
            else if(erRasterInput.GetCellHeight() != MasterMeta.GetCellHeight()
                    || erRasterInput.GetCellWidth() != MasterMeta.GetCellWidth() ){
                throw std::runtime_error("ERROR: cell resolutions must be the same for all rasters");
            }
            else {
                MasterMeta.Union(&erRasterInput);
            }
        }
    }

    /*****************************************************************************************
     * Loop over the inputs and then the rows and columns
     */
    int i,j;
    sInPutFileName = "";
    sOutputFileName = "";
    sRasterInputTokens = sRasterInputFiles;
    sRasterOutputTokens = sRasterOutputFiles;

    while(sRasterInputTokens != ""){

        sInPutFileName = sRasterInputTokens.substr(0,sRasterInputTokens.find_first_of(";"));
        sRasterInputTokens = sRasterInputTokens.substr(sRasterInputTokens.find_first_of(";") + 1);

        sOutputFileName = sRasterOutputTokens.substr(0,sRasterOutputTokens.find_first_of(";"));
        sRasterOutputTokens = sRasterOutputTokens.substr(sRasterOutputTokens.find_first_of(";") + 1);

        // Create the output dataset for writing
        GDALDataset * pDSOutput = CreateOutputDS(sOutputFileName.c_str(), &MasterMeta);
        double * pOutputLine = (double *) CPLMalloc(sizeof(double)*MasterMeta.GetCols());

        GDALDataset * pDS = (GDALDataset*) GDALOpen(sInPutFileName.c_str(), GA_ReadOnly);
        GDALRasterBand * pRBInput = pDS->GetRasterBand(1);

        RasterMeta inputMeta (sInPutFileName.c_str());

        /*****************************************************************************************
         * Loop over the output file to make sure every cell gets a value of fNoDataValue
         * Every line is the same so we can have the for loops adjacent
         */
        for (int outj = 0; outj < MasterMeta.GetCols(); outj++){
            pOutputLine[outj] = dNoDataValue;
        }
        for (int outi = 0; outi < MasterMeta.GetRows(); outi++){
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  outi, MasterMeta.GetCols(), 1, pOutputLine, MasterMeta.GetCols(), 1, GDT_Float64, 0, 0);
        }

        // We need to figure out where in the output the input lives.
        int trans_i = MasterMeta.GetRowTranslation(&inputMeta);
        int trans_j = MasterMeta.GetColTranslation(&inputMeta);

        double * pInputLine = (double *) CPLMalloc(sizeof(double)*pRBInput->GetXSize());

        for (i = 0; i < pRBInput->GetYSize(); i++){
            if (i - trans_i <= MasterMeta.GetRows() ){
                pRBInput->RasterIO(GF_Read, 0,  i, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
                // here's where we need to get the correct row of the output. Replace
                pDSOutput->GetRasterBand(1)->RasterIO(GF_Read, 0,  i- trans_i, MasterMeta.GetCols(), 1, pOutputLine, MasterMeta.GetCols(), 1, GDT_Float64, 0, 0);

                for (j = 0; j < pRBInput->GetXSize(); j++){
                    // If the input line is empty then do nothing
                    if ( trans_j+j <= MasterMeta.GetCols()
                         && pInputLine[j] != inputMeta.GetNoDataValue()
                         && pOutputLine[trans_j+j] ==  MasterMeta.GetNoDataValue() )
                    {
                        pOutputLine[trans_j+j] = pInputLine[j];
                    }
                }
                // here's where we need to get the correct row of the output. Replace
                pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i - trans_i, MasterMeta.GetCols(), 1, pOutputLine, MasterMeta.GetCols(), 1, GDT_Float64, 0, 0);
            }

        }
        CPLFree(pOutputLine);
        CPLFree(pInputLine);

        Raster::CalculateStats(pDSOutput->GetRasterBand(1));

        GDALClose(pDSOutput);

        PrintRasterProperties(sOutputFileName.c_str());
    }

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
    nDataType = (int) r.GetGDALDataType();


    std::cout << "\n     Raster: " << ppszRaster;
    std::printf( "\n       Left: %.8lf      Right: %.8lf", fLeft, fRight);
    std::printf( "\n        Top: %.8lf     Bottom: %.8lf", fTop, fBottom);
    std::cout << "\n       Rows: " << nRows << "    Cols: " << nCols;
    std::cout << "\n      ";
    std::printf( "\n     Cell Width: %.1lf", fCellWidth);
    std::printf( "\n     Min: %.3lf     Max: %.3lf", dRasterMin, dRasterMax);
    if (orthogonal == 1 ){
        std::printf( "\n     Orthogonal: True");
    }
    else {
        std::printf( "\n     Orthogonal: False");
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

    std::cout << "\n      Projection: " << projection.substr(0,70) << "...";
    std::cout << "\n ";


}

extern "C" DLL_API int CreatePNG(const char * psInputRaster, const char * psOutputPNG, int nImageQuality, int nLongAxisPixels, int nOpacity, int eRasterType)
{
    RasterManager::Raster rOriginal(psInputRaster);
    int eResult = rOriginal.PNG(psOutputPNG, nImageQuality, nLongAxisPixels, nOpacity, (Raster_SymbologyStyle) eRasterType);
    return eResult;
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
    for (int i = strlen(FileName); i >= 0; i--) {
        if (FileName[i] == '.' ){
            return &FileName[i];
        }
    }
    return NULL;
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

    case RM_PNG_QUALITY:
        return "Invalid image quality.";

    case RM_PNG_TRANSPARENCY:
        return "Invalid PNG transparency.";

    case RM_PNG_LONG_AXIS:
            return "Invalid Long Axis specified.";

    default:
        std::string errMsg = "Unhandled Raster Manager return code: " + std::to_string((long long)eErrorCode);
        return errMsg.c_str();
    }

}

} // namespace
