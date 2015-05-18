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
        const char * psOutputRaster,
        const char * psUnits ){

    CheckFile(psInputRaster, true);
    CheckFile(psOutputRaster, false);

    RasterMeta rmRasterMeta(psInputRaster);

    // Pixel units are the default
    double dfDistMult = 1.0;
    bool bInner = true;


    // If geo is specified then we multiply by cell width
    if (QString(psUnits).compare("geo", Qt::CaseInsensitive) == 0){
        //Mean is the only operation we currently supportl
        dfDistMult = fabs(rmRasterMeta.GetCellWidth());
    }


    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(psInputRaster, GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException(INPUT_FILE_ERROR, "Could not open input Raster");

    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */
    RasterMeta rmOutputMeta;
    rmOutputMeta = rmRasterMeta;

    int nCols = rmRasterMeta.GetCols();
    int nRows = rmRasterMeta.GetRows();

    // Crude Maximum Distance
    double dfMaxDist = (double)(nRows + nCols);


    // Decision: We can't mix types here so the output will always be double
    GDALDataType outDataType = GDT_Float64;
    rmOutputMeta.SetGDALDataType(&outDataType);

    double fNoDataValue = (double) -std::numeric_limits<float>::max();
    rmOutputMeta.SetNoDataValue(&fNoDataValue);

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutputRaster, &rmRasterMeta);
    GDALRasterBand * pRBOutput = pDSOutput->GetRasterBand(1);

    double * pReadBuffer = (double*) CPLMalloc(sizeof(double) * nCols);
    double * pOutputBuffer = (double*) CPLMalloc(sizeof(double) * nCols);

    int * panNearX = (int *) CPLMalloc(sizeof(int) * nCols);
    int * panNearY = (int *) CPLMalloc(sizeof(int) * nCols);

    // Read from top to bottom of file
    // ----------------------------------------------------

    // Reset the buffers
    for( int i = 0; i < nCols; i++ )
        panNearX[i] = panNearY[i] = -1;

    for ( int iLine = 0; iLine < nRows; iLine++)
    {
        pRBInput->RasterIO(GF_Read, 0,  iLine, nCols, 1, pReadBuffer, nCols, 1, GDT_Float64, 0, 0);

        for( int j = 0; j < nCols; j++ )
            pOutputBuffer[j] = -1.0;

        // Left to rightpOutputBuffer
        EuclideanDistanceProcessLine( pReadBuffer, panNearX, panNearY,
                                      TRUE, iLine, nCols, dfMaxDist,
                                      pOutputBuffer, rmRasterMeta.GetNoDataValue());

        // Right to Left
        EuclideanDistanceProcessLine( pReadBuffer, panNearX, panNearY,
                                      FALSE, iLine, nCols, dfMaxDist,
                                      pOutputBuffer, rmRasterMeta.GetNoDataValue());

        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  iLine, nCols, 1, pOutputBuffer,
                                              nCols, 1, GDT_Float64, 0, 0);

    }

    // Read from bottom to top of file
    // ----------------------------------------------------

    // Reset the buffers
    for( int i = 0; i < nCols; i++ )
        panNearX[i] = panNearY[i] = -1;

    for( int iLine = nRows -1; iLine >= 0; iLine-- )
    {
        pRBInput->RasterIO(GF_Read, 0,  iLine, nCols, 1, pReadBuffer, nCols, 1, GDT_Float64, 0, 0);

        // Read the output buffer from the previous for loop
        pRBOutput->RasterIO(GF_Read, 0,  iLine, nCols, 1, pOutputBuffer, nCols, 1, GDT_Float64, 0, 0);

        // Right to Left
        EuclideanDistanceProcessLine( pReadBuffer, panNearX, panNearY,
                                      FALSE, iLine, nCols, dfMaxDist,
                                      pOutputBuffer, rmRasterMeta.GetNoDataValue());

        // Left to rightpOutputBuffer
        EuclideanDistanceProcessLine( pReadBuffer, panNearX, panNearY,
                                      TRUE, iLine, nCols, dfMaxDist,
                                      pOutputBuffer, rmRasterMeta.GetNoDataValue());

        // Final post processing of distances.
        for( int iCol = 0; iCol < nCols; iCol++ )
        {
            if( pOutputBuffer[iCol] < 0.0 || pReadBuffer[iCol] == rmRasterMeta.GetNoDataValue())
                pOutputBuffer[iCol] = rmRasterMeta.GetNoDataValue();
            else if( pOutputBuffer[iCol] > 0.0 )
            {
                pOutputBuffer[iCol] = pOutputBuffer[iCol] * dfDistMult;
            }
        }

        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  iLine, nCols, 1, pOutputBuffer,
                                              nCols, 1, GDT_Float64, 0, 0);
    }

    CPLFree(pReadBuffer);
    CPLFree(pOutputBuffer);

    CPLFree(panNearX);
    CPLFree(panNearY);

    CalculateStats(pRBOutput);

    GDALClose(pDSInput);
    GDALClose(pDSOutput);

    return PROCESS_OK;

}


