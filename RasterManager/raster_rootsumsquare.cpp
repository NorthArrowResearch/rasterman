#define MY_DLL_EXPORT
/*
 * Root Sum Square Operation on a Raster
 *
 * 6 December 2014
 *
*/
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "raster.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "rastermanager.h"



namespace RasterManager {


int Raster::RasterRootSumSquares(const char * psRaster1, const char * psRaster2, const char * psOutput){

    /*****************************************************************************************
     * Raster 1
     */
    if (psRaster1 == NULL)
        return INPUT_FILE_ERROR;

    //Input Validation
    CheckFile(psRaster1, true);
    CheckFile(psRaster2, true);

    GDALDataset * pDS1 = (GDALDataset*) GDALOpen(psRaster1, GA_ReadOnly);
    if (pDS1 == NULL)
        return INPUT_FILE_ERROR;

    GDALRasterBand * pRBInput1 = pDS1->GetRasterBand(1);
    RasterMeta InputMeta1(psRaster1);

    /*****************************************************************************************
     * Raster 2
     */
    if (psRaster2 == NULL)
        return INPUT_FILE_ERROR;

    GDALDataset * pDS2 = (GDALDataset*) GDALOpen(psRaster2, GA_ReadOnly);
    if (pDS1 == NULL)
        return INPUT_FILE_ERROR;

    GDALRasterBand * pRBInput2 = pDS2->GetRasterBand(1);
    RasterMeta InputMeta2(psRaster2);

    /*****************************************************************************************
     * Check that input rasters have the same numbers of rows and columns
     */
    if (pRBInput1->GetXSize() != pRBInput2->GetXSize())
        return COLS_ERROR;

    if (pRBInput1->GetYSize() != pRBInput2->GetYSize())
        return ROWS_ERROR;

    if (psOutput == NULL)
        return OUTPUT_FILE_MISSING;

    // They are the same. Now we can say this:
    int cols = InputMeta1.GetCols();

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */
    double fNoDataValue = (double) -std::numeric_limits<float>::max();

    // Create the output dataset for writing
    RasterMeta OutputMeta(InputMeta1);
    OutputMeta.SetNoDataValue(&fNoDataValue);
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, &OutputMeta);

    /*****************************************************************************************
     * Allocate the memory for the input / output lines
     */
    double * pInputLine1 = (double *) CPLMalloc(sizeof(double)*cols);
    double * pInputLine2 = (double *) CPLMalloc(sizeof(double)*cols);
    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*cols);

    int i, j;
    for (i = 0; i < InputMeta1.GetRows(); i++)
    {
        pRBInput1->RasterIO(GF_Read, 0,  i, cols, 1, pInputLine1, cols, 1, GDT_Float64, 0, 0);
        pRBInput2->RasterIO(GF_Read, 0,  i, cols, 1, pInputLine2, cols, 1, GDT_Float64, 0, 0);

        for (j = 0; j < InputMeta1.GetCols(); j++)
        {
            if ( ( pInputLine1[j] == InputMeta1.GetNoDataValue() ) ||
                 ( pInputLine2[j] == InputMeta2.GetNoDataValue() ) ) {
                pOutputLine[j] = fNoDataValue;
            }
            else
                pOutputLine[j] = sqrt( pow(pInputLine1[j], 2) + pow(pInputLine2[j], 2) );
        }

        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, cols, 1, pOutputLine, cols, 1, GDT_Float64, 0, 0);
    }

    CPLFree(pInputLine1);
    CPLFree(pInputLine2);
    CPLFree(pOutputLine);

    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDS1);
    GDALClose(pDS2);
    GDALClose(pDSOutput);

    return PROCESS_OK;

}

}
