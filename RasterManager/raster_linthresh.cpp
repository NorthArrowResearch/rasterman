#define MY_DLL_EXPORT

#include <gdal_priv.h>
#include <gdal_alg.h>
#include <QDebug>

#include "raster.h"
#include "rastermeta.h"
#include "rastermanager.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"

namespace RasterManager {

int Raster::LinearThreshold(const char * psInputRaster,
                            const char * psOutputRaster,
                            double dLowThresh,
                            double dLowThreshVal,
                            double dHighThresh,
                            double dHighThreshVal){

    // Check for input and output files
    CheckFile(psInputRaster, true);
    CheckFile(psOutputRaster, false);

    if (dLowThresh > dHighThresh)
        throw RasterManagerException(ARGUMENT_VALIDATION, "Low threshold must be smaller or equal to the high threshold");

    RasterMeta rmRasterMeta(psInputRaster);

    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(psInputRaster, GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException(INPUT_FILE_ERROR, "Could not open input Raster");

    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */

    RasterMeta rmOutputMeta = rmRasterMeta;

    // Decision: Output needs to be a double raster.
    GDALDataType outDataType = GDT_Float64;
    rmOutputMeta.SetGDALDataType(&outDataType);

    double fNoDataValue = (double) -std::numeric_limits<float>::max();
    rmOutputMeta.SetNoDataValue(&fNoDataValue);

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutputRaster, &rmRasterMeta);

    double * pInputLine = (double *) CPLMalloc(sizeof(double)*rmRasterMeta.GetCols());
    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*rmRasterMeta.GetCols());

    // REcall: y =mx +b  where m=slope
    double dSlope = 0;

    // Kids, division by zero is a serious offense!
    if ( dHighThresh != dLowThresh)
        dSlope =  (dHighThreshVal - dLowThreshVal) / (dHighThresh - dLowThresh);

    double dBparam = dHighThreshVal - ( dSlope * dHighThresh );

    for (int i = 0; i < rmRasterMeta.GetRows(); i++)
    {
        pRBInput->RasterIO(GF_Read, 0,  i, rmRasterMeta.GetCols(), 1, pInputLine, rmRasterMeta.GetCols(), 1, GDT_Float64, 0, 0);
        for (int j = 0; j < rmRasterMeta.GetCols(); j++)
        {
            // First the 3 easy cases: greater than upper threshold, less than lower or nodataval.
            if (pInputLine[j] == fNoDataValue){
                pOutputLine[j] = fNoDataValue;
            }
            else if (pInputLine[j] >= dHighThresh)
                pOutputLine[j] = dHighThreshVal;
            else if (pInputLine[j] < dLowThresh)
                pOutputLine[j] = dLowThreshVal;
            else
            {
                pOutputLine[j] = ( pInputLine[j] * dSlope ) + dBparam;
            }
        }
        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, rmRasterMeta.GetCols(), 1, pOutputLine, rmRasterMeta.GetCols(), 1, GDT_Float64, 0, 0);
    }

    CPLFree(pInputLine);
    CPLFree(pOutputLine);

    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDSInput);
    GDALClose(pDSOutput);

    return PROCESS_OK;
}

}
