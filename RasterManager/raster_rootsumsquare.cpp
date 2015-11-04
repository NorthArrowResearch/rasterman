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
    float fNoDataValue = (float) -std::numeric_limits<float>::max();

    // Create the output dataset for writing
    RasterMeta InputMeta(psRaster1);
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, &InputMeta);


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

    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDS1);
    GDALClose(pDS2);
    GDALClose(pDSOutput);

    return PROCESS_OK;

}

}
