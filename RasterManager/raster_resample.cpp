#include "raster.h"
#include "rastermanager_interface.h"
#include "gdal_priv.h"

#include <limits>
#include <math.h>
#include <string>

namespace RasterManager {

int Raster::ReSample_Float64(GDALRasterBand * pRBInput, GDALRasterBand * pRBOutput, double fNewCellSize, double fNewLeft, double fNewTop, int nNewRows, int nNewCols)
{
    // The properties of the original raster.
    double fOldLeft = GetLeft();
    double fOldYOrigin = GetRight();
    double fOldCellHeight = GetCellHeight();
    double fOldCellWidth = GetCellWidth();

    int nSuccess = 0;
    double fNoData = pRBInput->GetNoDataValue(&nSuccess);
    if (nSuccess == 0)
        fNoData = std::numeric_limits<double>::lowest();//return INPUT_FILE_TRANSFORM_ERROR;

    /*************************************************************************************************
    * Create the memory buffers for reading the old raster and writing the new raster.
    */
    double * pOutputLine = (double *) CPLMalloc(sizeof(double) *pRBOutput->GetXSize());
    double * pTopLine = (double *) CPLMalloc(sizeof(double)*pRBInput->GetXSize());
    double * pBotLine = (double *) CPLMalloc(sizeof(double)*pRBInput->GetXSize());

    /*************************************************************************************************
    * Loop over the raster rows. Note that geographic coordinate origin is bottom left. But
    * the GDAL image anchor is top left. The cell height is negative.
    *
    * The loop is over the centres of the output raster cells. Two rows are read from the
    * input raster. The line just above the output cell and the line just below. The line
    * just above is called the "anchor" row.
    */
    int i, j;
    int nOldTopRow, nOldBotRow;

    for (i = 0; i < nNewRows; i++)
    {
        double fNewY = fNewTop - (i * fNewCellSize) - (fNewCellSize / 2);
        double fOldRow = (fNewY - fOldYOrigin) / fOldCellHeight;
        if ( fmod(fOldRow, 1) < 0.5)
        {
            nOldTopRow = (int) floor(fOldRow) - 1;
            nOldBotRow = (int) floor(fOldRow);
        }
        else
        {
            nOldTopRow = (int) floor(fOldRow);
            nOldBotRow = (int) floor(fOldRow) + 1;
        }

        double fOldY = fOldYOrigin + nOldTopRow * fOldCellHeight + (fOldCellHeight / 2);

        if (nOldTopRow >=0 && nOldBotRow < pRBInput->GetYSize())
        {
            pRBInput->RasterIO(GF_Read, 0, nOldTopRow, pRBInput->GetXSize(), 1, pTopLine, pRBInput->GetXSize(), 1, (GDALDataType) this->GetGDALDataType(), 0, 0);
            pRBInput->RasterIO(GF_Read, 0, nOldBotRow, pRBInput->GetXSize(), 1, pBotLine, pRBInput->GetXSize(), 1, (GDALDataType) this->GetGDALDataType(), 0, 0);

            for (j = 0; j < nNewCols; j++)
            {
                pOutputLine[j] = (float) fNoData;
                double fNewX = fNewLeft + (j * fNewCellSize) + (fNewCellSize / 2);
                double fOldCol = (fNewX - fOldLeft) / fOldCellWidth;

                int nOldLeftCol;
                if ( fmod(fOldCol, 1) < 0.5)
                {
                    nOldLeftCol = (int) floor(fOldCol) - 1;
                }
                else
                {
                    nOldLeftCol = (int) floor(fOldCol);
                }

                double fOldX = fOldLeft + (nOldLeftCol * fOldCellWidth) + (fOldCellWidth / 2);
                /*
printf("Output row : % 4d Output  Y: %0.6f\n", i, fNewY);
printf("Input Row  : % 4d Input   Y: %0.6f\n", nOldAnchorRow, fOldY);
printf("Input Row+1: % 4d Input+1 Y: %0.6f\n\n", nOldAnchorRow + 1, fOldY + fOldCellHeight);

printf("Output Col : % 4d Output  X: %0.6f\n", j, fNewX);
printf("Input Col  : % 4d Input   X: %0.6f\n", nOldLeftCol, fOldX);
printf("Input Col+1: % 4d Input+1 X: %0.6f\n\n", nOldLeftCol + 1, fOldX + fOldCellWidth);
*/

                if (nOldLeftCol >= 0 && nOldLeftCol < pRBInput->GetXSize())
                {
                    double Z01 = pTopLine[nOldLeftCol];
                    double Z11 = pTopLine[nOldLeftCol + 1];

                    double Z00 = pBotLine[nOldLeftCol];
                    double Z10 = pBotLine[nOldLeftCol + 1];

                    // Proceed with calculation if the input cells have valid data. this is true if the
                    // input raster does not possess a NoData value or if it does, and all the cells do
                    // not equal the missing data value.
                    if (!this->HasNoDataValue() || (Z01 != fNoData) && (Z11 != fNoData) && (Z00 != fNoData) && (Z10 != fNoData))
                    {
                        double Lx = fOldX;
                        double Z1 = Z01 + (Z11 - Z01) * ((fNewX - Lx) / fOldCellWidth);
                        double Z0 = Z00 + (Z10 - Z00) * ((fNewX - Lx) / fOldCellWidth);

                        double Ty = fOldY;
                        double Z = Z1 - (Z1 - Z0) * ((fNewY - Ty) / fOldCellHeight);

                        pOutputLine[j] = (double) Z;
                    }
                    else
                        pOutputLine[j] = (double) fNoData;
                }
                else
                    pOutputLine[j] = (double) fNoData;
            }
        }
        else
        {
            // Outside the bounds of the input image. Loop over all cells in current output row and set to NoData.
            for (j=0; j< nNewCols; j++)
            {
                pOutputLine[j] = (double) fNoData;
            }
        }
        pRBOutput->RasterIO(GF_Write, 0, i, pRBOutput->GetXSize(), 1, pOutputLine, pRBOutput->GetXSize(), 1, (GDALDataType) this->GetGDALDataType(), 0, 0);
    }

    CPLFree(pTopLine);
    CPLFree(pBotLine);
    CPLFree(pOutputLine);

    return PROCESS_OK;
}


int Raster::ReSample_Float32(GDALRasterBand * pRBInput, GDALRasterBand * pRBOutput, double fNewCellSize, double fNewLeft, double fNewTop, int nNewRows, int nNewCols)
{
    // The properties of the original raster.
    double fOldLeft = GetLeft();
    double fOldYOrigin = GetTop();
    double fOldCellHeight = GetCellHeight();
    double fOldCellWidth = GetCellWidth();

    int nSuccess = 0;
    double fNoData = pRBInput->GetNoDataValue(&nSuccess);
    if (nSuccess == 0)
        fNoData = std::numeric_limits<float>::lowest();//return INPUT_FILE_TRANSFORM_ERROR;

    /*************************************************************************************************
    * Create the memory buffers for reading the old raster and writing the new raster.
    */
    float * pOutputLine = (float *) CPLMalloc(sizeof(float) *pRBOutput->GetXSize());
    float * pTopLine = (float *) CPLMalloc(sizeof(float)*pRBInput->GetXSize());
    float * pBotLine = (float *) CPLMalloc(sizeof(float)*pRBInput->GetXSize());

    /*************************************************************************************************
    * Loop over the raster rows. Note that geographic coordinate origin is bottom left. But
    * the GDAL image anchor is top left. The cell height is negative.
    *
    * The loop is over the centres of the output raster cells. Two rows are read from the
    * input raster. The line just above the output cell and the line just below. The line
    * just above is called the "anchor" row.
    */
    int i, j;
    int nOldTopRow, nOldBotRow;

    for (i = 0; i < nNewRows; i++)
    {
        double fNewY = fNewTop - (i * fNewCellSize) - (fNewCellSize / 2);
        double fOldRow = (fNewY - fOldYOrigin) / fOldCellHeight;
        if ( fmod(fOldRow, 1) < 0.5)
        {
            nOldTopRow = (int) floor(fOldRow) - 1;
            nOldBotRow = (int) floor(fOldRow);
        }
        else
        {
            nOldTopRow = (int) floor(fOldRow);
            nOldBotRow = (int) floor(fOldRow) + 1;
        }

        double fOldY = fOldYOrigin + nOldTopRow * fOldCellHeight + (fOldCellHeight / 2);

        if (nOldTopRow >=0 && nOldBotRow < pRBInput->GetYSize())
        {
            pRBInput->RasterIO(GF_Read, 0, nOldTopRow, pRBInput->GetXSize(), 1, pTopLine, pRBInput->GetXSize(), 1, (GDALDataType) this->GetGDALDataType(), 0, 0);
            pRBInput->RasterIO(GF_Read, 0, nOldBotRow, pRBInput->GetXSize(), 1, pBotLine, pRBInput->GetXSize(), 1, (GDALDataType) this->GetGDALDataType(), 0, 0);

            for (j = 0; j < nNewCols; j++)
            {
                pOutputLine[j] = (float) fNoData;
                double fNewX = fNewLeft + (j * fNewCellSize) + (fNewCellSize / 2);
                double fOldCol = (fNewX - fOldLeft) / fOldCellWidth;

                int nOldLeftCol;
                if ( fmod(fOldCol, 1) < 0.5)
                {
                    nOldLeftCol = (int) floor(fOldCol) - 1;
                }
                else
                {
                    nOldLeftCol = (int) floor(fOldCol);
                }

                double fOldX = fOldLeft + (nOldLeftCol * fOldCellWidth) + (fOldCellWidth / 2);
                /*
printf("Output row : % 4d Output  Y: %0.6f\n", i, fNewY);
printf("Input Row  : % 4d Input   Y: %0.6f\n", nOldAnchorRow, fOldY);
printf("Input Row+1: % 4d Input+1 Y: %0.6f\n\n", nOldAnchorRow + 1, fOldY + fOldCellHeight);

printf("Output Col : % 4d Output  X: %0.6f\n", j, fNewX);
printf("Input Col  : % 4d Input   X: %0.6f\n", nOldLeftCol, fOldX);
printf("Input Col+1: % 4d Input+1 X: %0.6f\n\n", nOldLeftCol + 1, fOldX + fOldCellWidth);
*/

                if (nOldLeftCol >= 0 && (nOldLeftCol + 1) < pRBInput->GetXSize())
                {
                    float Z01 = pTopLine[nOldLeftCol];
                    float Z11 = pTopLine[nOldLeftCol + 1];

                    float Z00 = pBotLine[nOldLeftCol];
                    float Z10 = pBotLine[nOldLeftCol + 1];

                    // Proceed with calculation if the input cells have valid data. this is true if the
                    // input raster does not possess a NoData value or if it does, and all the cells do
                    // not equal the missing data value.
                    if (!this->HasNoDataValue() || (Z01 != fNoData) && (Z11 != fNoData) && (Z00 != fNoData) && (Z10 != fNoData))
                    {
                        double Lx = fOldX;
                        float Z1 = Z01 + (Z11 - Z01) * ((fNewX - Lx) / fOldCellWidth);
                        float Z0 = Z00 + (Z10 - Z00) * ((fNewX - Lx) / fOldCellWidth);

                        double Ty = fOldY;
                        float Z = Z1 - (Z1 - Z0) * ((fNewY - Ty) / fOldCellHeight);

                        pOutputLine[j] = (float) Z;
                    }
                    else
                        pOutputLine[j] = (float) fNoData;
                }
                else
                    pOutputLine[j] = (float) fNoData;
            }
        }
        else
        {
            // Outside the bounds of the input image. Loop over all cells in current output row and set to NoData.
            for (j=0; j< nNewCols; j++)
            {
                pOutputLine[j] = (float) fNoData;
            }
        }
        pRBOutput->RasterIO(GF_Write, 0, i, pRBOutput->GetXSize(), 1, pOutputLine, pRBOutput->GetXSize(), 1, (GDALDataType) this->GetGDALDataType(), 0, 0);
    }

    CPLFree(pTopLine);
    CPLFree(pBotLine);
    CPLFree(pOutputLine);

    return PROCESS_OK;
}

} // Namespace
