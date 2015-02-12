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

int Raster::Hillshade(const char * psOutputHillshade){

    // This is a byte Raster so the nodataval must be 0
    RasterMeta OutputRasterMeta(m_sFilePath);
    OutputRasterMeta.SetNoDataValue(0);
    GDALDataType nDtype = GDT_Byte;
    OutputRasterMeta.SetGDALDataType(&nDtype);

    GDALDataset * pDemDS = (GDALDataset*) GDALOpen(m_sFilePath, GA_ReadOnly);
    GDALDataset * pHsDS = CreateOutputDS(psOutputHillshade, &OutputRasterMeta);

    const double PI = 3.14159265;
    double dzdx, dzdy, azimuthRad, slopeRad, aspectRad;
    int hlsdByte;
    bool bCalculate;

    //setup azimuth and zenith variables
    double azimuth = 315.0, zFactor = 1.0, altDeg = 45.0;
    double zenDeg = 90.0 - altDeg;
    double zenRad = zenDeg * PI / 180.0;
    double azimuthMath = 360.0 - azimuth + 90.0;

    if (azimuthMath >= 360.0)
    {
        azimuthMath = azimuthMath - 360.0;
    }
    azimuthRad = azimuthMath * PI / 180.0;

    //allocate memory for reading from DEM and writing to hillshade
    double *fElev = (double*) CPLMalloc(sizeof(double)*9);
    unsigned char *hlsd = (unsigned char*) CPLMalloc(sizeof(int)*GetCols());

    //loop through each DEM cell and do the hillshade calculation, do not loop through edge cells
    for (int i=1; i < GetRows() - 1; i++)
    {
        //assign no data for first and last positions in the row
        hlsd[0] = GetNoDataValue(), hlsd[ GetCols() - 1 ] = GetNoDataValue();
        for (int j=1; j < GetCols() - 1; j++)
        {
            //read 3x3 from DEM dataset
            pDemDS->GetRasterBand(1)->RasterIO(GF_Read,j-1,i-1,3,3,fElev,3,3,GDT_Float64,0,0);
            bCalculate = true;
            for (int k=0; k<9; k++)
            {
                //if NoData value present in 3x3 do not calculate hillshade
                if (fElev[k] == GetNoDataValue())
                {
                    bCalculate = false;
                }
            }

            if (bCalculate)
            {
                dzdx = ((fElev[2]+(2*fElev[5])+fElev[8]) - (fElev[0]+(2*fElev[3])+fElev[6])) / (8* GetCellWidth());
                dzdy = ((fElev[6]+(2*fElev[7])+fElev[8]) - (fElev[0]+(2*fElev[1])+fElev[2])) / (8* GetCellWidth());
                slopeRad = atan(zFactor * sqrt(pow(dzdx, 2.0) + pow(dzdy, 2.0)));

                if (dzdx != 0.0)
                {
                    aspectRad = atan2(dzdy, (dzdx*(-1.0)));
                    if (aspectRad < 0.0)
                    {
                        aspectRad = 2.0 * PI + aspectRad;
                    }
                }
                else
                {
                    if (dzdy > 0.0)
                    {
                        aspectRad = PI / 2.0;
                    }
                    else if (dzdy < 0.0)
                    {
                        aspectRad = 2.0*PI - PI/2.0;
                    }
                    else
                    {
//                        aspectRad = aspectRad;
                    }
                }
                hlsdByte = roundf(254 * ((cos(zenRad) * cos(slopeRad)) + (sin(zenRad) * sin(slopeRad) * cos(azimuthRad - aspectRad)))) + 1.0;
                hlsd[j] = hlsdByte;
            }
            else
            {
                hlsd[j] = OutputRasterMeta.GetNoDataValue();
            }
        }
        pHsDS->GetRasterBand(1)->RasterIO(GF_Write,0,i,GetCols(),1,hlsd,GetCols(),1,GDT_Byte,0,0);
    }

    CalculateStats(pHsDS->GetRasterBand(1));

    //close datasets
    GDALClose(pDemDS);
    GDALClose(pHsDS);

    CPLFree(fElev);
    CPLFree(hlsd);
    fElev = NULL;
    hlsd = NULL;

    return PROCESS_OK;
}

}
