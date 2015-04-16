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

int Raster::StackStats(Raster_Stats_Operation eOperation, const char * csRasters, const char * psOutput){
    // Check for input and output files
    CheckFile(psOutput, false);

    //TODO: handle concurrency and orthogonality

    // Split the string with delimiters into individual paths
    // Also check that those files exist
    QList<QString> slRasters = RasterUnDelimit(csRasters, true, false, false);
    RasterMeta * OutputMeta = new RasterMeta(slRasters.at(0));
    RasterMetaExpand(slRasters, OutputMeta);


    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, OutputMeta);

    //projectionRef use from inputs.

    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*OutputMeta->GetCols());

    /*****************************************************************************************
     * Loop over the inputs and then the rows and columns
     *
     */
    foreach(QString raster, slRasters){

        const QByteArray qbRasterFilePath = raster.toLocal8Bit();
        GDALDataset * pDS = (GDALDataset*) GDALOpen(qbRasterFilePath.data(), GA_ReadOnly);
        GDALRasterBand * pRBInput = pDS->GetRasterBand(1);

        RasterMeta inputMeta (qbRasterFilePath.data());
        // We need to figure out where in the output the input lives.
        int trans_i = OutputMeta->GetRowTranslation(&inputMeta);
        int trans_j = OutputMeta->GetColTranslation(&inputMeta);

        double * pInputLine = (double *) CPLMalloc(sizeof(double)*pRBInput->GetXSize());

        for (int i = 0; i < pRBInput->GetYSize(); i++){
            pRBInput->RasterIO(GF_Read, 0,  i, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
            // here's where we need to get the correct row of the output. Replace
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Read, 0,  trans_i+i, OutputMeta->GetCols(), 1, pOutputLine, OutputMeta->GetCols(), 1, GDT_Float64, 0, 0);

            for (int j = 0; j < pRBInput->GetXSize(); j++){
                // If the input line is empty then do nothing
                if ( (pInputLine[j] != inputMeta.GetNoDataValue())
                     && pOutputLine[trans_j+j] ==  OutputMeta->GetNoDataValue())
                {
//                    switch (eOperation) {
//                    case STATS_MEDIAN:   return RasterStatMedian(&RasterArray); break;
//                    case STATS_MAJORITY: return RasterStatMajority(&RasterArray); break;
//                    case STATS_MINORITY: return RasterStatMinority(&RasterArray); break;
//                    case STATS_SUM:      return RasterStatSum(&RasterArray); break;
//                    case STATS_VARIETY:  return RasterStatVariety(&RasterArray); break;
//                    case STATS_RANGE:    return RasterStatRange(&RasterArray); break;
//                    default: break;
//                    }
                    pOutputLine[trans_j+j] = pInputLine[j];
                }
            }
            // here's where we need to get the correct row of the output. Replace
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  trans_i+i, OutputMeta->GetCols(), 1, pOutputLine, OutputMeta->GetCols(), 1, GDT_Float64, 0, 0);

        }
        CPLFree(pInputLine);
    }





    /*****************************************************************************************
     * Now Close everything and clean it all up
     */
    CPLFree(pOutputLine);
    CalculateStats(pDSOutput->GetRasterBand(1));
    GDALClose(pDSOutput);

    delete OutputMeta;

    return PROCESS_OK;
}

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

    // TODO: Everything below this point is unimplemented
    throw RasterManagerException(OTHER_ERROR, "Feature not implemented");

    // Some of these take a little more math.
    int nNumCells = GetCols() * GetRows();
    std::vector<double> RasterArray;
    RasterArray.resize(nNumCells);

    // Setup our Read buffer and read the entire raster into an array
    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(FilePath(), GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException(INPUT_FILE_ERROR, "Could not open input Raster");

    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    double * pInputLine = (double *) CPLMalloc(sizeof(double)*GetCols());
    for (int nRow = 0; nRow < pRBInput->GetYSize(); nRow++){
        pRBInput->RasterIO(GF_Read, 0,  nRow, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
        for (int nCol = 0; nCol < pRBInput->GetXSize(); nCol++){
            RasterArray.at( (nRow * GetCols() ) + nCol) = pInputLine[nCol];
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


}

double Raster::RasterStatMedian(std::vector<double> * RasterArray){
    // Allocate an array of the same size and sort it.
//    for (int i = 0; i < iSize; ++i) {
//        dpSorted[i] = daArray[i];
//    }
//    for (int i = iSize - 1; i > 0; --i) {
//        for (int j = 0; j < i; ++j) {
//            if (dpSorted[j] > dpSorted[j+1]) {
//                double dTemp = dpSorted[j];
//                dpSorted[j] = dpSorted[j+1];
//                dpSorted[j+1] = dTemp;
//            }
//        }
//    }

//    // Middle or average of middle values in the sorted array.
//    double dMedian = 0.0;
//    if ((iSize % 2) == 0) {
//        dMedian = (dpSorted[iSize/2] + dpSorted[(iSize/2) - 1])/2.0;
//    } else {
//        dMedian = dpSorted[iSize/2];
//    }
//    delete [] dpSorted;
//    return dMedian;
    return -1;
}
double Raster::RasterStatMajority(std::vector<double> * RasterArray){
    return -1;
}
double Raster::RasterStatMinority(std::vector<double> * RasterArray){
    return -1;
}
double Raster::RasterStatSum(std::vector<double> * RasterArray){
    double total = 0;
    for (int i = 0; i < (int)RasterArray->size(); i++){
        total += RasterArray->at(i);
    }
    return total;
}
double Raster::RasterStatVariety(std::vector<double> * RasterArray){
    return -1;
}
double Raster::RasterStatRange(std::vector<double> * RasterArray){
    return -1;
}

}
