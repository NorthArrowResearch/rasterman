#define MY_DLL_EXPORT
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "raster.h"
#include "rastermeta.h"
#include "rastermanager.h"
#include "gdal.h"
#include "gdal_priv.h"

/*
 * Raster Combine -- Combine multiple rasters using a particular Method
 *
 * 28 February 2015
 *
*/
\
namespace RasterManager {

int Raster::CombineRaster(
        const char * psInputRasters,
        const char * psOutputRaster,
        const char * psOperation
        ){

    // Check for input and output files
    CheckFile(psOutputRaster, false);

    RasterManagerCombineOperations eOp;

    if (QString(psOperation).compare("multiply", Qt::CaseInsensitive) == 0){
        //Mean is the only operation we currently supportl
        eOp = COMBINE_MULTIPLY;
    }
    else{
        throw RasterManagerException(ARGUMENT_VALIDATION, QString("Operation argument was invalid: %1").arg(psOperation) );
    }

    // Split the string with delimiters into individual paths
    // Also check that those files exist, are concurrent AND orthogonal
    QList<QString> slRasters = RasterUnDelimit(psInputRasters, true, true, true);

    /************** SET UP THE OUTPUT DS **************/

    //Orthogonal and concurrent means we can set the output meta equal to the input
    RasterMeta OutputMeta(slRasters.at(0));

    // Create the output dataset for writing
    GDALDataset * pOutputDS = CreateOutputDS(psOutputRaster, &OutputMeta);

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
            pReadBuffer[j] = CombineRasterValues(eOp, dCellContents, dNoDataVal);
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


double Raster::CombineRasterValues(RasterManagerCombineOperations eOp,
                                   QHash<int, double> dCellContents,
                                   double dNoDataVal){

    if (dCellContents.size() == 0)
        return dNoDataVal;

    switch (eOp) {
    case COMBINE_MULTIPLY:
        return CombineRasterValuesMultiply(dCellContents, dNoDataVal);
        break;
    default:
        return dNoDataVal;
        break;
    }

}

double Raster::CombineRasterValuesMultiply(QHash<int, double> dCellContents,
                                           double dNoDataVal){
    double dProd = 1;
    QHashIterator<int, double> x(dCellContents);
    while (x.hasNext()) {
        x.next();

        // If anything is NoData then that's the return
        if (x.value() == dNoDataVal)
            return dNoDataVal;

        dProd *= x.value();
    }
    return dProd;
}

};
