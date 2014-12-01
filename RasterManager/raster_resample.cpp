#include "raster.h"
#include "rastermanager_interface.h"
#include "gdal_priv.h"

#include <limits>
#include <math.h>
#include <string>

namespace RasterManager {

int Raster::ReSampleRaster(GDALRasterBand * pRBInput, GDALRasterBand * pRBOutput, double fNewCellSize, double fNewLeft, double fNewTop, int nNewRows, int nNewCols)
{
    // The properties of the original raster.
    double fOldLeft = GetLeft();
    double fOldYOrigin = GetTop();
    double fOldCellHeight = GetCellHeight();
    double fOldCellWidth = GetCellWidth();

    double dNoData = GetNoDataValue();

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
            pRBInput->RasterIO(GF_Read, 0, nOldTopRow, pRBInput->GetXSize(), 1, pTopLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
            pRBInput->RasterIO(GF_Read, 0, nOldBotRow, pRBInput->GetXSize(), 1, pBotLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);

            for (j = 0; j < nNewCols; j++)
            {
                pOutputLine[j] = dNoData;
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

                if (nOldLeftCol >= 0 && nOldLeftCol < pRBInput->GetXSize())
                {
                    double Z01 = pTopLine[nOldLeftCol];
                    double Z11 = pTopLine[nOldLeftCol + 1];

                    double Z00 = pBotLine[nOldLeftCol];
                    double Z10 = pBotLine[nOldLeftCol + 1];

                    // On some dirty rasters there seems to be a loss of precision in the way they show
                    // The nodata value from the raster is -3.402823e+38
                    // The Nodata value extracted from the Z val is -3.4028230607371e+38
                    // We'll cast everything down to a float just for the comparison

                    float fNoData = static_cast<float>(dNoData);

                    // Proceed with calculation if the input cells have valid data. this is true if the
                    // input raster does not possess a NoData value or if it does, and all the cells do
                    // not equal the missing data value.



                    if (!this->HasNoDataValue() ||
                               ((static_cast<float>(Z01) != fNoData)
                            && (static_cast<float>(Z11) != fNoData)
                            && (static_cast<float>(Z00) != fNoData)
                            && (static_cast<float>(Z10) != fNoData)))
                    {
                        double Lx = fOldX;
                        double Z1 = Z01 + (Z11 - Z01) * ((fNewX - Lx) / fOldCellWidth);
                        double Z0 = Z00 + (Z10 - Z00) * ((fNewX - Lx) / fOldCellWidth);

                        double Ty = fOldY;
                        double Z = Z1 - (Z1 - Z0) * ((fNewY - Ty) / fOldCellHeight);

                        pOutputLine[j] = Z;
                    }
                    else
                        pOutputLine[j] = dNoData;
                }
                else
                    pOutputLine[j] = dNoData;
            }
        }
        else
        {
            // Outside the bounds of the input image. Loop over all cells in current output row and set to NoData.
            for (j=0; j< nNewCols; j++)
            {
                pOutputLine[j] = dNoData;
            }
        }
        pRBOutput->RasterIO(GF_Write, 0, i, pRBOutput->GetXSize(), 1, pOutputLine, pRBOutput->GetXSize(), 1, GDT_Float64, 0, 0);
    }

    CPLFree(pTopLine);
    CPLFree(pBotLine);
    CPLFree(pOutputLine);

    return PROCESS_OK;
}


} // Namespace
