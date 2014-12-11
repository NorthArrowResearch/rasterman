#define MY_DLL_EXPORT

#include "raster.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "gdal_priv.h"
#include "rastermanager.h"
#include <limits>
#include <math.h>
#include <string>

namespace RasterManager {

int Raster::Slope(const char * psOutputSlope, int nSlpType){

    GDALDataset * pSlopeDS = CreateOutputDS(psOutputSlope, this);
    GDALDataset * pDemDS = (GDALDataset*) GDALOpen(m_sFilePath, GA_ReadOnly);

    const double PI = 3.14159265;
    double dzdx, dzdy, dzxy, riseRun;
    bool bCalculate;

    //allocate memory for reading from DEM and writing to slope
    double* fElev = (double*) CPLMalloc(sizeof(double) * 9);
    double* fSlope = (double*) CPLMalloc(sizeof(double) * GetCols());

    //Loop through each DEM cell to do the slope calculation,do not loop through edge cells
    for (int i=1; i< GetRows() -1; i++)
    {
        //assign NoData to first and last columns of the slope value array

        fSlope[0] = GetNoDataValue(), fSlope[ GetCols() - 1] = GetNoDataValue();

        for (int j=1; j < GetCols() - 1; j++)
        {
            //read 3x3 around cell (j,i)
            pDemDS->GetRasterBand(1)->RasterIO(GF_Read,j-1,i-1,3,3,fElev,3,3,GDT_Float64,0,0);
            bCalculate = true;
            //check for NoData values in 3x3
            for (int k=0; k<9; k++)
            {
                //if NoData value present in 3x3 do not calculate slope
                if (fElev[k] == GetNoDataValue())
                {
                    bCalculate = false;
                }
            }

            if (bCalculate)
            {
                dzdx = ((fElev[2]-fElev[0]) + ((2*fElev[5])-(2*fElev[3])) + (fElev[8]-fElev[6])) / (8* GetCellWidth());
                dzdy = ((fElev[0]-fElev[6]) + ((2*fElev[1])-(2*fElev[7])) + (fElev[2]-fElev[8])) / (8* GetCellWidth());
                dzxy = pow(dzdx, 2.0) + pow(dzdy, 2.0);
                riseRun = pow(dzxy, 0.5);
                if (nSlpType == SLOPE_DEGREES)
                {
                    fSlope[j] = atan(riseRun) * (180.0/PI);
                }
                else if (nSlpType == SLOPE_PERCENT)
                {
                    fSlope[j] = riseRun*100.0;
                }
            }
            else
            {
                fSlope[j] = GetNoDataValue();
            }
        }
        pSlopeDS->GetRasterBand(1)->RasterIO(GF_Write,0,i,GetCols(),1,fSlope,GetCols(),1,GDT_Float64,0,0);
    }

    CalculateStats(pSlopeDS->GetRasterBand(1));

    //close datasets
    GDALClose(pDemDS);
    GDALClose(pSlopeDS);

    //free allocated memory
    CPLFree(fElev);
    CPLFree(fSlope);
    fElev = NULL;
    fSlope = NULL;

    return PROCESS_OK;

}

}
