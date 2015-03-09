#define MY_DLL_EXPORT
/*
 * Raster Filter -- Moving window Operations
 *
 * 28 February 2015
 *
*/

#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "raster.h"
#include "gdal.h"
#include "gdal_alg.h"
#include "gdal_priv.h"
#include "rastermanager.h"

namespace RasterManager {

int Raster::EuclideanDistance(
        const char * psInputRaster,
        const char * psOutputRaster ){

    CheckFile(psInputRaster, true);
    CheckFile(psOutputRaster, false);

    RasterMeta rmRasterMeta(psInputRaster);

    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(psInputRaster, GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException(INPUT_FILE_ERROR, "Could not open input Raster");

    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */
    RasterMeta rmOutputMeta;
    rmOutputMeta = rmRasterMeta;

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutputRaster, &rmRasterMeta);
    GDALRasterBand * pRBOutput = pDSOutput->GetRasterBand(1);

    char **papszOptions = NULL;
    //    papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "LZW");

    //  We're using GDAL's computeproximity
    //  http://www.gdal.org/gdal__alg_8h.html#a851815400a579aae9de01199b416fa42
    CPLErr err = GDALComputeProximity(pRBInput, pRBOutput, papszOptions, NULL, NULL);
    CSLDestroy( papszOptions );


    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDSInput);
    GDALClose(pDSOutput);

    PrintRasterProperties(psOutputRaster);

    return PROCESS_OK;

}

}
