#define MY_DLL_EXPORT

#include "gdal_priv.h"
#include "raster.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "gdalgrid.h"
#include "rastermanager.h"

#include <math.h>
#include <QFile>
#include <QStringList>

namespace RasterManager {

double Raster::RasterStat(Raster_Stats_Operation eOperation){

    // Get the easy stuff out of the way first
    switch (eOperation) {
    case STATS_MAXIMUM:  return m_dRasterMax; break;
    case STATS_MINIMUM:  return m_dRasterMin; break;
    case STATS_MEAN:     return m_dRasterMean; break;
    case STATS_STD:      return m_dRasterStdDev; break;
    case STATS_RANGE:    return m_dRasterMax - m_dRasterMin; break;
    default: break;
    }

    // Some of these take a little more math.
    int nNumCells = GetCols() * GetRows();
    std::vector<double> RasterArray;
    RasterArray.resize(nNumCells);

    // Setup our Read buffer and read the entire raster into an array
    double * pInputLine = (double *) CPLMalloc(sizeof(double)*rInputRaster->GetCols());
    for (int nRow = 0; nRow < pRBInput->GetYSize(); nRow++){
        pRBInput->RasterIO(GF_Read, 0,  nRow, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
        for (int nCol = 0; nCol < pRBInput->GetXSize(); nCol++){
            RasterArray.
            RasterArray.at(nIndex) = pInputLine[nCol];
        }
    }
    CPLFree(pInputLine);

    switch (eOperation) {
    case STATS_MEDIAN:   return RasterStatMedian(&RasterArray); break;
    case STATS_MAJORITY: return RasterStatMajority(&RasterArray); break;
    case STATS_MINORITY: return RasterStatMinority(&RasterArray); break;
    case STATS_SUM:      return RasterStatSum(&RasterArray); break;
    case STATS_VARIETY:  return RasterStatVariety(&RasterArray); break;
    case STATS_RANGE:    return RasterStatRange(&RasterArray); break;
    default: break;
    }
    return PROCESS_OK;
}

double Raster::RasterStatMedian(std::vector<double> * RasterArray){
    // Allocate an array of the same size and sort it.
    for (int i = 0; i < iSize; ++i) {
        dpSorted[i] = daArray[i];
    }
    for (int i = iSize - 1; i > 0; --i) {
        for (int j = 0; j < i; ++j) {
            if (dpSorted[j] > dpSorted[j+1]) {
                double dTemp = dpSorted[j];
                dpSorted[j] = dpSorted[j+1];
                dpSorted[j+1] = dTemp;
            }
        }
    }

    // Middle or average of middle values in the sorted array.
    double dMedian = 0.0;
    if ((iSize % 2) == 0) {
        dMedian = (dpSorted[iSize/2] + dpSorted[(iSize/2) - 1])/2.0;
    } else {
        dMedian = dpSorted[iSize/2];
    }
    delete [] dpSorted;
    return dMedian;
}
double Raster::RasterStatMajority(std::vector<double> * RasterArray){

}
double Raster::RasterStatMinority(std::vector<double> * RasterArray){

}
double Raster::RasterStatSum(std::vector<double> * RasterArray){

}
double Raster::RasterStatVariety(std::vector<double> * RasterArray){

}
double Raster::RasterStatRange(std::vector<double> * RasterArray){

}

}
