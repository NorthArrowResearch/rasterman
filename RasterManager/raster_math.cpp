#define MY_DLL_EXPORT
/*
 * Do Basic Raster Math Operations
 *
 * 6 December 2014
 *
*/
#include "raster.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "rastermanager.h"

#include <QtCore>
#include <QString>



namespace RasterManager {

int Raster::RasterMath(const char * psRaster1,
               const char * psRaster2,
               const double * dNumericArg,
               const char * psOperation,
               const char * psOutput){

    int iOperation = GetMathOpFromString(psOperation);

    if (iOperation < 0)
        return NO_OPERATION_SPECIFIED;

    // Everything except square root needs at least one other parameter (raster or doube)
    if (psRaster2 == NULL && dNumericArg == NULL && iOperation != RM_BASIC_MATH_SQRT)
        return MISSING_ARGUMENT;

    /*****************************************************************************************
     * Raster 1
     */
    CheckFile(psRaster1, true);

    RasterMeta rmRasterMeta1(psRaster1);

    GDALDataset * pDS1 = (GDALDataset*) GDALOpen(psRaster1, GA_ReadOnly);
    if (pDS1 == NULL)
        return INPUT_FILE_ERROR;

    GDALRasterBand * pRBInput1 = pDS1->GetRasterBand(1);

    double * pInputLine1 = (double *) CPLMalloc(sizeof(double)*rmRasterMeta1.GetCols());

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */
    RasterMeta rmOutputMeta;
    rmOutputMeta = rmRasterMeta1;

    // Decision: We can't mix types here so the output will always be double
    GDALDataType outDataType = GDT_Float64;
    rmOutputMeta.SetGDALDataType(&outDataType);

    double fNoDataValue = (double) -std::numeric_limits<float>::max();
    rmOutputMeta.SetNoDataValue(&fNoDataValue);

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, &rmOutputMeta);

    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*rmOutputMeta.GetCols());

    /*****************************************************************************************
     * Raster 2 to be used
     */
    if (psRaster2 != NULL){

        CheckFile(psRaster2, true);

        GDALDataset * pDS2 = (GDALDataset*) GDALOpen(psRaster2, GA_ReadOnly);
        if (pDS1 == NULL)
            return INPUT_FILE_ERROR;

        RasterMeta rmRasterMeta2(psRaster2);

        GDALRasterBand * pRBInput2 = pDS2->GetRasterBand(1);

        /*****************************************************************************************
         * Check that input rasters have the same numbers of rows and columns
         */

        if (pRBInput1->GetXSize() != pRBInput2->GetXSize())
            return COLS_ERROR;

        if (pRBInput1->GetYSize() != pRBInput2->GetYSize())
            return ROWS_ERROR;

        if (psOutput == NULL)
            return OUTPUT_FILE_MISSING;

        double * pInputLine2 = (double *) CPLMalloc(sizeof(double)*rmRasterMeta2.GetCols());

        int i, j;
        for (i = 0; i < rmOutputMeta.GetRows(); i++)
        {
            pRBInput1->RasterIO(GF_Read, 0,  i, rmRasterMeta1.GetCols(), 1, pInputLine1, rmRasterMeta1.GetCols(), 1, GDT_Float64, 0, 0);
            pRBInput2->RasterIO(GF_Read, 0,  i, rmRasterMeta2.GetCols(), 1, pInputLine2, rmRasterMeta2.GetCols(), 1, GDT_Float64, 0, 0);

            for (j = 0; j < rmOutputMeta.GetCols(); j++)
            {
                if ( (pInputLine1[j] == rmRasterMeta1.GetNoDataValue()) ||
                     (pInputLine2[j] == rmRasterMeta2.GetNoDataValue()) )
                {
                    pOutputLine[j] = rmOutputMeta.GetNoDataValue();
                }
                else
                {
                    if (iOperation == RM_BASIC_MATH_ADD)
                        pOutputLine[j] = pInputLine1[j] + pInputLine2[j];
                    else if (iOperation == RM_BASIC_MATH_SUBTRACT)
                        pOutputLine[j] = pInputLine1[j] - pInputLine2[j];
                    else if (iOperation == RM_BASIC_MATH_MULTIPLY)
                        pOutputLine[j] = pInputLine1[j] * pInputLine2[j];
                    else if (iOperation == RM_BASIC_MATH_DIVIDE){
                        // Remember to cover the divide by zero case
                        if (psRaster2 != 0)
                            pOutputLine[j] = pInputLine1[j] / pInputLine2[j];
                        else
                            pOutputLine[j] = fNoDataValue;
                    }
                    else if (iOperation == RM_BASIC_MATH_THRESHOLD_PROP_ERROR){
                        if (abs(pInputLine1[j]) > pInputLine2[j]){
                            pOutputLine[j] = pInputLine1[j];
                        }
                        else{
                            pOutputLine[j] = fNoDataValue;
                        }
                    }
                    else
                        return MISSING_ARGUMENT;
                }
            }

            pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, rmOutputMeta.GetCols(), 1, pOutputLine, rmOutputMeta.GetCols(), 1, GDT_Float64, 0, 0);
        }
        CPLFree(pInputLine2);


    }
    else if (dNumericArg != NULL || iOperation == RM_BASIC_MATH_SQRT){
        /*****************************************************************************************
        * Numerical Value to be used
        */

        int i, j;
        for (i = 0; i < rmOutputMeta.GetRows(); i++)
        {
            pRBInput1->RasterIO(GF_Read, 0,  i, rmRasterMeta1.GetCols(), 1, pInputLine1, rmRasterMeta1.GetCols(), 1, GDT_Float64, 0, 0);

            for (j = 0; j < pRBInput1->GetXSize(); j++)
            {
                if (pInputLine1[j] == rmRasterMeta1.GetNoDataValue())
                {
                    pOutputLine[j] = fNoDataValue;
                }
                else
                {
                    if (iOperation == RM_BASIC_MATH_ADD)
                        pOutputLine[j] = pInputLine1[j] + *dNumericArg;

                    else if (iOperation == RM_BASIC_MATH_SUBTRACT)
                        pOutputLine[j] = pInputLine1[j] - *dNumericArg;

                    else if (iOperation == RM_BASIC_MATH_MULTIPLY)
                        pOutputLine[j] = pInputLine1[j] * (*dNumericArg);

                    else if (iOperation == RM_BASIC_MATH_DIVIDE){
                        // Remember to cover the divide by zero case
                        if(dNumericArg != 0)
                            pOutputLine[j] = pInputLine1[j] / *dNumericArg;
                        else
                            pOutputLine[j] = fNoDataValue;
                    }
                    else if (iOperation == RM_BASIC_MATH_POWER){
                        // We're throwing away imaginary numbers
                        if (dNumericArg >= 0)
                            pOutputLine[j] = pow(pInputLine1[j], *dNumericArg);
                        else
                            pOutputLine[j] = fNoDataValue;
                    }
                    else if (iOperation == RM_BASIC_MATH_SQRT){
                        // Throw away imaginary numbers
                        if (pInputLine1[j] >= 0)
                            pOutputLine[j] = sqrt(pInputLine1[j]);
                        else
                            pOutputLine[j] = fNoDataValue;
                    }
                    else
                        return MISSING_ARGUMENT;
                }
            }
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, rmOutputMeta.GetCols(), 1, pOutputLine,
                                                  rmOutputMeta.GetCols(), 1,
                                                  GDT_Float64, 0, 0);
        }
    }
    CPLFree(pInputLine1);
    CPLFree(pOutputLine);

    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDS1);
    GDALClose(pDSOutput);


    return PROCESS_OK;

}


}
