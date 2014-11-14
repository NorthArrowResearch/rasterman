#define MY_DLL_EXPORT

#include "dodraster.h"
#include "gdal_priv.h"

namespace RasterManager {


DoDRaster::DoDRaster(const char * psFilePath) : Raster(psFilePath)
{
}

void DoDRaster::GetChangeStats(double & fAreaErosion, double & fAreaDeposition, double & fVolErosion, double & fVolDeposition)
{
    // Set the output values to zero
    fAreaErosion = 0;
    fAreaDeposition = 0,
            fVolErosion = 0;
    fVolDeposition = 0;

    GDALAllRegister();

    GDALDataset * ds = (GDALDataset*) GDALOpen(this->FilePath(), GA_ReadOnly);
    if (ds == NULL)
        return;

    GDALRasterBand * pRB = ds->GetRasterBand(1);

    // Get the dataset cell size;
    double transform[6];
    ds->GetGeoTransform(transform);
    //double cellHeight = transform[5];
    double cellWidth = transform[1];
    double cellArea = cellWidth * cellWidth;

    int nSuccess;
    double fNoData = pRB->GetNoDataValue(&nSuccess);

    int i, j;
    double *pafScanline;
    pafScanline = (double *) CPLMalloc(sizeof(double)*GetCols());
    double fDoDValue = 0;

    for (i = 0; i < GetRows(); i++)
    {
        pRB->RasterIO(GF_Read, 0, i, GetCols(), 1, pafScanline, GetCols(), 1, GDT_Float64, 0, 0);

        for (j = 0; j < GetCols(); j++)
        {
            fDoDValue = pafScanline[j];
            if (fDoDValue != fNoData)
            {
                // Deposition
                if (fDoDValue > 0)
                {
                    fVolDeposition += (fDoDValue * cellArea);
                    fAreaDeposition += cellArea;
                }

                // Erosion
                if (fDoDValue < 0)
                {
                    fVolErosion += (fDoDValue * -1 * cellArea);
                    fAreaErosion += cellArea;
                }
            }
        }
    }

    CPLFree(pafScanline);
    GDALClose(ds);

    GDALDestroyDriverManager();
}

/*
 * PGB - 14 Aug 2014. Unable to get the detection of NoData in the propagated
 * error raster to work properly. So hacking it to simply look for positive
 * propagated error values. Do not use this method for any other raster types
 * than propagated error rasters.
 *
 */
void DoDRaster::GetChangeStats(Raster & pPropagatedError, double & fVolErosion, double & fVolDeposition)
{
    // Set the output values to zero
    fVolErosion = 0;
    fVolDeposition = 0;

    GDALAllRegister();

    GDALDataset * ds = (GDALDataset*) GDALOpen(this->FilePath(), GA_ReadOnly);
    if (ds == NULL)
        return;

    GDALRasterBand * pRB = ds->GetRasterBand(1);

    // Get the dataset cell size;
    double transform[6];
    ds->GetGeoTransform(transform);
    //double cellHeight = transform[5];
    double cellWidth = transform[1];
    double cellArea = cellWidth * cellWidth;

    int nSuccess;
    //float fNoData = (float) pRB->GetNoDataValue(&nSuccess);

    double *pafScanline;
    pafScanline = (double *) CPLMalloc(sizeof(double)*GetCols());

    //**********************************************************************************
    // Open the Mask

    GDALDataset * dsMask = (GDALDataset*) GDALOpen(pPropagatedError.FilePath(), GA_ReadOnly);
    if (dsMask == NULL)
        return;

    GDALRasterBand * pRBMask = dsMask->GetRasterBand(1);
    double fNoDataMask = pRBMask->GetNoDataValue(&nSuccess);
    double *pafScanlineMask;
    pafScanlineMask = (double *) CPLMalloc(sizeof(double)*GetCols());

    //**********************************************************************************
    // TODO: Confirm that the Mask and the member raster are the same size

    int i, j;
    double fPropErrorValue = 0;

    for (i = 0; i < GetRows(); i++)
    {
        pRB->RasterIO(GF_Read, 0, i, GetCols(), 1, pafScanline, GetCols(), 1, GDT_Float64, 0, 0);
        pRBMask->RasterIO(GF_Read, 0, i, GetCols(), 1, pafScanlineMask, GetCols(), 1, GDT_Float64, 0, 0);

        for (j = 0; j < GetCols(); j++)
        {
            fPropErrorValue = pafScanline[j];
            // See comment above method
            if (fPropErrorValue > 0)
            {
                if (pafScanlineMask[j] != fNoDataMask)
                {
                    // Deposition
                    if (pafScanlineMask[j] > 0)
                    {
                        fVolDeposition += (fPropErrorValue * cellArea);
                    }

                    // Erosion
                    if (pafScanlineMask[j] < 0)
                    {
                        fVolErosion += (fPropErrorValue * cellArea);
                    }
                }
            }
        }
    }

    CPLFree(pafScanline);
    CPLFree(pafScanlineMask);
    GDALClose(ds);
    GDALClose(dsMask);

    GDALDestroyDriverManager();
}

} // Namespace

