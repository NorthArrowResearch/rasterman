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
#include "gdal_priv.h"
#include "rastermanager.h"

namespace RasterManager {

const int MAX_WINDOW_WIDTH = 15;
const int MAX_WINDOW_HEIGHT = 15;

enum RasterManagerFilterOperations {
    FILTER_MEAN,
    FILTER_RANGE,
};

int Raster::FilterRaster(
        const char * psOperation,
        const char * psInputRaster,
        const char * psOutputRaster,
        int nWindowWidth,
        int nWindowHeight ){

    // Check for input and output files
    CheckFile(psInputRaster, true);
    CheckFile(psOutputRaster, false);

    RasterManagerFilterOperations nFilterOp;

    /*****************************************************************************************
     * Basic validation on files and parameters passed in
     */

    RasterMeta rmRasterMeta(psInputRaster);

    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(psInputRaster, GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException(INPUT_FILE_ERROR, "Could not open input Raster");

    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    if (QString(psOperation).compare("mean", Qt::CaseInsensitive) == 0){
        nFilterOp = FILTER_MEAN;
    }
    else if (QString(psOperation).compare("range", Qt::CaseInsensitive) == 0){
        nFilterOp = FILTER_RANGE;
    }
    else{
        throw RasterManagerException(ARGUMENT_VALIDATION, QString("Operation argument was invalid: %1").arg(psOperation) );
    }

    if ( nWindowWidth % 2 == 0 )
        throw RasterManagerException(ARGUMENT_VALIDATION, "height must be an odd number of cells.");
    if ( nWindowHeight % 2 == 0 )
        throw RasterManagerException(ARGUMENT_VALIDATION, "width must be an odd number of cells.");

    if ( nWindowWidth > MAX_WINDOW_WIDTH )
        throw RasterManagerException(ARGUMENT_VALIDATION, QString("Width was greater than allowed window size of %1 cells").arg(MAX_WINDOW_WIDTH) );
    if ( nWindowHeight > MAX_WINDOW_HEIGHT )
        throw RasterManagerException(ARGUMENT_VALIDATION, QString("Height was greater than allowed window size of %1 cells").arg(MAX_WINDOW_HEIGHT) );

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

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutputRaster, &rmRasterMeta);

    // Our Read Buffer is 2D: [nWindowHeight x entire row legnth]
    double * pInputWindow = (double *) CPLMalloc(sizeof(double)*rmRasterMeta.GetCols() * nWindowHeight);

    // Our write buffer is a regular line
    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*rmRasterMeta.GetCols());

    // Specify the middle index of the window with these two coords
    int nWindowMiddleRow = (nWindowHeight / 2);
    int nWindowMiddleCol = (nWindowWidth / 2);

    /*****************************************************************************************
     * Start looping over the raster
     */

    for (int nOutRow = 0; nOutRow < rmOutputMeta.GetRows(); nOutRow++)
    {
        // If the top of the window is below the top of the raster then nothing changes
        int nWindowTopRow = nOutRow - nWindowMiddleRow;
        int nWindowHeightAdj = nWindowHeight;
        int nWindowMiddleRowAdj = nWindowMiddleRow;

        // At the top and bottom we need to shrink our reading windows so we don't overlap the edge of the file.
        if (nWindowTopRow < 0){
            nWindowHeightAdj = nWindowHeight + nWindowTopRow;
            nWindowTopRow = 0;
        }
        else if( nWindowTopRow + nWindowHeight >= rmRasterMeta.GetRows() ){
            nWindowHeightAdj = rmRasterMeta.GetRows() - nWindowTopRow;
        }
        if ( nWindowMiddleRow > nOutRow ){
            nWindowMiddleRowAdj = 0;
        }

        // Read a 2D swath of the input, adjusting if any of the window top or bottom falls over the edge.
        pRBInput->RasterIO(GF_Read,
                           0, nWindowTopRow,
                           rmRasterMeta.GetCols(), nWindowHeightAdj,
                           pInputWindow,
                           rmRasterMeta.GetCols(), nWindowHeightAdj,
                           GDT_Float64, 0, 0);

        for ( int nOutCol = 0; nOutCol < rmOutputMeta.GetCols(); nOutCol++ )
        {
            // At the left and right we need to shrink our reading windows so we don't overlap the edge of the file.
            int nWindowLeftCol = nOutCol - nWindowMiddleCol;
            int nWindowWidthAdj = nWindowWidth;
            int nWindowMiddleColAdj = nWindowMiddleCol;

            if ( nWindowLeftCol < 0 ){
                nWindowWidthAdj = nWindowWidth + nWindowLeftCol;
                nWindowLeftCol = 0;
            }
            else if( nWindowLeftCol + nWindowWidth >= rmRasterMeta.GetCols() ){
                nWindowWidthAdj = rmRasterMeta.GetCols() - nWindowLeftCol;
            }
            if ( nWindowMiddleCol > nOutCol ){
                nWindowMiddleColAdj = 0;
            }

            // if the output row is nodataval then just set that.
            int nMiddleWindowInd = ( nWindowMiddleRowAdj * rmRasterMeta.GetCols()) + ( nWindowLeftCol + nWindowMiddleColAdj);

            if ( pInputWindow[ nMiddleWindowInd ] == rmRasterMeta.GetNoDataValue() )
            {
                pOutputLine[nOutCol] = rmOutputMeta.GetNoDataValue();
            }
            else{

                // Loop over the window for this particular output cell (i,j)
                double dSum = 0;
                double dMin = fNoDataValue;
                double dMax = fNoDataValue;
                int nCells = 0;
                for ( int nWrow = 0; nWrow < nWindowHeightAdj; nWrow++ ){
                    for ( int nWcol = 0; nWcol < nWindowWidthAdj; nWcol++ ){
                        // Translate the window coords into raster coords
                        int nWindowRasterInd = ( nWrow * rmRasterMeta.GetCols() ) + (nWcol + nWindowLeftCol);

                        // For Mean we need to sum so we can divide by total points later
                        if (nFilterOp == FILTER_MEAN){
                            if (pInputWindow[ nWindowRasterInd ] != fNoDataValue){
                                nCells++;
                                dSum += pInputWindow[ nWindowRasterInd ];
                            }
                        }

                        // For Range we need to set max and min if appropriate
                        else if (nFilterOp == FILTER_RANGE){
                            double val = pInputWindow[ nWindowRasterInd ];
                            if (val != fNoDataValue){
                                if (dMin == fNoDataValue || val < dMin)
                                    dMin = val;
                                if (dMax == fNoDataValue || val > dMax)
                                    dMax = val;
                            }
                        }
                    }
                }
                // Final Combination Method
                if (nFilterOp == FILTER_MEAN)
                    pOutputLine[nOutCol] = dSum / nCells;
                else if (nFilterOp == FILTER_RANGE){
                    if (dMax != fNoDataValue && dMin != fNoDataValue)
                        pOutputLine[nOutCol] = dMax - dMin;
                    else{
                        pOutputLine[nOutCol] = fNoDataValue;
                    }
                }
            }
        }

        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  nOutRow,
                                              rmOutputMeta.GetCols(), 1,
                                              pOutputLine,
                                              rmOutputMeta.GetCols(), 1,
                                              GDT_Float64, 0, 0);
    }

    // Free our buffers
    CPLFree(pInputWindow);
    CPLFree(pOutputLine);

    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDSInput);
    GDALClose(pDSOutput);

    return PROCESS_OK;

}

}
