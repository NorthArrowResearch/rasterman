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

    // Split the string with delimiters into individual paths
    // Also check that those files exist, are concurrent AND orthogonal
    QList<QString> slRasters = RasterUnDelimit(csRasters, true, true, true);

    /************** SET UP THE OUTPUT DS **************/

    //Orthogonal and concurrent means we can set the output meta equal to the input
    RasterMeta OutputMeta(slRasters.at(0));

    // Create the output dataset for writing
    GDALDataset * pOutputDS = CreateOutputDS(psOutput, &OutputMeta);

    int sRasterRows = OutputMeta.GetRows();
    int sRasterCols = OutputMeta.GetCols();
    double dNoDataVal = OutputMeta.GetNoDataValue();

    // Step it down to char* for Rasterman and create+open an output file
    GDALRasterBand * pOutputRB = pOutputDS->GetRasterBand(1);
    double * pReadBuffer = (double*) CPLMalloc(sizeof(double) * sRasterCols);

    // We store all the datasets in a hash
    QHash<int, GDALRasterBand *> dDatasets;
    QHash<int, double *> dInBuffers;

    // Populate the hashes with enough buffers and datasets
    int counter = 0;
    foreach (QString raster, slRasters) {
        counter++;
        // Here is the corresponding input raster, added as a hash to a dataset
        const QByteArray sHSIOutputQB = raster.toLocal8Bit();
        GDALDataset * pInputDS = (GDALDataset*) GDALOpen( sHSIOutputQB.data(), GA_ReadOnly);
        GDALRasterBand * pInputRB = pInputDS->GetRasterBand(1);

        // Add a buffer for reading this input
        double * pReadBuffer = (double*) CPLMalloc(sizeof(double) * sRasterCols);

        // Notice these get the same keys.
        dDatasets.insert(counter, pInputRB);
        dInBuffers.insert(counter, pReadBuffer);
    }

    // Loop over rows
    for (int i=0; i < sRasterRows; i++)
    {
        // Populate the buffers with a new line from each file.
        QHashIterator<int, GDALRasterBand *> dDatasetIterator(dDatasets);
        while (dDatasetIterator.hasNext()) {
            dDatasetIterator.next();
            // Read the row
            dDatasetIterator.value()->RasterIO(GF_Read, 0,  i, sRasterCols, 1, dInBuffers.value(dDatasetIterator.key()),
                                               sRasterCols, 1, GDT_Float64, 0, 0);
        }
        // Loop over columns
        for (int j=0; j < sRasterCols; j++)
        {
            QHash<int, double> dCellContents;
            QHashIterator<int, double *> QHIBIterator(dInBuffers);
            while (QHIBIterator.hasNext()) {
                QHIBIterator.next();
                dCellContents.insert(QHIBIterator.key(), QHIBIterator.value()[j]);
            }
            pReadBuffer[j] = PerformStackStatOp(eOperation, dCellContents, dNoDataVal);
        }
        // Write the row
        pOutputRB->RasterIO(GF_Write, 0, i, sRasterCols, 1, pReadBuffer, sRasterCols, 1, GDT_Float64, 0, 0 );
    }

    if ( pOutputDS != NULL)
        GDALClose(pOutputDS);
    CPLFree(pReadBuffer);
    pReadBuffer = NULL;

    // Let's remember to clean up the inputs
    QHashIterator<int, GDALRasterBand *> qhds(dDatasets);
    while (qhds.hasNext()) {
        qhds.next();
        GDALClose(qhds.value());
    }
    dDatasets.clear();
    QHashIterator<int, double *> qhbuff(dInBuffers);
    while (qhbuff.hasNext()) {
        qhbuff.next();
        CPLFree(qhbuff.value());
    }
    dInBuffers.clear();

    return PROCESS_OK;

}


double Raster::PerformStackStatOp(Raster_Stats_Operation eOp,
                                   QHash<int, double> dCellContents,
                                   double dNoDataVal){

    if (dCellContents.size() == 0)
        return dNoDataVal;

    // Get the easy stuff out of the way first
    switch (eOp) {
    case STATS_MAXIMUM:  return StatStackMax(dCellContents, dNoDataVal); break;
    case STATS_MINIMUM:  return StatStackMin(dCellContents, dNoDataVal); break;
    case STATS_MEAN:     return StatStackMean(dCellContents, dNoDataVal); break;
    case STATS_STD:      return StatStackSTDev(dCellContents, dNoDataVal); break;
    case STATS_RANGE:
        double dMax = StatStackMax(dCellContents, dNoDataVal);
        double dMin = StatStackMin(dCellContents, dNoDataVal);
        return  dMax - dMin;
        break;
    }
    return dNoDataVal;


}


int Raster::RasterStat(Raster_Stats_Operation eOperation, double * pdResult){

    // Get the easy stuff out of the way first
    switch (eOperation) {
    case STATS_MAXIMUM:
        *pdResult = m_dRasterMax;
        return PROCESS_OK;
        break;
    case STATS_MINIMUM:
        *pdResult = m_dRasterMin;
        return PROCESS_OK;
        break;
    case STATS_MEAN:
        *pdResult = m_dRasterMean;
        return PROCESS_OK;
        break;
    case STATS_STD:
        *pdResult = m_dRasterStdDev;
        return PROCESS_OK;
        break;
    case STATS_RANGE:
        *pdResult = m_dRasterMax - m_dRasterMin;
        return PROCESS_OK;
        break;
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

double Raster::StatStackMax(QHash<int, double> dCellContents,
                            double dNoDataVal){

    QHashIterator<int, double> x(dCellContents);
    double dProd = x.value();
    while (x.hasNext()) {
        x.next();

        if (x.value() > dProd)
            dProd = x.value();

        // If anything is NoData then that's the return
        if (x.value() == dNoDataVal)
            return dNoDataVal;
    }
    return dProd;

}

double Raster::StatStackMin(QHash<int, double> dCellContents,
                            double dNoDataVal){

    QHashIterator<int, double> x(dCellContents);
    double dProd = x.value();
    while (x.hasNext()) {
        x.next();

        if (x.value() < dProd)
            dProd = x.value();

        // If anything is NoData then that's the return
        if (x.value() == dNoDataVal)
            return dNoDataVal;
    }
    return dProd;

}

double Raster::StatStackMean(QHash<int, double> dCellContents,
                             double dNoDataVal){

    QHashIterator<int, double> x(dCellContents);
    double dProd = 0;
    while (x.hasNext()) {
        x.next();
        dProd += x.value();

        // If anything is NoData then that's the return
        if (x.value() == dNoDataVal)
            return dNoDataVal;

    }
    return dProd / dCellContents.size();
}

double Raster::StatStackSTDev(QHash<int, double> dCellContents,
                              double dNoDataVal){

    double dAverage = StatStackMean(dCellContents, dNoDataVal);
    if (dAverage == dNoDataVal)
        return dNoDataVal;

    QHashIterator<int, double> x(dCellContents);
    double dStdDev = 0;
    while (x.hasNext()) {
        x.next();
        dStdDev += pow( (x.value() - dAverage), 2);

        // If anything is NoData then that's the return
        if (x.value() == dNoDataVal)
            return dNoDataVal;

        dStdDev *= x.value();
    }

    // Return the square root of the whole mess.
    return pow(dStdDev / dCellContents.size() , 0.5);

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