extern "C" __declspec(dllexport) void GetDoDMinLoDStats(const char * ppszRawDoD, double fThreshold,
                                                     double * fAreaErosionRaw, double * fAreaDepositonRaw,
                                                     double * fAreaErosionThr, double * fAreaDepositionThr,
                                                     double * fVolErosionRaw, double * fVolDepositionRaw,
                                                     double * fVolErosionThr, double * fVolDepositionThr,
                                                     double * fVolErosionErr, double * fVolDepositonErr)
{
    // Set the output values to zero
    *fAreaErosionRaw = 0;
    *fAreaDepositonRaw = 0,
    *fAreaErosionThr = 0;
    *fAreaDepositionThr = 0;
    *fVolErosionRaw = 0;
    *fVolDepositionRaw = 0;
    *fVolErosionThr = 0;
    *fVolDepositionThr = 0;
    *fVolErosionErr = 0;
    *fVolDepositonErr = 0;

    long nRawErosionCount = 0;
    long nRawDepositionCount = 0;
    long nThrErosionCount = 0;
    long nThrDepositionCount = 0;

    GDALAllRegister();

    GDALDataset * ds = (GDALDataset*) GDALOpen(ppszRawDoD, GA_ReadOnly);
    if (ds == NULL)
        return;

    GDALRasterBand * pRB = ds->GetRasterBand(1);
    double GetCols = pRB->GetXSize();
    double GetRows = pRB->GetYSize();

    // Get the dataset cell size;
    double transform[6];
    ds->GetGeoTransform(transform);
    //double cellHeight = transform[5];
    double cellWidth = transform[1];
    double cellArea = cellWidth * cellWidth; //sqrt(pow(cellWidth, 2) + pow(cellHeight, 2));

    int nSuccess;
    double fNoData = pRB->GetNoDataValue(&nSuccess);

    int i, j;
    double *pafScanline;
    pafScanline = (double *) CPLMalloc(sizeof(double)*GetCols);
    double fDoDValue = 0;

    for (i = 0; i < GetRows; i++)
    {
        pRB->RasterIO(GF_Read, 0, i, GetCols, 1, pafScanline, GetCols, 1, GDT_Float64, 0, 0);

        for (j = 0; j < GetCols; j++)
        {
            fDoDValue = pafScanline[j];
            if (fDoDValue != fNoData)
            {
                // Deposition
                if (fDoDValue > 0)
                {
                    // Raw Deposition
                    *fVolDepositionRaw += fDoDValue ;
                    nRawDepositionCount += 1;

                    if (fDoDValue > fThreshold)
                    {
                        // Thresholded Deposition
                        *fVolDepositionThr += fDoDValue;
                        nThrDepositionCount += 1;
                    }
                }

                // Erosion
                if (fDoDValue < 0)
                {
                    // Raw Erosion
                    *fVolErosionRaw += fDoDValue * -1;
                    nRawErosionCount += 1;

                    if (fDoDValue < (fThreshold * -1))
                    {
                        // Thresholded Erosion
                        *fVolErosionThr += fDoDValue * -1;
                        nThrErosionCount += 1;
                    }
                }
            }
        }
    }

    // Areas are the count of cells multipled by the area of one cell
    *fAreaErosionRaw = nRawErosionCount * cellArea;
    *fAreaDepositonRaw  = nRawDepositionCount * cellArea;
    *fAreaErosionThr = nThrErosionCount * cellArea;
    *fAreaDepositionThr = nThrDepositionCount * cellArea;

    // Volumes are the accumulated change multiplied by the area of one cell
   *fVolErosionRaw = *fVolErosionRaw * cellArea;
   *fVolDepositionRaw = *fVolDepositionRaw * cellArea;
   *fVolErosionThr = *fVolErosionThr * cellArea;
   *fVolDepositionThr = *fVolDepositionThr * cellArea;

    // Error volumes are the area of change multipled by the Threshold
    *fVolErosionErr = *fAreaErosionThr * fThreshold;
    *fVolDepositonErr = *fAreaDepositionThr * fThreshold;

    CPLFree(pafScanline);

    GDALClose(ds);

    GDALDestroyDriverManager();
}

