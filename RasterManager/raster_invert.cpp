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

int Raster::InvertRaster(const char * psInputRaster,
                         const char * psOutputRaster,
                         double dValue)
{
    CheckFile(psInputRaster, true);
    CheckFile(psOutputRaster, false);

    RasterMeta rmRasterMeta(psInputRaster);

    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(psInputRaster, GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException(INPUT_FILE_ERROR, "Could not open input Raster");

    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    double * pInputLine = (double *) CPLMalloc(sizeof(double)*rmRasterMeta.GetCols());

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */
    RasterMeta rmOutputMeta;
    rmOutputMeta = rmRasterMeta;

    double fNoDataValue;
    if (rmRasterMeta.GetNoDataValuePtr() == NULL){
        fNoDataValue = (double) -std::numeric_limits<float>::max();
    }
    else {
        fNoDataValue = rmRasterMeta.GetNoDataValue();
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
            if ( pInputLine[j] == rmRasterMeta.GetNoDataValue() )
            {
                pOutputLine[j] = dValue;
            }
            else
            {
                pOutputLine[j] = rmOutputMeta.GetNoDataValue();
            }
        }

        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, rmOutputMeta.GetCols(), 1, pOutputLine, rmOutputMeta.GetCols(), 1, GDT_Float64, 0, 0);
    }
    CPLFree(pInputLine);
    CPLFree(pOutputLine);

    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDSInput);
    GDALClose(pDSOutput);

    PrintRasterProperties(psOutputRaster);

    return PROCESS_OK;

}

}
