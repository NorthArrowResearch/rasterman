#define MY_DLL_EXPORT
/*
 * Raster Mosaic Operations
 *
 * 6 December 2014
 *
*/
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "raster.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "rastermanager.h"



namespace RasterManager {

int Raster::RasterMosaic(const char * csRasters, const char * psOutput)
{
    // Loop through the strings, delimited by ;
    std::string RasterFileName, RasterFiles(csRasters), RasterFilesToken;

    // Terminate with a semicolon if it hasn't already been done
    if (!EndsWith(psOutput, ";")){
        RasterFiles.append(";");
    }
    RasterFilesToken = RasterFiles;

    // The output raster info
    RasterMeta OutputMeta;

    double fNoDataValue = (double) -std::numeric_limits<float>::max();

    /*****************************************************************************************
     * Open all the relevant files and figure out the bounds of the final file.
     */
    int counter = 0;
    while(RasterFilesToken != ""){
        counter++;
        RasterFileName = RasterFilesToken.substr(0,RasterFilesToken.find_first_of(";"));
        RasterFilesToken = RasterFilesToken.substr(RasterFilesToken.find_first_of(";") + 1);

        if (RasterFileName.c_str() == NULL)
            return INPUT_FILE_ERROR;

        CheckFile(RasterFileName.c_str(), true);

        RasterMeta erRasterInput (RasterFileName.c_str());

        // First time round set the bounds to the first raster we give it.
        if (counter==1){
            OutputMeta = erRasterInput;
            OutputMeta.SetNoDataValue(&fNoDataValue);
            GDALDataType nDType = GDT_Float32;
            OutputMeta.SetGDALDataType(&nDType);
        }
        else{
            OutputMeta.Union(&erRasterInput);
        }
    }

    /*****************************************************************************************
     * The default output type is 32 bit floating point.
     */

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, &OutputMeta);

    //projectionRef use from inputs.

    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*OutputMeta.GetCols());

    /*****************************************************************************************
     * Loop over the inputs and then the rows and columns
     *
     */
    int i,j;
    RasterFilesToken = RasterFiles;
    RasterFileName = "";

    while(RasterFilesToken != ""){
        RasterFileName = RasterFilesToken.substr(0,RasterFilesToken.find_first_of(";"));
        RasterFilesToken = RasterFilesToken.substr(RasterFilesToken.find_first_of(";") + 1);

        GDALDataset * pDS = (GDALDataset*) GDALOpen(RasterFileName.c_str(), GA_ReadOnly);
        GDALRasterBand * pRBInput = pDS->GetRasterBand(1);

        RasterMeta inputMeta (RasterFileName.c_str());

        // We need to figure out where in the output the input lives.
        int trans_i = OutputMeta.GetRowTranslation(&inputMeta);
        int trans_j = OutputMeta.GetColTranslation(&inputMeta);

        double * pInputLine = (double *) CPLMalloc(sizeof(double)*pRBInput->GetXSize());

        for (i = 0; i < pRBInput->GetYSize(); i++){
            pRBInput->RasterIO(GF_Read, 0,  i, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
            // here's where we need to get the correct row of the output. Replace
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Read, 0,  trans_i+i, OutputMeta.GetCols(), 1, pOutputLine, OutputMeta.GetCols(), 1, GDT_Float64, 0, 0);

            for (j = 0; j < pRBInput->GetXSize(); j++){
                // If the input line is empty then do nothing
                if ( (pInputLine[j] != inputMeta.GetNoDataValue())
                     && pOutputLine[trans_j+j] ==  OutputMeta.GetNoDataValue())
                {
                    pOutputLine[trans_j+j] = pInputLine[j];
                }
            }
            // here's where we need to get the correct row of the output. Replace
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  trans_i+i, OutputMeta.GetCols(), 1, pOutputLine, OutputMeta.GetCols(), 1, GDT_Float64, 0, 0);

        }
        CPLFree(pInputLine);

    }

    /*****************************************************************************************
     * Now Close everything and clean it all up
     */
    CPLFree(pOutputLine);
    CalculateStats(pDSOutput->GetRasterBand(1));
    GDALClose(pDSOutput);

    return PROCESS_OK;
}


}
