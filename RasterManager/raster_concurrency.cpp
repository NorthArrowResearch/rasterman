#define MY_DLL_EXPORT

#include "raster.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "gdal_priv.h"
#include "helpers.h"

namespace RasterManager {

int RasterMeta::IsConcurrent(RasterMeta * pCompareMeta){
    if (pCompareMeta->GetTop() == GetTop()
            && pCompareMeta->GetLeft() == GetLeft()
            && pCompareMeta->GetRows() == GetRows()
            && pCompareMeta->GetCols() == GetCols() ){
        return 1;
    }
    return 0;
}


int Raster::MakeConcurrent(const char * csRasters, const char * csRasterOutputs){
    // Loop through the strings, delimited by ;
    std::string sInPutFileName,
            sOutputFileName,
            sRasterInputFiles(csRasters), sRasterInputTokens,
            sRasterOutputFiles(csRasterOutputs), sRasterOutputTokens;

    // Terminate with a semicolon if it hasn't already been done
    if (!EndsWith(csRasters, ";")){
        sRasterInputFiles.append(";");
    }

    sRasterInputTokens = sRasterInputFiles;
    if (!EndsWith(csRasterOutputs, ";")){
        sRasterOutputFiles.append(";");
    }

    sRasterOutputTokens = sRasterOutputFiles;
    // The Master meta is the one we will use to output all the raster files
    // It will be the boolean intersect of all
    RasterMeta MasterMeta;

    double dNoDataValue = (double) std::numeric_limits<float>::lowest();

    /*****************************************************************************************
     * Open all the relevant files and figure out the bounds of the final file.
     */
    int counter = 0;
    while(sRasterInputTokens != ""){
        counter++;
        sInPutFileName = sRasterInputTokens.substr(0,sRasterInputTokens.find_first_of(";"));
        sRasterInputTokens = sRasterInputTokens.substr(sRasterInputTokens.find_first_of(";") + 1);

        sOutputFileName = sRasterOutputTokens.substr(0,sRasterOutputTokens.find_first_of(";"));
        sRasterOutputTokens = sRasterOutputTokens.substr(sRasterOutputTokens.find_first_of(";") + 1);

        if (sInPutFileName.c_str() == NULL)
            return INPUT_FILE_ERROR;

        if (sOutputFileName == "")
            throw RasterManagerException(ARGUMENT_VALIDATION, "Number of output filepaths does not match number of input filepaths.");

        CheckFile(sInPutFileName.c_str(), true);

        RasterMeta erRasterInput (sInPutFileName.c_str());

        // First time round set the bounds to the first raster we give it.
        if (counter==1){
            MasterMeta = erRasterInput;
            MasterMeta.SetNoDataValue(dNoDataValue);
            MasterMeta.SetGDALDataType(GDT_Float32);
        }
        else{
            if (erRasterInput.IsOthogonal() == 0){
                QString sErr = QString("All rasters must be orthogonal: %1").arg(sInPutFileName.c_str());
                throw RasterManagerException(INPUT_FILE_NOT_VALID, sErr);
            }
            else if(erRasterInput.GetCellHeight() != MasterMeta.GetCellHeight()
                    || erRasterInput.GetCellWidth() != MasterMeta.GetCellWidth() ){
                QString sErr = QString("Cell resolutions must be the same for all rasters: %1").arg(sInPutFileName.c_str());
                throw RasterManagerException(INPUT_FILE_NOT_VALID, sErr);
            }
            else {
                MasterMeta.Union(&erRasterInput);
            }
        }
    }

    /*****************************************************************************************
     * Loop over the inputs and then the rows and columns
     */
    int i,j;
    sInPutFileName = "";
    sOutputFileName = "";
    sRasterInputTokens = sRasterInputFiles;
    sRasterOutputTokens = sRasterOutputFiles;

    while(sRasterInputTokens != ""){

        sInPutFileName = sRasterInputTokens.substr(0,sRasterInputTokens.find_first_of(";"));
        sRasterInputTokens = sRasterInputTokens.substr(sRasterInputTokens.find_first_of(";") + 1);

        sOutputFileName = sRasterOutputTokens.substr(0,sRasterOutputTokens.find_first_of(";"));
        sRasterOutputTokens = sRasterOutputTokens.substr(sRasterOutputTokens.find_first_of(";") + 1);

        // Create the output dataset for writing
        GDALDataset * pDSOutput = CreateOutputDS(sOutputFileName.c_str(), &MasterMeta);
        double * pOutputLine = (double *) CPLMalloc(sizeof(double)*MasterMeta.GetCols());

        GDALDataset * pDS = (GDALDataset*) GDALOpen(sInPutFileName.c_str(), GA_ReadOnly);
        GDALRasterBand * pRBInput = pDS->GetRasterBand(1);

        RasterMeta inputMeta (sInPutFileName.c_str());

        /*****************************************************************************************
         * Loop over the output file to make sure every cell gets a value of fNoDataValue
         * Every line is the same so we can have the for loops adjacent
         */
        for (int outj = 0; outj < MasterMeta.GetCols(); outj++){
            pOutputLine[outj] = dNoDataValue;
        }
        for (int outi = 0; outi < MasterMeta.GetRows(); outi++){
            pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  outi, MasterMeta.GetCols(), 1, pOutputLine, MasterMeta.GetCols(), 1, GDT_Float64, 0, 0);
        }

        // We need to figure out where in the output the input lives.
        int trans_i = MasterMeta.GetRowTranslation(&inputMeta);
        int trans_j = MasterMeta.GetColTranslation(&inputMeta);

        double * pInputLine = (double *) CPLMalloc(sizeof(double)*pRBInput->GetXSize());

        for (i = 0; i < pRBInput->GetYSize(); i++){
            if (i - trans_i <= MasterMeta.GetRows() ){
                pRBInput->RasterIO(GF_Read, 0,  i, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
                // here's where we need to get the correct row of the output. Replace
                pDSOutput->GetRasterBand(1)->RasterIO(GF_Read, 0,  i- trans_i, MasterMeta.GetCols(), 1, pOutputLine, MasterMeta.GetCols(), 1, GDT_Float64, 0, 0);

                for (j = 0; j < pRBInput->GetXSize(); j++){
                    // If the input line is empty then do nothing
                    if ( trans_j+j <= MasterMeta.GetCols()
                         && pInputLine[j] != inputMeta.GetNoDataValue()
                         && pOutputLine[trans_j+j] ==  MasterMeta.GetNoDataValue() )
                    {
                        pOutputLine[trans_j+j] = pInputLine[j];
                    }
                }
                // here's where we need to get the correct row of the output. Replace
                pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i - trans_i, MasterMeta.GetCols(), 1, pOutputLine, MasterMeta.GetCols(), 1, GDT_Float64, 0, 0);
            }

        }
        CPLFree(pOutputLine);
        CPLFree(pInputLine);

        CalculateStats(pDSOutput->GetRasterBand(1));

        GDALClose(pDSOutput);

        PrintRasterProperties(sOutputFileName.c_str());
    }
    return PROCESS_OK;
}

}
