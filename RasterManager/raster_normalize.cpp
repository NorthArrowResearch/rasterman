#define MY_DLL_EXPORT
/*
 * Raster Invert
 *
 * 28 February 2015
 *
*/
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "raster.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "rastermanager.h"


namespace RasterManager {

int Raster::NormalizeRaster(const char * psInputRaster,
                         const char * psOutputRaster){

    CheckFile(psInputRaster, true);
    CheckFile(psOutputRaster, false);
    bool bValidRaster = true;

    RasterMeta rmRasterMeta(psInputRaster);

    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(psInputRaster, GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException(INPUT_FILE_ERROR, "Could not open input Raster");


    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    // Calculate stats to make sure we have the latest values for max and min.
    CalculateStats(pDSInput->GetRasterBand(1));

    double dRMin, dRMax, dRMean, dRStdDev;
    pRBInput->GetStatistics( 0 , true, &dRMin, &dRMax, &dRMean, &dRStdDev );

    double * pInputLine = (double *) CPLMalloc(sizeof(double)*rmRasterMeta.GetCols());

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */
    RasterMeta rmOutputMeta;
    rmOutputMeta = rmRasterMeta;

    // Decision: We can't mix types here so the output will always be double
    GDALDataType outDataType = GDT_Float64;
    rmOutputMeta.SetGDALDataType(&outDataType);

    double fNoDataValue = (double) -std::numeric_limits<float>::max();
    rmOutputMeta.SetNoDataValue(&fNoDataValue);

    // Before we were throwing an exception but we need this to go through and not fail.
    if (dRMin == fNoDataValue || dRMax == fNoDataValue || dRMax == dRMin || dRMax < dRMin){
        bValidRaster = false;
        //throw RasterManagerException(INPUT_FILE_NOT_VALID, QString("The raster max or min value was invalid: Min: {0} Max: {1}").arg(dRMin).arg(dRMax) );
    }

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutputRaster, &rmRasterMeta);

    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*rmOutputMeta.GetCols());

    int i, j;
    for (i = 0; i < rmOutputMeta.GetRows(); i++)
    {
        pRBInput->RasterIO(GF_Read, 0,  i, rmRasterMeta.GetCols(), 1, pInputLine, rmRasterMeta.GetCols(), 1, GDT_Float64, 0, 0);

        for (j = 0; j < rmOutputMeta.GetCols(); j++)
        {
            if ( pInputLine[j] == rmRasterMeta.GetNoDataValue() || bValidRaster == false)
            {
                pOutputLine[j] = fNoDataValue;
            }
            else
            {
                pOutputLine[j] = (pInputLine[j] - dRMin) / (dRMax - dRMin);
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