int Raster::EuclideanDistanceProcessLine(double *pReadBuffer, int *panNearX, int *panNearY,
                                         int bForward, int iLine, int nXSize, double dfMaxDist,
                                         double *pOutputBuffer, double dNoData )
{
    int iStart, iEnd, iStep, iPixel;

    // Going forwards or backwards?
    if( bForward )
    {
        iStart = 0;
        iEnd = nXSize;
        iStep = 1;
    }
    else
    {
        iStart = nXSize-1;
        iEnd = -1;
        iStep = -1;
    }

    for( iPixel = iStart; iPixel != iEnd; iPixel += iStep )
    {
        int bIsTarget = FALSE;

        /* -------------------------------------------------------------------- */
        /*      Is the current pixel a target pixel?                            */
        /* -------------------------------------------------------------------- */
        bIsTarget = (pReadBuffer[iPixel] == dNoData);

        if( bIsTarget )
        {
            pOutputBuffer[iPixel] = 0.0;
            panNearX[iPixel] = iPixel;
            panNearY[iPixel] = iLine;
            continue;
        }

        /* -------------------------------------------------------------------- */
        /*      Are we near(er) to the closest target to the above (below)      */
        /*      pixel?                                                          */
        /* -------------------------------------------------------------------- */
        double fNearDistSq = (double) (MAX(dfMaxDist,nXSize) * MAX(dfMaxDist,nXSize) * 2);
        double fDistSq;

        if( panNearX[iPixel] != -1 )
        {
            fDistSq = (double)
                    ((panNearX[iPixel] - iPixel) * (panNearX[iPixel] - iPixel)
                     + (panNearY[iPixel] - iLine) * (panNearY[iPixel] - iLine));

            if( fDistSq < fNearDistSq ) {
                fNearDistSq = fDistSq;
            }
            else {
                panNearX[iPixel] = -1;
                panNearY[iPixel] = -1;
            }
        }

        /* -------------------------------------------------------------------- */
        /*      Are we near(er) to the closest target to the left (right)       */
        /*      pixel?                                                          */
        /* -------------------------------------------------------------------- */
        int iLast = iPixel-iStep;

        if( iPixel != iStart && panNearX[iLast] != -1 )
        {
            fDistSq = (double)
                    ((panNearX[iLast] - iPixel) * (panNearX[iLast] - iPixel)
                     + (panNearY[iLast] - iLine) * (panNearY[iLast] - iLine));

            if( fDistSq < fNearDistSq )
            {
                fNearDistSq = fDistSq;
                panNearX[iPixel] = panNearX[iLast];
                panNearY[iPixel] = panNearY[iLast];
            }
        }

        /* -------------------------------------------------------------------- */
        /*      Are we near(er) to the closest target to the topright           */
        /*      (bottom left) pixel?                                            */
        /* -------------------------------------------------------------------- */
        int iTR = iPixel+iStep;

        if( iTR != iEnd && panNearX[iTR] != -1 )
        {
            fDistSq = (double)
                    ((panNearX[iTR] - iPixel) * (panNearX[iTR] - iPixel)
                     + (panNearY[iTR] - iLine) * (panNearY[iTR] - iLine));

            if( fDistSq < fNearDistSq )
            {
                fNearDistSq = fDistSq;
                panNearX[iPixel] = panNearX[iTR];
                panNearY[iPixel] = panNearY[iTR];
            }
        }

        /* -------------------------------------------------------------------- */
        /*      Update our proximity value.                                     */
        /* -------------------------------------------------------------------- */
        if( panNearX[iPixel] != -1
                && fNearDistSq <= dfMaxDist * dfMaxDist
                && (pOutputBuffer[iPixel] < 0
                    || fNearDistSq < pOutputBuffer[iPixel] * pOutputBuffer[iPixel]) )
            pOutputBuffer[iPixel] = sqrt(fNearDistSq);
    }

    return PROCESS_OK;
}


}
