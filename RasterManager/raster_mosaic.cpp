#define MY_DLL_EXPORT
/*
 * Raster Mosaic Operations
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

int Raster::RasterMosaic(const char * csRasters, const char * psOutput)
{
    // Check for input and output files
    CheckFile(psOutput, false);

    // Split the string with delimiters into individual paths
    // Also check that those files exist
    QList<QString> slRasters = RasterUnDelimit(csRasters, true, false, false);
    RasterMeta * OutputMeta = RasterMetaExpand(slRasters);

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, OutputMeta);

    //projectionRef use from inputs.

    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*OutputMeta->GetCols());

    /*****************************************************************************************
     * Loop over the inputs and then the rows and columns
     *
     */
    foreach(QString raster, slRasters){

        const QByteArray qbRasterFilePath = raster.toLocal8Bit();
        GDALDataset * pDS = (GDALDataset*) GDALOpen(qbRasterFilePath.data(), GA_ReadOnly);
        GDALRasterBand * pRBInput = pDS->GetRasterBand(1);

        RasterMeta inputMeta (qbRasterFilePath.data());
        // We need to figure out where in the output the input lives.
        int trans_i = OutputMeta->GetRowTranslation(&inputMeta);
        int trans_j = OutputMeta->GetColTranslation(&inputMeta);

        double * pInputLine = (double *) CPLMalloc(sizeof(double)*pRBInput->GetXSize());

        for (int i = 0; i < pRBInput->GetYSize(); i++){
            pRBInput->RasterIO(GF_Read, 0,  i, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
            // here's where we need to get the correct row of the output. Replace
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Read, 0,  trans_i+i, OutputMeta->GetCols(), 1, pOutputLine, OutputMeta->GetCols(), 1, GDT_Float64, 0, 0);

            for (int j = 0; j < pRBInput->GetXSize(); j++){
                // If the input line is empty then do nothing
                if ( (pInputLine[j] != inputMeta.GetNoDataValue())
                     && pOutputLine[trans_j+j] ==  OutputMeta->GetNoDataValue())
                {
                    pOutputLine[trans_j+j] = pInputLine[j];
                }
            }
            // here's where we need to get the correct row of the output. Replace
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  trans_i+i, OutputMeta->GetCols(), 1, pOutputLine, OutputMeta->GetCols(), 1, GDT_Float64, 0, 0);

        }
        CPLFree(pInputLine);
    }


    /*****************************************************************************************
     * Now Close everything and clean it all up
     */
    CPLFree(pOutputLine);
    CalculateStats(pDSOutput->GetRasterBand(1));
    GDALClose(pDSOutput);

    return PROCESS_OK;
}


}
