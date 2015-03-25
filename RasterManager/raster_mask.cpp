#define MY_DLL_EXPORT
/*
 * Raster Mask Operation
 *
 * 6 December 2014
 *
*/
#include "rastermanager_exception.h"
#include "rastermanager_interface.h"
#include "raster.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "rastermanager.h"


namespace RasterManager {

int Raster::RasterMask(const char * psInputRaster, const char * psMaskRaster, const char * psOutput){

    // Everything except square root needs at least one other parameter (raster or doube)
    if (psMaskRaster == NULL || psInputRaster == NULL || psOutput == NULL)
        throw RasterManagerException( MISSING_ARGUMENT);

    // Input Validation
    CheckFile(psInputRaster, true);
    CheckFile(psMaskRaster, true);
    CheckFile(psOutput, false);


    /*****************************************************************************************
     * Raster 1
     */
    if (psInputRaster == NULL)
        throw RasterManagerException( INPUT_FILE_ERROR, "Input file was not specified");

    RasterMeta rmInputMeta(psInputRaster);

    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(psInputRaster, GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException( INPUT_FILE_ERROR, "Input file could not be opened");

    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    double * pInputLine = (double *) CPLMalloc(sizeof(double)*rmInputMeta.GetCols());

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */
    RasterMeta rmOutputMeta;
    rmOutputMeta = rmInputMeta;

    double fNoDataValue;
    if (!rmInputMeta.HasNoDataValue()){
        fNoDataValue = (double) -std::numeric_limits<float>::max();
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
            // If dMaskVal isn't used then we mask out any nodata values. Otherwise mask out anything not
            // equal to the dMaskVal
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

    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDSInput);
    GDALClose(pDSOutput);
    GDALClose(pDSMask);

    return PROCESS_OK;

}


int Raster::RasterMaskValue(const char * psInputRaster, const char * psOutput, double dMaskVal){

    // Everything except square root needs at least one other parameter (raster or doube)
    if ( psInputRaster == NULL || psOutput == NULL)
        throw RasterManagerException( MISSING_ARGUMENT );

    // Input Validation
    CheckFile(psInputRaster, true);
    CheckFile(psOutput, false);

    /*****************************************************************************************
     * Raster 1
     */
    if (psInputRaster == NULL)
        throw RasterManagerException( INPUT_FILE_ERROR, "Input raster not specified");

    RasterMeta rmInputMeta(psInputRaster);

    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(psInputRaster, GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException( INPUT_FILE_ERROR, "Input file could not be opened");

    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    double * pInputLine = (double *) CPLMalloc(sizeof(double)*rmInputMeta.GetCols());

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */
    RasterMeta rmOutputMeta;
    rmOutputMeta = rmInputMeta;

    double fNoDataValue;
    if (!rmInputMeta.HasNoDataValue()){
        fNoDataValue = (double) -std::numeric_limits<float>::max();
    }
    else {
        fNoDataValue = rmInputMeta.GetNoDataValue();
    }

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, &rmInputMeta);

    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*rmOutputMeta.GetCols());

    for (int i = 0; i < rmOutputMeta.GetRows(); i++)
    {
        pRBInput->RasterIO(GF_Read, 0,  i, rmInputMeta.GetCols(), 1, pInputLine, rmInputMeta.GetCols(), 1, GDT_Float64, 0, 0);

        for (int j = 0; j < rmOutputMeta.GetCols(); j++)
        {
            // Mask out anything not equal to the dMaskVal
            if ( isEqual(pInputLine[j], dMaskVal) || pInputLine[j] == fNoDataValue )
            {
                pOutputLine[j] = fNoDataValue;
            }
            else
            {
                pOutputLine[j] = pInputLine[j];
            }
        }

        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, rmOutputMeta.GetCols(), 1, pOutputLine, rmOutputMeta.GetCols(), 1, GDT_Float64, 0, 0);
    }
    CPLFree(pInputLine);
    CPLFree(pOutputLine);

    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDSInput);
    GDALClose(pDSOutput);

    return PROCESS_OK;

}

}