extern "C" __declspec(dllexport) void GetDoDPropStats(const char * ppszRawDoD, const char * ppszPropError,
                                                     double * fAreaErosionRaw, double * fAreaDepositonRaw,
                                                     double * fAreaErosionThr, double * fAreaDepositionThr,
                                                     double * fVolErosionRaw, double * fVolDepositionRaw,
                                                     double * fVolErosionThr, double * fVolDepositionThr,
                                                     double * fVolErosionErr, double * fVolDepositonErr)
{
    // Set the output values to zero
    *fAreaErosionRaw = 0;
    *fAreaDepositonRaw = 0,
    *fAreaErosionThr = 0;
    *fAreaDepositionThr = 0;
    *fVolErosionRaw = 0;
    *fVolDepositionRaw = 0;
    *fVolErosionThr = 0;
    *fVolDepositionThr = 0;
    *fVolErosionErr = 0;
    *fVolDepositonErr = 0;

    long nRawErosionCount = 0;
    long nRawDepositionCount = 0;
    long nThrErosionCount = 0;
    long nThrDepositionCount = 0;

    GDALAllRegister();

    // Open the raw DoD raster and determine it's size.
    GDALDataset * dsDod = (GDALDataset*) GDALOpen(ppszRawDoD, GA_ReadOnly);
    if (dsDod == NULL)
        return;
    GDALRasterBand * pRBDoD = dsDod->GetRasterBand(1);
    double GetCols = pRBDoD->GetXSize();
    double GetRows = pRBDoD->GetYSize();
    // Get the dataset cell size;
    double transform[6];
    dsDod->GetGeoTransform(transform);
    //double cellHeight = transform[5];
    double cellWidth = transform[1];
    double cellArea = cellWidth * cellWidth;

    // Open the propagated Error Raster and make sure the size matches the DoD
    GDALDataset * dsErr = (GDALDataset*) GDALOpen(ppszPropError, GA_ReadOnly);
    if (dsErr == NULL)
        return;

    GDALRasterBand * pRBErr = dsErr->GetRasterBand(1);
    double GetColsErr = pRBErr->GetXSize();
    double GetRowsErr = pRBErr->GetYSize();

    if ((GetCols != GetColsErr) || (GetRows != GetRowsErr))
        return;

    int nSuccess;
    double fNoDataDoD = pRBDoD->GetNoDataValue(&nSuccess);
    double fNoDataErr = pRBErr->GetNoDataValue(&nSuccess);

    int i, j;
    double * pafScanlineDoD = (double *) CPLMalloc(sizeof(double)*GetCols);
    double * pafScanlineErr = (double *) CPLMalloc(sizeof(double)*GetColsErr);

    double fDoDValue = 0;
    double fPropErr = 0;

    for (i = 0; i < GetRows; i++)
    {
        pRBDoD->RasterIO(GF_Read, 0, i, GetCols, 1, pafScanlineDoD, GetCols, 1, GDT_Float64, 0, 0);
        pRBErr->RasterIO(GF_Read, 0, i, GetCols, 1, pafScanlineErr, GetCols, 1, GDT_Float64, 0, 0);

        for (j = 0; j < GetCols; j++)
        {
            fDoDValue = pafScanlineDoD[j];
            fPropErr = pafScanlineErr[j];

            if ((fDoDValue != fNoDataDoD) && (fPropErr != fNoDataErr))
            {
                // Deposition
                if (fDoDValue > 0)
                {
                    // Raw Deposition
                    *fVolDepositionRaw += fDoDValue;
                    nRawDepositionCount += 1;

                    if (fDoDValue > fPropErr)
                    {
                        // Thresholded Deposition
                        *fVolDepositionThr += fDoDValue;
                        *fVolDepositonErr += fPropErr;
                        nThrDepositionCount += 1;
                    }
                }

                // Erosion
                if (fDoDValue < 0)
                {
                    // Raw Erosion
                    *fVolErosionRaw += fDoDValue * -1;
                    nRawErosionCount += 1;

                    if (fDoDValue < (fPropErr * -1))
                    {
                        // Thresholded Erosion
                        *fVolErosionThr += fDoDValue * -1;
                        *fVolErosionErr += fPropErr;
                        nThrErosionCount += 1;
                    }
                }
            }
        }
    }

    // Areas are the count of cells multipled by the area of one cell
    *fAreaErosionRaw = nRawErosionCount * cellArea;
    *fAreaDepositonRaw  = nRawDepositionCount * cellArea;
    *fAreaErosionThr = nThrErosionCount * cellArea;
    *fAreaDepositionThr = nThrDepositionCount * cellArea;

     // Volumes are the accumulated change multiplied by the area of one cell
    *fVolErosionRaw = *fVolErosionRaw * cellArea;
    *fVolDepositionRaw = *fVolDepositionRaw * cellArea;
    *fVolErosionThr = *fVolErosionThr * cellArea;
    *fVolDepositionThr = *fVolDepositionThr * cellArea;

    // Error volumes are the area of change multipled by the Threshold
    *fVolErosionErr = *fVolErosionErr * cellArea;
    *fVolDepositonErr = *fVolDepositonErr * cellArea;

    CPLFree(pafScanlineDoD);
    CPLFree(pafScanlineErr);

    GDALClose(dsDod);
    GDALClose(dsErr);

    GDALDestroyDriverManager();
}

