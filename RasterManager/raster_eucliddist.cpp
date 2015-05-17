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
        const char * psOutputRaster, const char * psUnits ){

    CheckFile(psInputRaster, true);
    CheckFile(psOutputRaster, false);

    RasterMeta rmRasterMeta(psInputRaster);

    //"Pixels not square, distances will be inaccurate."
    double dfDistMult = 1.0;
    dfDistMult = fabs(rmRasterMeta.GetCellWidth());



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

//    char **papszOptions = NULL;
//    papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "LZW");
//    papszOptions = CSLSetNameValue(papszOptions, "DISTUNITS", "GEO");

    //  We're using GDAL's computeproximity
    //  http://www.gdal.org/gdal__alg_8h.html#a851815400a579aae9de01199b416fa42
    //    CPLErr err = GDALComputeProximity(pRBInput, pRBOutput, papszOptions, NULL, NULL);

    CalculateStats(pRBOutput);
//    CSLDestroy( papszOptions );

    GDALClose(pDSInput);
    GDALClose(pDSOutput);

    return PROCESS_OK;

}


int Raster::EuclideanDistanceProcessLine(GInt32 *panSrcScanline, int *panNearX, int *panNearY,
                                         int bForward, int iLine, int nXSize, double dfMaxDist,
                                         float *pafProximity,
                                         int nTargetValues, int *panTargetValues)
{

    int iStart, iEnd, iStep, iPixel;

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
        if( nTargetValues == 0 )
            bIsTarget = (panSrcScanline[iPixel] != 0);
        else
        {
            int i;

            for( i = 0; i < nTargetValues; i++ )
            {
                if( panSrcScanline[iPixel] == panTargetValues[i] )
                    bIsTarget = TRUE;
            }
        }

        if( bIsTarget )
        {
            pafProximity[iPixel] = 0.0;
            panNearX[iPixel] = iPixel;
            panNearY[iPixel] = iLine;
            continue;
        }

        /* -------------------------------------------------------------------- */
        /*      Are we near(er) to the closest target to the above (below)      */
        /*      pixel?                                                          */
        /* -------------------------------------------------------------------- */
        float fNearDistSq = (float) (MAX(dfMaxDist,nXSize) * MAX(dfMaxDist,nXSize) * 2);
        float fDistSq;

        if( panNearX[iPixel] != -1 )
        {
            fDistSq = (float)
                    ((panNearX[iPixel] - iPixel) * (panNearX[iPixel] - iPixel)
                     + (panNearY[iPixel] - iLine) * (panNearY[iPixel] - iLine));

            if( fDistSq < fNearDistSq )
            {
                fNearDistSq = fDistSq;
            }
            else
            {
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
            fDistSq = (float)
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
            fDistSq = (float)
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
                && (pafProximity[iPixel] < 0
                    || fNearDistSq < pafProximity[iPixel] * pafProximity[iPixel]) )
            pafProximity[iPixel] = sqrt(fNearDistSq);
    }

    return CE_None;
}


}
