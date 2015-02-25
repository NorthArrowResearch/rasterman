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
        //Mean is the only operation we currently supportl
        nFilterOp = FILTER_MEAN;
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

    double fNoDataValue;
    if (rmRasterMeta.GetNoDataValuePtr() == NULL){
        fNoDataValue = (double) -std::numeric_limits<float>::max();
    }
    else {
        fNoDataValue = rmRasterMeta.GetNoDataValue();
    }

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutputRaster, &rmRasterMeta);

    // Our Read Buffer is 2D: [nWindowHeight x entire row legnth]
    double * pInputWindow[nWindowHeight];
    for (int i = 0; i < nWindowHeight; i++)
        pInputWindow[i] = (double *) CPLMalloc(sizeof(double)*rmRasterMeta.GetCols());

    // Our write buffer is a regular line
    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*rmRasterMeta.GetCols());

    // Specify the middle pixel of the window with these two coords
    int nWindowMiddleWidth = (nWindowWidth / 2) + 1;
    int nWindowMiddleHeight = (nWindowHeight / 2) + 1;

    /*****************************************************************************************
     * Start looping over the raster
     */

    for (int nOutRow = 0; nOutRow < rmOutputMeta.GetRows(); nOutRow++)
    {
        int nWindowRow = nOutRow - nWindowMiddleHeight;
        int nWindowTrimRow = 0;

        // At the top and bottom we need to shrink our reading windows so we don't overlap the edge of the file.
        if (nWindowRow < 0){
            nWindowTrimRow = abs(nWindowRow);
            nWindowRow = 0;
        }
        else if( nWindowRow + nWindowHeight >= rmRasterMeta.GetRows() ){
            nWindowTrimRow = abs(nWindowRow);
        }

        // Read a 2D swath of the input, adjusting if any of the window top or bottom falls over the edge.
        pRBInput->RasterIO(GF_Read, 0,  nWindowRow, nWindowWidth, (nWindowHeight - nWindowTrimRow), pInputWindow, rmRasterMeta.GetCols(), nWindowHeight, GDT_Float64, 0, 0);

        for (int nOutCol = 0; nOutCol < rmOutputMeta.GetCols(); nOutCol++)
        {
            if ( pInputWindow[nOutRow][nOutCol] == rmRasterMeta.GetNoDataValue() )
            {
                pInputWindow[nOutRow][nOutCol] = pOutputLine[nOutCol] = rmOutputMeta.GetNoDataValue();
            }
            else{

                // At the left and rightwe need to shrink our reading windows so we don't overlap the edge of the file.
                int nWindowCol = nOutCol - nWindowMiddleWidth;
                int nWindowTrimCol = 0;
                if (nWindowCol < 0){
                    nWindowTrimCol = abs( nWindowCol );
                    nWindowCol = 0;
                }
                else if( nWindowCol + nWindowWidth >= rmRasterMeta.GetCols() ){
                    nWindowTrimCol = abs( nWindowCol );
                }

                // Loop over the window for this particular output cell (i,j)
                double dSum = 0;
                int nCells = 0;
                for ( int nWrow = 0; nWrow < ( nWindowHeight - nWindowTrimRow ); nWrow++ ){
                    for ( int nWcol = 0; nWrow < ( nWindowWidth - nWindowTrimCol ); nWcol++ ){

                        // Translate the window coords into raster coords
                        int nRasterRow = nOutRow;
                        int nRasterCol = nOutCol;
                        if ( nRasterRow >= 0 && nRasterRow < rmRasterMeta.GetRows() &&
                                nRasterCol >= 0 && nRasterCol < rmRasterMeta.GetCols() ){
                            if (pInputWindow[nRasterRow][nRasterCol] != fNoDataValue){
                                nCells++;
                                dSum += pInputWindow[nRasterRow][nRasterCol];
                            }
                        }
                    }
                }
                if (nFilterOp == FILTER_MEAN)
                    pOutputLine[nOutCol] = dSum / nCells;
            }
        }

        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  nOutRow, rmOutputMeta.GetCols(), 1, pOutputLine, rmOutputMeta.GetCols(), 1, *rmRasterMeta.GetGDALDataType() , 0, 0);
    }

    // Free our 2D buffer
    for (int t=0; t< nWindowHeight; t++)
         CPLFree(pInputWindow[t]);

    // Free our 1D Output buffer
    CPLFree(pOutputLine);

    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDSInput);
    GDALClose(pDSOutput);

    return PROCESS_OK;

}

}