extern "C" __declspec(dllexport) void GetDoDProbStats(const char * ppszRawDoD, const char * ppszThrDod,
                                                      const char * ppszPropError,
                                                      double * fAreaErosionRaw, double * fAreaDepositonRaw,
                                                      double * fAreaErosionThr, double * fAreaDepositionThr,
                                                      double * fVolErosionRaw, double * fVolDepositionRaw,
                                                      double * fVolErosionThr, double * fVolDepositionThr,
                                                      double * fVolErosionErr, double * fVolDepositonErr)
{
    // Get the area and volumes from the raw DoD
   RasterManager::DoDRaster raw(ppszRawDoD);
   raw.GetChangeStats(*fAreaErosionRaw, *fAreaDepositonRaw, *fVolErosionRaw, *fVolDepositionRaw);

   // Get the area and volumes from the thresholded DoD
   RasterManager::DoDRaster thr(ppszThrDod);
   thr.GetChangeStats(*fAreaErosionThr, *fAreaDepositionThr, *fVolErosionThr, *fVolDepositionThr);

   // Get the volume of error from the propagated error surface using the thresholded DoD as a mask
   RasterManager::DoDRaster err(ppszPropError);
   RasterManager::Raster thrTest(ppszThrDod);
   err.GetChangeStats(thrTest, *fVolErosionErr, *fVolDepositonErr);
}
