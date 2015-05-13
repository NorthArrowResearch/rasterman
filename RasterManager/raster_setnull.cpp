#define MY_DLL_EXPORT

#include "raster.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "gdal_priv.h"
#include "rastermanager.h"
#include <QString>

namespace RasterManager {

int Raster::SetNull(const char * psOutputSlope, const char * psOperator, double dThresh1, double dThresh2){

    int nOpType;
    QString sType(psOperator);
    if (sType.compare(sType, "above", Qt::CaseInsensitive) == 0)
        nOpType = SETNULL_ABOVE;
    else if (sType.compare(sType, "below", Qt::CaseInsensitive) == 0){
        nOpType = SETNULL_BELOW;
    }
    else if (sType.compare(sType, "between", Qt::CaseInsensitive) == 0){
        nOpType = SETNULL_BETWEEN;
    }
    else if (sType.compare(sType, "value", Qt::CaseInsensitive) == 0){
        nOpType = SETNULL_VALUE;
    }
    else{
        throw RasterManagerException( MISSING_ARGUMENT, "Could not detect a valid setnull Operation.");
    }

    // Open up the Input File
    GDALDataset * pInputDS = (GDALDataset*) GDALOpen(m_sFilePath, GA_ReadOnly);
    if (pInputDS == NULL)
        throw RasterManagerException( INPUT_FILE_ERROR, "Input file could not be opened");

    GDALRasterBand * pRBInput = pInputDS->GetRasterBand(1);

    // Create the output dataset for writing
    GDALDataset * pOutputDS = CreateOutputDS(psOutputSlope, this);
    GDALRasterBand * pOutputRB = pOutputDS->GetRasterBand(1);

    // Assign our buffers
    double * pInputLine = (double*) CPLMalloc(sizeof(double) * GetCols());
    double * pOutputLine = (double*) CPLMalloc(sizeof(double) * GetCols());

    // Loop over rows
    for (int i=0; i < GetRows(); i++)
    {
        // Populate the buffer
        pRBInput->RasterIO(GF_Read, 0,  i, GetCols(), 1, pInputLine, GetCols(), 1, GDT_Float64, 0, 0);

        // Loop over columns
        for (int j=0; j < GetCols(); j++)
        {
            if ((nOpType == SETNULL_ABOVE && pInputLine[j] > dThresh1) ||
                    (nOpType == SETNULL_BELOW && pInputLine[j] < dThresh1) ||
                    (nOpType == SETNULL_BETWEEN && (pInputLine[j] < dThresh1 || pInputLine[j] > dThresh2) ) ||
                    (nOpType == SETNULL_VALUE && pInputLine[j] == dThresh1 )){
                pOutputLine[j] = GetNoDataValue();
            }
            else {
                pOutputLine[j] = pInputLine[j];
            }

        }
        // Write the row
        pOutputRB->RasterIO(GF_Write, 0, i, GetCols(), 1, pOutputLine, GetCols(), 1, GDT_Float64, 0, 0 );
    }

    CPLFree(pOutputLine);
    CPLFree(pInputLine);

    CalculateStats(pOutputDS->GetRasterBand(1));

    if ( pInputDS != NULL)
        GDALClose(pInputDS);
    if ( pOutputDS != NULL)
        GDALClose(pOutputDS);

    return PROCESS_OK;


}

}
